#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 定义引脚
#define GPIO_INPUT_IO_0     4
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0))
#define ESP_INTR_FLAG_DEFAULT 0

// 任务句柄对象
TaskHandle_t gpio_task_handle;
// 任务名称
const char* task_name = "gpio_task";


BaseType_t pxHigherPriorityTaskWoken = pdTRUE;

// 中断服务函数
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;

    xTaskNotifyFromISR(
        gpio_task_handle,               /*任务句柄对象*/
        gpio_num,                       /*通知值*/
        eSetValueWithoutOverwrite,      /*通知方式*/
        &pxHigherPriorityTaskWoken      /*用于保存是否有高优先级任务准备就绪。如果函数执行完毕后，此参数的数值是pdTRUE，说明有高优先级任务要执行，否则没有。*/
        );
}

// 中断服务响应任务函数
static void gpio_task_example(void* params)
{
    const char* name = (const char*)params;
    uint32_t gpio_num = 0;
    BaseType_t result;
    TickType_t max_wait_time = pdMS_TO_TICKS(500);

    printf("task %s inited.\n",name);

    for(;;){
        // 获取通知消息
        result = xTaskNotifyWait(0x000000000,0xFFFFFFFF,&gpio_num,max_wait_time);
        if(result == pdPASS){
            printf("task %s received gpio interruption %d.\n",name,gpio_num);
        }
        
    }
}


void app_main(void)
{
    /*输出芯片信息 */
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

    // GPIO配置
    gpio_config_t io_conf;
    // 上升沿中断触发
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    // 引脚位掩码
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // 引脚输入模式 
    io_conf.mode = GPIO_MODE_INPUT;
    // 引脚设置成上拉
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // 应用配置
    gpio_config(&io_conf);

    // 设置引脚中断类型
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    // 创建任务
    xTaskCreate(gpio_task_example, task_name, 2048, (void*)task_name, 2, &gpio_task_handle);

    // 注册中断服务
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // 绑定中断服务
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
}
