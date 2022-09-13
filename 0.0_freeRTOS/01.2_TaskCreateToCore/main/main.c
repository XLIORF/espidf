#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 任务句柄
TaskHandle_t task1_handle,task2_handle;

// 任务函数
void task1_func(void* params);
void task2_func(void* params);

void app_main(void)
{
    /* 查询芯片信息并输出 */
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

    // 查询当前主函数在哪个CPU运行
    printf("main task run at cpu%d .\n",xPortGetCoreID());

    // 将任务在指定CPU中创建并运行
    xTaskCreatePinnedToCore(
        task1_func,         /*任务函数*/
        "task1",            /*任务名称*/
        2048,               /*任务堆栈大小*/
        NULL,               /*任务参数*/
        1,                  /*任务优先级*/
        &task1_handle,      /*任务句柄*/
        0                   /*处理器ID,即指定的CPU*/
        );
    xTaskCreatePinnedToCore(
        task2_func,         /*任务函数*/
        "task2",            /*任务名称*/
        2048,               /*任务堆栈大小*/
        NULL,               /*任务参数*/
        1,                  /*任务优先级*/
        &task2_handle,      /*任务句柄*/
        1                   /*处理器ID,即指定的CPU*/
        );
}

void task1_func(void* params){
    int counter = 1;
    printf("task1 inited.\n");
    printf("task1 running at cpu%d.\n",xPortGetCoreID());
    for(;;){
        printf("[task1]:this is the helloworld task:%d.\n",counter);
        counter++;
        // 任务延时阻塞
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


void task2_func(void* params){
    int counter = 1;
    printf("task2 inited.\n");
    printf("task2 running at cpu%d.\n",xPortGetCoreID());
    for(;;){
        printf("[task2]:this is the helloworld task:%d.\n",counter);
        counter++;
        // 任务延时阻塞
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
