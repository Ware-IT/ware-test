#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "button_test";

#define BUTTON_GPIO     GPIO_NUM_0

static void IRAM_ATTR button_isr_handler(void* arg)
{
    static uint32_t last_press_time = 0;
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    if ((current_time - last_press_time) > 300) {
        last_press_time = current_time;
        ESP_LOGI(TAG, "Button pressed!");
    }
}

void app_main(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL));

    ESP_LOGI(TAG, "Button test initialized on GPIO %d", BUTTON_GPIO);

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
