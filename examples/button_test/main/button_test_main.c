#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_GPIO     0  // Change this to your button GPIO
#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "button_test";

static void IRAM_ATTR button_isr_handler(void* arg)
{
    static uint32_t last_press_time = 0;
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Simple debounce
    if ((current_time - last_press_time) > 300) {
        last_press_time = current_time;
        ESP_LOGI(TAG, "Button pressed!");
    }
}

void app_main(void)
{
    // Configure button GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,    // Interrupt on falling edge
        .mode = GPIO_MODE_INPUT,           // Set as input
        .pin_bit_mask = (1ULL << BUTTON_GPIO), // Bit mask of the pin
        .pull_up_en = GPIO_PULLUP_ENABLE,  // Enable pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down
    };
    
    gpio_config(&io_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    
    // Hook ISR handler for specific gpio pin
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);

    ESP_LOGI(TAG, "Button test initialized. Press the button to test!");

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
