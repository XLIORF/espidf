#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 任务句柄
TaskHandle_t task1_handle,task2_handle;

// 任务名称
const char* task1_name = "task1";
const char* task2_name = "task2";

// 任务函数
void task1_func(void* params);
void task2_func(void* params);

void app_main(void)
{
    printf("Hello world!\n");

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

    printf("main task inited.\n");
    
    // 创建任务
    xTaskCreate(task1_func,     /*任务函数*/
    task1_name,                 /*任务名称*/
    2048,                       /*任务堆栈大小*/
    (void*)task1_name,          /*任务参数*/
    1,                          /*任务优先级*/
    &task1_handle               /*任务句柄*/
    );
    xTaskCreate(task2_func,     /*任务函数*/
    task2_name,                 /*任务名称*/
    2048,                       /*任务堆栈大小*/
    (void*)task2_name,          /*任务参数*/
    1,                          /*任务优先级*/
    &task2_handle               /*任务句柄*/
    );
}

void task1_func(void* params){
    const char* task_name = (void*)params;
    int counts = 1;
    printf("task %s inited.\n",task1_name);

    for(;;){
        printf("task %s is running.%d\n",task_name,counts);
        counts++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task2_func(void* params){
    const char* task_name = (const char*)params;
    printf("this is the task %s running.\n",task_name);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    printf("task %s finised and deleted.\n",task_name);
    // vTaskDelete(NULL);//删除自己
    // 通过任务句柄删除指定任务
    vTaskDelete(task2_handle);//删除指定任务
    // 下面代码将不会被运行
    printf("task %s finised and deleted.\n",task_name);
}
