#include "arfi/apps/RestReaderApp.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/services/RestService.hpp"
#include "arfi/services/SettingsService.hpp"
#include "arfi/services/WifiService.hpp"
#include "arfi/ui/Canvas.hpp"

#include "cJSON.h"

#include <algorithm>
#include <cctype>
#include <cstdio>

#ifndef ARFI_REST_URL
#define ARFI_REST_URL "http://worldtimeapi.org/api/timezone/Europe/Madrid"
#endif

namespace arfi {

namespace {
constexpr size_t kPreviewCharsPerLine = 36;

bool readJsonItem(cJSON* root, const char* key, std::string& out) {
    cJSON* item = cJSON_GetObjectItemCaseSensitive(root, key);
    if (item == nullptr) {
        return false;
    }

    char line[80];
    if (cJSON_IsString(item) && item->valuestring != nullptr) {
        out = item->valuestring;
        return true;
    }
    if (cJSON_IsNumber(item)) {
        std::snprintf(line, sizeof(line), "%.2f", item->valuedouble);
        out = line;
        return true;
    }
    if (cJSON_IsBool(item)) {
        out = cJSON_IsTrue(item) ? "true" : "false";
        return true;
    }

    return false;
}
} // namespace

void RestReaderApp::onEnter() {
    endpoint_ = ctx_.settings == nullptr ? ARFI_REST_URL : ctx_.settings->getString("rest_url", ARFI_REST_URL);
    if (endpoint_.empty()) {
        endpoint_ = ARFI_REST_URL;
    }
    state_ = State::Idle;
    reading_.clear();
    response_ = RestResponse{};
}

void RestReaderApp::update(uint32_t) {}

void RestReaderApp::render(Canvas& canvas) {
    canvas.clear(theme_.background);
    canvas.drawText(6, 4, "REST API", theme_.text, 1);
    canvas.hline(0, 16, canvas.width(), theme_.surface_alt);

    const bool has_wifi = ctx_.wifi != nullptr && ctx_.wifi->initialized() && ctx_.wifi->hasCredentials();
    if (!has_wifi) {
        canvas.drawCenteredText(canvas.width() / 2, 38, "WIFI NOT SET", theme_.warning, 2);
        canvas.drawText(8, 70, "SET ARFI_WIFI_SSID", theme_.muted, 1);
        canvas.drawText(8, 84, "SET ARFI_WIFI_PASSWORD", theme_.muted, 1);
        canvas.drawText(8, 112, "A GET  B RAW", theme_.muted, 1);
        canvas.drawText(8, 126, "A LONG HOME", theme_.muted, 1);
        return;
    }

    char line[48];
    std::snprintf(
        line,
        sizeof(line),
        "%s HTTP %d N%lu",
        stateName_(state_),
        response_.status_code,
        static_cast<unsigned long>(request_count_));
    canvas.drawText(8, 28, line, state_ == State::Error ? theme_.warning : theme_.accent, 1);

    drawSanitizedLine(canvas, 8, 44, endpoint_, theme_.muted, 0);

    if (state_ == State::Idle) {
        canvas.drawCenteredText(canvas.width() / 2, 68, "PRESS A", theme_.text, 2);
    } else if (state_ == State::Fetching) {
        canvas.drawCenteredText(canvas.width() / 2, 68, "FETCHING", theme_.text, 2);
    } else if (raw_view_) {
        drawBodyPreview(canvas, 8, 62, theme_.text);
    } else if (!reading_.empty()) {
        canvas.drawText(8, 66, "READING", theme_.muted, 1);
        drawSanitizedLine(canvas, 8, 84, reading_, theme_.text, 0);
    } else if (state_ == State::Error) {
        std::snprintf(line, sizeof(line), "ERR %s", errorLabel_(response_));
        canvas.drawCenteredText(canvas.width() / 2, 72, line, theme_.warning, 1);
        std::snprintf(
            line,
            sizeof(line),
            "0X%X E%d",
            static_cast<unsigned>(response_.error),
            response_.transport_errno);
        canvas.drawCenteredText(canvas.width() / 2, 86, line, theme_.muted, 1);
    } else {
        canvas.drawCenteredText(canvas.width() / 2, 72, "NO FIELD", theme_.warning, 1);
    }

    canvas.drawText(8, 112, "A GET  B RAW", theme_.muted, 1);
    canvas.drawText(8, 126, "A LONG HOME", theme_.muted, 1);
}

bool RestReaderApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Primary && event.type == InputEventType::ShortPress) {
        fetch();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        raw_view_ = !raw_view_;
        return true;
    }

    return false;
}

