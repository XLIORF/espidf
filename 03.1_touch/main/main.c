#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "touch.h"

#define BOARD_LED 2

void app_main(void)
{
    uint16_t led_state=0;
    gpio_config_t led_pin = {
        .pin_bit_mask = 1UL << BOARD_LED,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = false,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_pin);
    touch_init();
    touch_config(Touch0);
    while (1)
	{
        if(touch_state(Touch0))
        {
            led_state = ~led_state;
            gpio_set_level(BOARD_LED,led_state);
        }
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}