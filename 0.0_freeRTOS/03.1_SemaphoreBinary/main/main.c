#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 二值信号句柄对象
static xSemaphoreHandle binary_semphore;

// 任务句柄对象
TaskHandle_t task1_handle,task2_handle;

// 任务名称
const char* task1_name = "task1";
const char* task2_name = "task2";

// 任务函数
void task1_func(void* params);
void task2_func(void* params);

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

    // 初始化信号量
    vSemaphoreCreateBinary(binary_semphore);

    // 创建任务
    xTaskCreate(task1_func,         /*任务函数*/
        task1_name,                 /*任务名称*/
        2048,                       /*任务堆栈大小*/
        (void*)task1_name,          /*任务参数*/
        1,                          /*任务优先级*/
        &task1_handle               /*任务句柄对象*/
    );

    xTaskCreate(task2_func,         /*任务函数*/
        task2_name,                 /*任务名称*/
        2048,                       /*任务堆栈大小*/
        (void*)task2_name,          /*任务参数*/
        1,                          /*任务优先级*/
        &task2_handle               /*任务句柄对象*/
    );

}

void task1_func(void* params){
    const char* name = (const char*)params;
    int counts = 1;
    printf("task %s inited.\n",name);
    // 获取信号量资源，初始运行时进入阻塞状态
    xSemaphoreTake(binary_semphore,portMAX_DELAY);
    for(;;){
        // 获取信号量资源
        xSemaphoreTake(binary_semphore,portMAX_DELAY);
        // 当获取到信号量资源，就会执行以下代码
        printf("task %s has got semaphore.%d\n",name,counts);
        counts++;
    }
}

void task2_func(void* params){
    const char* name = (const char*)params;
    int counts = 1;
    printf("task %s inited.\n",name);
    for(;;){
        printf("task %s is running.%d\n",name,counts);

        // 释放信号量资源
        xSemaphoreGive(binary_semphore);

        counts++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
