#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 定义输入接口
#define GPIO_INPUT_IO_0     4
// 接口选择
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0))
#define ESP_INTR_FLAG_DEFAULT 0

// 消息附一我
static xQueueHandle gpio_evt_queue = NULL;

// GPIO中断服务函数
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    // 从中断服务中发送消息
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// 响应GPIO中断任务函数
static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        // 接收消息
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}


void app_main(void)
{
    /* 获取芯片信息并输出 */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Free heap: %d\n", esp_get_free_heap_size());

    // GPIO配置对象
    gpio_config_t io_conf;

    // 上升沿触发
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    // 引脚的位掩码
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // 将GPIO设置为输入模式   
    io_conf.mode = GPIO_MODE_INPUT;
    // 使用上拉模式
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // 应用配置
    gpio_config(&io_conf);

    // 设置GPOI的中断类型
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    // 创建消息队列
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // 创建GPIO中断响应任务
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    // 注册GPIO中断服务
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // 绑定GPIO中断服务
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
}

