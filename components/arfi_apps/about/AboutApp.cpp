#include "arfi/apps/AboutApp.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/ui/Canvas.hpp"

#include "esp_heap_caps.h"
#include "esp_partition.h"

#include <cstdio>

namespace arfi {

namespace {
struct ResourceUsage {
    size_t ram_total = 0;
    size_t ram_used = 0;
    uint8_t ram_percent = 0;
    uint32_t flash_total = 0;
    uint32_t flash_used = 0;
    uint8_t flash_percent = 0;
};

uint8_t percentUsed(uint32_t used, uint32_t total) {
    if (total == 0) {
        return 0;
    }
    return static_cast<uint8_t>((used * 100U + total / 2U) / total);
}

uint32_t readLe32(const uint8_t* data) {
    return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
}

bool readAppImageUsage(uint32_t& used, uint32_t& total) {
    const esp_partition_t* partition =
        esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, nullptr);
    if (partition == nullptr) {
        return false;
    }

    total = partition->size;

    uint8_t header[24] = {};
    if (esp_partition_read(partition, 0, header, sizeof(header)) != ESP_OK || header[0] != 0xE9) {
        return false;
    }

    const uint8_t segment_count = header[1];
    if (segment_count == 0 || segment_count > 16) {
        return false;
    }

    uint32_t offset = sizeof(header);
    for (uint8_t segment = 0; segment < segment_count; ++segment) {
        uint8_t segment_header[8] = {};
        if (offset + sizeof(segment_header) > total ||
            esp_partition_read(partition, offset, segment_header, sizeof(segment_header)) != ESP_OK) {
            return false;
        }

        const uint32_t data_len = readLe32(segment_header + 4);
        if (data_len > total || offset + sizeof(segment_header) + data_len > total) {
            return false;
        }
        offset += sizeof(segment_header) + data_len;
    }

    used = ((offset + 15U) & ~15U) + 1U; // checksum byte after 16-byte padding
    if (header[23] == 1) {
        used += 32U; // appended SHA-256 digest
    }
    return used <= total;
}

ResourceUsage readResourceUsage() {
    ResourceUsage usage;
    usage.ram_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    const size_t ram_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    usage.ram_used = usage.ram_total > ram_free ? usage.ram_total - ram_free : 0;
    usage.ram_percent = percentUsed(static_cast<uint32_t>(usage.ram_used), static_cast<uint32_t>(usage.ram_total));

    if (readAppImageUsage(usage.flash_used, usage.flash_total)) {
        usage.flash_percent = percentUsed(usage.flash_used, usage.flash_total);
    }

    return usage;
}
} // namespace

void AboutApp::update(uint32_t) {}

void AboutApp::render(Canvas& canvas) {
    const ResourceUsage usage = readResourceUsage();
    char line[48];

    canvas.clear(theme_.background);
    canvas.drawCenteredText(canvas.width() / 2, 10, "ARFIOS", theme_.accent, 2);
    canvas.drawCenteredText(canvas.width() / 2, 34, "VERSION 0.1.0", theme_.text, 1);
    canvas.drawCenteredText(canvas.width() / 2, 48, ctx_.board.name, theme_.text, 1);

    std::snprintf(
        line,
        sizeof(line),
        "RAM %u%% %lu/%luK",
        usage.ram_percent,
        static_cast<unsigned long>(usage.ram_used / 1024),
        static_cast<unsigned long>(usage.ram_total / 1024));
    canvas.drawCenteredText(canvas.width() / 2, 68, line, theme_.text, 1);

    if (usage.flash_total > 0 && usage.flash_used > 0) {
        std::snprintf(
            line,
            sizeof(line),
            "FLASH %u%% %lu/%luK",
            usage.flash_percent,
            static_cast<unsigned long>(usage.flash_used / 1024),
            static_cast<unsigned long>(usage.flash_total / 1024));
    } else {
        std::snprintf(line, sizeof(line), "FLASH N/A");
    }
    canvas.drawCenteredText(canvas.width() / 2, 84, line, theme_.text, 1);

    canvas.drawCenteredText(canvas.width() / 2, 104, "ESP-IDF RUNTIME", theme_.muted, 1);
    canvas.drawCenteredText(canvas.width() / 2, 122, "A LONG HOME", theme_.muted, 1);
}

bool AboutApp::handleInput(const InputEvent&) { return false; }

} // namespace arfi