void RestReaderApp::fetch() {
    if (ctx_.rest == nullptr || !ctx_.rest->available()) {
        state_ = State::Error;
        response_.error = ESP_ERR_INVALID_STATE;
        return;
    }

    state_ = State::Fetching;
    reading_.clear();
    response_ = RestResponse{};

    const esp_err_t err = ctx_.rest->get(endpoint_.c_str(), response_);
    ++request_count_;
    if (err == ESP_OK && response_.status_code >= 200 && response_.status_code < 300) {
        extractReading();
        state_ = State::Done;
    } else {
        state_ = State::Error;
    }
}

void RestReaderApp::extractReading() {
    reading_.clear();
    if (response_.body.empty()) {
        return;
    }

    cJSON* root = cJSON_ParseWithLength(response_.body.c_str(), response_.body.size());
    if (root == nullptr) {
        reading_ = response_.body.substr(0, kPreviewCharsPerLine);
        return;
    }

    const char* keys[] = {"reading", "value", "temperature", "datetime", "unixtime", "message"};
    for (const char* key : keys) {
        if (readJsonItem(root, key, reading_)) {
            cJSON_Delete(root);
            return;
        }
    }

    cJSON_Delete(root);
}

void RestReaderApp::drawSanitizedLine(
    Canvas& canvas,
    int x,
    int y,
    const std::string& text,
    Color color,
    size_t offset) const {
    char line[kPreviewCharsPerLine + 1] = {};
    size_t written = 0;
    for (size_t i = offset; i < text.size() && written < kPreviewCharsPerLine; ++i) {
        line[written++] = sanitize_(text[i]);
    }
    canvas.drawText(x, y, line, color, 1);
}

void RestReaderApp::drawBodyPreview(Canvas& canvas, int x, int y, Color color) const {
    for (size_t row = 0; row < 3; ++row) {
        drawSanitizedLine(canvas, x, y + static_cast<int>(row * 14), response_.body, color, row * kPreviewCharsPerLine);
    }
}

const char* RestReaderApp::stateName_(State state) {
    switch (state) {
    case State::Idle: return "IDLE";
    case State::Fetching: return "FETCH";
    case State::Done: return "DONE";
    case State::Error: return "ERROR";
    }
    return "ERROR";
}

const char* RestReaderApp::errorLabel_(const RestResponse& response) {
    if (response.error_name == "ESP_ERR_HTTP_FETCH_HEADER") {
        return "FETCH HEADER";
    }
    if (response.error_name == "ESP_ERR_HTTP_CONNECT") {
        return "HTTP CONNECT";
    }
    if (response.error_name == "ESP_ERR_HTTP_WRITE_DATA") {
        return "WRITE DATA";
    }
    if (response.error_name == "ESP_ERR_HTTP_READ_TIMEOUT") {
        return "READ TIMEOUT";
    }
    if (response.error_name == "ESP_ERR_HTTP_INVALID_TRANSPORT") {
        return "BAD SCHEME";
    }
    if (response.error_name == "ESP_ERR_HTTP_CONNECTION_CLOSED") {
        return "CLOSED";
    }
    if (response.error_name == "ESP_ERR_HTTP_INCOMPLETE_DATA") {
        return "INCOMPLETE";
    }
    if (response.error_name == "ESP_ERR_TIMEOUT") {
        return "WIFI TIMEOUT";
    }
    if (response.error_name == "ESP_FAIL") {
        return "WIFI FAIL";
    }
    if (!response.error_name.empty()) {
        return response.error_name.c_str();
    }
    return "UNKNOWN";
}

char RestReaderApp::sanitize_(char c) {
    const unsigned char uc = static_cast<unsigned char>(c);
    if (std::isalnum(uc) || c == ' ') {
        return c;
    }

    switch (c) {
    case ':':
    case '-':
    case '.':
    case '/':
    case '%':
    case '+':
    case '>':
    case '<':
        return c;
    default:
        return ' ';
    }
}

} // namespace arfi
