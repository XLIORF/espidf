#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// GPIO定义
#define GPIO_INPUT_IO_0     4
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0))
#define ESP_INTR_FLAG_DEFAULT 0

// 计数信号量句柄对象
static xSemaphoreHandle counting_semaphore;

// 中断服务函数
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;

    // 中断服务触发后，释放信号量资源
    xSemaphoreGiveFromISR(counting_semaphore,&higher_priority_task_woken);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num = (uint32_t)arg;
    // 初始获取信号量资源，将进入阻塞状态
    xSemaphoreTake(counting_semaphore,0);
    for(;;) {
        // 获取信号量资源
        xSemaphoreTake(counting_semaphore,portMAX_DELAY);
        // 任务逻辑代码
        printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));

    }
}


void app_main(void)
{
    /* 输出芯片信息 */
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

    printf("main task inited.\n");

    // GPIO配置对象
    gpio_config_t io_conf;
    // 上升沿中断触发
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    // GPIO引脚位掩码
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // GPIO模式设置为输入 
    io_conf.mode = GPIO_MODE_INPUT;
    // 引脚为上拉模式
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // 应用配置
    gpio_config(&io_conf);

    // 设置GPIO中断类型
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    // 创建计数信号量对象，容量为10，默认为值为0
    counting_semaphore = xSemaphoreCreateCounting(10,0);


    if(counting_semaphore != NULL){
        printf("main task created binary semaphore.\n");
         // 创建中断服务响应任务
        xTaskCreate(gpio_task_example, "gpio_task_example", 2048, (void*)GPIO_INPUT_IO_0, 10, NULL);

        // 安装（注册）中断服务
        gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
        // 绑定中断服务
        gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    }else{
        printf("cannot create binary semaphore.\n");
    }
}
