#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 按位定义事件
#define WRITE_TASK_1 (1 << 0)
#define WRITE_TASK_2 (1 << 1)

// 任务句柄对象
TaskHandle_t write_task1_handle,write_task2_handle,read_task_handle;

// 任务名称
const char* task1_name = "write_task1";
const char* task2_name = "write_task2";
const char* task3_name = "read_task";

// 事件组句柄对象
EventGroupHandle_t event_group;

// 写任务函数
void write_task1_func(void* params);
void write_task2_func(void* params);
// 读任务函数
void read_task_func(void* params);

void app_main(void)
{
    /* 获取并输出芯片信息 */
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

    // 创建并初始化事件组
    event_group = xEventGroupCreate();

    // 创建任务
    xTaskCreate(write_task1_func,task1_name,2048,(void*)task1_name,1,&write_task1_handle);
    xTaskCreate(write_task2_func,task2_name,2048,(void*)task2_name,2,&write_task2_handle);
    xTaskCreate(read_task_func,task3_name,2048,(void*)task3_name,1,&read_task_handle);
}

void write_task1_func(void* params){
    const char* task_name = (const char*)params;
    int counts =  1;
    printf("task %s inited.\n",task_name);
    for(;;){
        printf("task %s is running.%d\n",task_name,counts);
        // 事件唤醒
        xEventGroupSetBits(event_group,WRITE_TASK_1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        counts++;
    }
}
void write_task2_func(void* params){
    const char* task_name = (const char*)params;
    int counts =  1;
    printf("task %s inited.\n",task_name);
    for(;;){
        printf("task %s is running.%d\n",task_name,counts);
         // 事件唤醒
        xEventGroupSetBits(event_group,WRITE_TASK_1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        counts++;
    }
}
void read_task_func(void* params){
    EventBits_t event_bits;
    const char* task_name = (const char*)params;
    printf("task %s inited.\n",task_name);
    for(;;){
        printf("task %s is running...\n",task_name);
        // 等待事件
        event_bits = xEventGroupWaitBits(event_group, 
        WRITE_TASK_1 | WRITE_TASK_2, 
        pdTRUE,
        pdFALSE, /*设置为不用等待所有事件发生才唤醒*/
        portMAX_DELAY);

        // 两个事件同时发生时唤醒
        if((event_bits & (WRITE_TASK_1 | WRITE_TASK_2)) == (WRITE_TASK_1 | WRITE_TASK_2)){
            printf("both task bit are setted.\n");
        }else if((event_bits & WRITE_TASK_1) == WRITE_TASK_1){ // 单独事件唤醒
            printf("**[task 1]** event bits is setted.\n");
        }else if((event_bits & WRITE_TASK_2) == WRITE_TASK_2){
            printf("**[task 2]** event bits is setted.\n");
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
