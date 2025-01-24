#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_log.h"

static const char *TAG = "mic_test";

#define I2S_NUM         I2S_NUM_0  // I2S peripheral number
#define SAMPLE_RATE     16000      // Sample rate in Hz
#define SAMPLE_BITS     16         // Sample bits
#define CHANNEL_NUM     1          // Number of channels (1 for mono, 2 for stereo)
#define DMA_BUF_COUNT   8          // Number of DMA buffers
#define DMA_BUF_LEN     1024       // Length of each DMA buffer

void app_main(void)
{
    // Configure I2S
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = SAMPLE_BITS,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = DMA_BUF_COUNT,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // Install and start I2S driver
    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL));

    // Configure I2S pins
    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,    // Bit clock pin
        .ws_io_num = 25,     // Word select pin
        .data_out_num = -1,  // Not used for mic input
        .data_in_num = 22    // Data input pin
    };
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM, &pin_config));

    // Buffer for audio samples
    int32_t samples[1024];
    size_t bytes_read;

    ESP_LOGI(TAG, "Microphone test initialized. Starting to read audio samples...");

    while (1) {
        // Read audio samples
        i2s_read(I2S_NUM, samples, sizeof(samples), &bytes_read, portMAX_DELAY);
        
        // Calculate average amplitude for basic audio level monitoring
        int32_t sum = 0;
        int samples_read = bytes_read / sizeof(int32_t);
        
        for (int i = 0; i < samples_read; i++) {
            sum += abs(samples[i]);
        }
        
        int32_t average = sum / samples_read;
        ESP_LOGI(TAG, "Average audio level: %ld", average);
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
