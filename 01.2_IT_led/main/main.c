#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define ONBOARD_LED 2
#define KEY 33
#define ESP_INTR_FLAG_DEFAULT  0

static xQueueHandle key_msg_queue = NULL;
//IRAM_ATTR 声明编译后的代码将放置在 ESP32 的内部 RAM
static void IRAM_ATTR interruprt_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    if(gpio_num == KEY)
    {
        xQueueSendFromISR(key_msg_queue,&gpio_num,NULL);
    }
}

void app_main(void)
{
    gpio_config_t led = 
    {
        .pin_bit_mask = 1ULL << ONBOARD_LED,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = false,
        .pull_down_en = false,
        .intr_type = false
    };
    gpio_config(&led);

    gpio_config_t key = 
    {
        .pin_bit_mask = 1ULL << KEY,
        .mode = GPIO_MODE_INPUT,//输入模式
        .pull_up_en = true,//上拉
        .pull_down_en = false,
        .intr_type = GPIO_INTR_NEGEDGE//下降沿触发
    };
    gpio_config(&key);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    gpio_isr_handler_add(KEY,interruprt_handler,(void *)KEY);

    key_msg_queue = xQueueCreate(1,sizeof(uint32_t)); //一定要先创建队列再使用。一定要分配足空间，否则数据会出错

    uint32_t io_num;
    bool led_state  = 0;
    while (true)
    {
        if(xQueueReceive(key_msg_queue, &io_num, portMAX_DELAY))
        {
            ESP_LOGI("led","led num:%d",io_num);
            if (io_num == KEY)
            {
                vTaskDelay(100/portTICK_PERIOD_MS);
                if(gpio_get_level(KEY) == 0)
                {
                    led_state = !led_state;
                }
            }
        }
        gpio_set_level(ONBOARD_LED,led_state);
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    
}

// static void IRAM_ATTR interruprt_handler(void *arg)
// {
//     uint32_t gpio_num = (uint32_t) arg;
//     if(gpio_num  == KEY)
//     {
//         if(gpio_get_level(KEY) == 0)
//         {
//             if(gpio_get_level(KEY) == 0)
//             {
//                 if(led_state == true)
//                     gpio_set_level(ONBOARD_LED,0);
//                 else
//                     gpio_set_level(ONBOARD_LED,1);
//                 led_state = !led_state;
//             }
//         }
//     }
// }

// void app_main(void)
// {
//     gpio_config_t led = 
//     {
//         .pin_bit_mask = 1ULL << ONBOARD_LED,
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_up_en = false,
//         .pull_down_en = false,
//         .intr_type = false
//     };
//     gpio_config(&led);

//     gpio_config_t key = 
//     {
//         .pin_bit_mask = 1ULL << KEY,
//         .mode = GPIO_MODE_INPUT,//输入模式
//         .pull_up_en = true,//上拉
//         .pull_down_en = false,
//         .intr_type = GPIO_INTR_NEGEDGE//下降沿触发
//     };
//     gpio_config(&key);

//     gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

//     gpio_isr_handler_add(KEY,interruprt_handler,(void *)KEY);
// }