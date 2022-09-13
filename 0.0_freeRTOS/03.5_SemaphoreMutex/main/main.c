#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 互斥量句柄对象
static xSemaphoreHandle mutex;

// 任务句柄
TaskHandle_t task1_handle,task2_handle;

// 任务名称
const char* task1_name = "task1";
const char* task2_name = "task2";

// 共享资源
static int counts = 0;

// 任务函数
void task1_func(void* params);
void task2_func(void* params);


void app_main(void)
{
    /* 获取并打印芯片信息 */
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
    // 创建互斥量句柄对象
    mutex = xSemaphoreCreateMutex();

    if(mutex){
        // 创建任务
        xTaskCreate(task1_func,         /*任务函数*/
            task1_name,                 /*任务名称*/
            2048,                       /*任务堆栈*/
            (void*)task1_name,          /*任务参数*/
            1,                          /*任务优先级*/
            &task1_handle               /*任务句柄*/
        );

        // 创建任务
        xTaskCreate(task2_func,         /*任务函数*/
            task2_name,                 /*任务名称*/
            2048,                       /*任务堆栈*/
            (void*)task2_name,          /*任务参数*/
            1,                          /*任务优先级*/
            &task2_handle               /*任务句柄*/
        );
    }else{
        printf("can not create mutex semaphore.\n");
    }
}

void task1_func(void* params){
    const char* name = (const char*)params;
    TickType_t last_wake_time;
    const TickType_t x_frequency = 1000;
    // 获取当前系统时间
    last_wake_time = xTaskGetTickCount();

    printf("task %s inited.\n",name);

    for(;;){
        // 获取互斥量
        xSemaphoreTake(mutex,portMAX_DELAY);
        printf("task %s has got semaphore.current count = %d\n",name,counts);
        // 共享资源更新
        counts++;
        // 释放互斥量
        xSemaphoreGive(mutex);
        // 任务阻塞
        vTaskDelayUntil(&last_wake_time,x_frequency);
    }
}

void task2_func(void* params){
     const char* name = (const char*)params;
    TickType_t last_wake_time;
    const TickType_t x_frequency = 1000;
    // 获取当前系统时间
    last_wake_time = xTaskGetTickCount();

    printf("task %s inited.\n",name);

    for(;;){
        // 获取互斥量
        xSemaphoreTake(mutex,portMAX_DELAY);
        printf("task %s has got semaphore.current count = %d\n",name,counts);
        // 共享资源更新
        counts--;
        // 释放互斥量
        xSemaphoreGive(mutex);
        // 任务阻塞
        vTaskDelayUntil(&last_wake_time,x_frequency);
    }
}
