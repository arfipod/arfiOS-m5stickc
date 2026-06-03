#include "arfi/hal/M5StickCPlusDisplay.hpp"
#include "arfi/hal/M5StickCPlusPins.hpp"
#include "arfi/hal/M5StickCPlusPower.hpp"

#include "driver/spi_master.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_check.h"
#include "esp_log.h"

namespace arfi {

static const char* TAG = "m5_display";

esp_err_t M5StickCPlusDisplay::begin(const BoardConfig&, M5StickCPlusPower& power) {
    if (initialized_) {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(power.setBacklightPercent(90), TAG, "Backlight setup failed");

    spi_bus_config_t bus_config = {};
    bus_config.mosi_io_num = m5stickcplus::kLcdMosi;
    bus_config.miso_io_num = GPIO_NUM_NC;
    bus_config.sclk_io_num = m5stickcplus::kLcdSclk;
    bus_config.quadwp_io_num = GPIO_NUM_NC;
    bus_config.quadhd_io_num = GPIO_NUM_NC;
    bus_config.max_transfer_sz = m5stickcplus::kDisplayWidth * 40 * sizeof(uint16_t);

    esp_err_t err = spi_bus_initialize(m5stickcplus::kLcdSpiHost, &bus_config, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.dc_gpio_num = m5stickcplus::kLcdDc;
    io_config.cs_gpio_num = m5stickcplus::kLcdCs;
    io_config.pclk_hz = m5stickcplus::kLcdPixelClockHz;
    io_config.lcd_cmd_bits = m5stickcplus::kLcdCommandBits;
    io_config.lcd_param_bits = m5stickcplus::kLcdParameterBits;
    io_config.spi_mode = 0;
    io_config.trans_queue_depth = m5stickcplus::kLcdTransQueueDepth;

    ESP_RETURN_ON_ERROR(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)m5stickcplus::kLcdSpiHost, &io_config, &io_),
        TAG,
        "LCD IO creation failed");

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = m5stickcplus::kLcdReset;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panel_config.bits_per_pixel = 16;

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(io_, &panel_config, &panel_), TAG, "ST7789 creation failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel_), TAG, "LCD reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel_), TAG, "LCD init failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(panel_, true), TAG, "LCD invert failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_swap_xy(panel_, m5stickcplus::kLcdSwapXY), TAG, "LCD swap failed");
    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_mirror(panel_, m5stickcplus::kLcdMirrorX, m5stickcplus::kLcdMirrorY),
        TAG,
        "LCD mirror failed");
    ESP_RETURN_ON_ERROR(
        esp_lcd_panel_set_gap(panel_, m5stickcplus::kLcdGapX, m5stickcplus::kLcdGapY),
        TAG,
        "LCD gap failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel_, true), TAG, "LCD display on failed");

    initialized_ = true;
    ESP_LOGI(TAG, "ST7789 initialized");
    return ESP_OK;
}

esp_err_t M5StickCPlusDisplay::flush(const uint16_t* pixels, uint16_t width, uint16_t height) {
    if (!initialized_ || panel_ == nullptr || pixels == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }

    return esp_lcd_panel_draw_bitmap(panel_, 0, 0, width, height, pixels);
}

} // namespace arfi
