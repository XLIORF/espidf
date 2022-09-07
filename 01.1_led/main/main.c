#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


#define BOARD_LED 2
#define KEY 25 
void app_main(void)
{
    int level = 0;
    gpio_config_t led_io;
    led_io.pin_bit_mask = 1 << BOARD_LED;
    led_io.mode = GPIO_MODE_OUTPUT;
    led_io.pull_up_en = GPIO_PULLUP_DISABLE;
    led_io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    led_io.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&led_io);

    // gpio_config_t key;
    // key.pin_bit_mask = 1ULL << KEY; //不加ULL只能用前32个，因为1默认为32位，应该是这样的
    // key.mode = GPIO_MODE_INPUT;
    // key.pull_up_en = GPIO_PULLUP_DISABLE;
    // key.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // key.intr_type = GPIO_INTR_DISABLE;
    // gpio_config(&key);

    gpio_config_t key={
        .pin_bit_mask = 1ULL << KEY, //不加ULL只能用前32个，因为1默认为32位，应该是这样的
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&key);

    while (1)
    {
        while(gpio_get_level(KEY));
        vTaskDelay(pdMS_TO_TICKS(10));
        if(gpio_get_level(KEY) == 0)
        {
            level = !level;
            while(!gpio_get_level(KEY));
        }
        gpio_set_level(BOARD_LED,level);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
}

