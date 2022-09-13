#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 计数信号量句柄对象
static xSemaphoreHandle counting_semaphore;

// 任务句柄对象
TaskHandle_t task1_handle,task2_handle,task3_handle;

// 任务名称
const char* task1_name = "task1";
const char* task2_name = "task2";
const char* task3_name = "task3";

// 任务函数
void task1_func(void* params);
void task2_func(void* params);
void task3_func(void* params);

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

    // 创建计数信号是，大小为5，初始值为0
    counting_semaphore = xSemaphoreCreateCounting(5,0);

    // 创建任务
    xTaskCreate(task1_func,     /*任务函数*/
        task1_name,             /*任务名称*/
        2048,                   /*任务堆栈大小*/
        (void*)task1_name,      /*任务参数*/
        1,                      /*任务优先级*/
        &task1_handle           /*任务句柄对象*/
    );

    // 创建任务
    xTaskCreate(task2_func,     /*任务函数*/
        task2_name,             /*任务名称*/
        2048,                   /*任务堆栈大小*/
        (void*)task2_name,      /*任务参数*/
        1,                      /*任务优先级*/
        &task2_handle           /*任务句柄对象*/
    );

     // 创建任务
    xTaskCreate(task3_func,     /*任务函数*/
        task3_name,             /*任务名称*/
        2048,                   /*任务堆栈大小*/
        (void*)task3_name,      /*任务参数*/
        1,                      /*任务优先级*/
        &task3_handle           /*任务句柄对象*/
    );

}

void task1_func(void* params){
    const char* name = (const char*)params;
    int counts = 1;
    printf("task %s inited.\n",name);
    // 初始化时，进入阻塞状态
    xSemaphoreTake(counting_semaphore,0);
    for(;;){
        // 获取信号量
        xSemaphoreTake(counting_semaphore,portMAX_DELAY);
        printf("task %s has got semaphore.%d\n",name,counts);
        counts++;
    }
}

void task2_func(void* params){
    const char* name = (const char*)params;
    int counts = 1;
    printf("task %s inited.\n",name);

    xSemaphoreTake(counting_semaphore,0);
    for(;;){
        xSemaphoreTake(counting_semaphore,portMAX_DELAY);
        printf("task %s has got semaphore.%d\n",name,counts);
        counts++;
    }
}

void task3_func(void* params){
    const char* name = (const char*)params;
    int counts = 1;
    printf("task %s inited.\n",name);
    for(;;){
        printf("task %s is running.%d\n",name,counts);

        // 释放信号量
        xSemaphoreGive(counting_semaphore);

        counts++;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
