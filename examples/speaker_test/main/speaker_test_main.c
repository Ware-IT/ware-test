#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "speaker_test";

#define I2S_BCLK        (GPIO_NUM_4)
#define I2S_LRCLK       (GPIO_NUM_3)
#define I2S_DOUT        (GPIO_NUM_2)

#define I2S_NUM         I2S_NUM_0
#define SAMPLE_RATE     44100
#define SAMPLE_BITS     16
#define CHANNEL_NUM     2
#define DMA_BUF_COUNT   8
#define DMA_BUF_LEN     1024

i2s_chan_handle_t tx_handle = NULL;

void generate_sine_wave(int16_t* samples, int count, float frequency) {
    static float phase = 0.0f;
    for (int i = 0; i < count; i++) {
        float sample = sinf(phase) * 8192.0f;
        samples[i] = (int16_t)sample;
        phase += 2.0f * M_PI * frequency / SAMPLE_RATE;
        if (phase >= 2.0f * M_PI) {
            phase -= 2.0f * M_PI;
        }
    }
}

void app_main(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK,
            .ws   = I2S_LRCLK,
            .dout = I2S_DOUT,
            .din  = I2S_GPIO_UNUSED,
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));

    static int16_t samples[1024];
    static size_t bytes_written;

    ESP_LOGI(TAG, "Speaker test initialized. Playing test tones...");

    float test_frequencies[] = {440.0f, 880.0f, 1760.0f};
    
    while (1) {
        for (int i = 0; i < sizeof(test_frequencies) / sizeof(float); i++) {
            ESP_LOGI(TAG, "Playing %.1f Hz tone", test_frequencies[i]);
            
            for (int j = 0; j < SAMPLE_RATE / DMA_BUF_LEN; j++) {
                generate_sine_wave(samples, DMA_BUF_LEN, test_frequencies[i]);
                ESP_ERROR_CHECK(i2s_channel_write(tx_handle, samples, sizeof(samples), &bytes_written, portMAX_DELAY));
            }
            
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}
