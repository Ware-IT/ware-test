#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_pdm.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "mic_test";

#define I2S_CLK       GPIO_NUM_42
#define I2S_DIN       GPIO_NUM_41

#define SAMPLE_RATE   44100
#define SAMPLE_BITS   16
#define DMA_BUF_COUNT 8
#define DMA_BUF_LEN   1024

static i2s_chan_handle_t rx_handle = NULL;

static int32_t calculate_audio_level(int16_t* samples, size_t count) {
    int32_t sum = 0;
    for (size_t i = 0; i < count; i++) {
        sum += abs(samples[i]);
    }
    return sum / count;
}

void app_main(void)
{
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_handle));

    i2s_pdm_rx_config_t pdm_rx_cfg = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = I2S_CLK,
            .din = I2S_DIN,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(rx_handle, &pdm_rx_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

    static int16_t samples[DMA_BUF_LEN];
    size_t bytes_read = 0;
    int32_t audio_level;

    ESP_LOGI(TAG, "Starting mic test...");

    while (1) {
        ESP_ERROR_CHECK(i2s_channel_read(rx_handle, samples, sizeof(samples), &bytes_read, portMAX_DELAY));
        
        if (bytes_read > 0) {
            audio_level = calculate_audio_level(samples, bytes_read / sizeof(int16_t));
            ESP_LOGI(TAG, "Audio level: %ld", audio_level);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
