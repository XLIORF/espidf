#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

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
//xTaskNotifyGive和 ulTaskNotifyTake()
//和信号量差不多用法

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

    // 创建任务
    xTaskCreate(task1_func,task1_name,2048,(void*)task1_name,1,&task1_handle);
    xTaskCreate(task2_func,task2_name,2048,(void*)task2_name,1,&task2_handle);
    xTaskCreate(task3_func,task3_name,2048,(void*)task3_name,1,&task3_handle);

}


void task1_func(void* params){
    const char* name = (const char*)params;
    uint32_t value = 0;
    BaseType_t result;

    // 等待通知最大时间
    TickType_t max_wait_time = pdMS_TO_TICKS(500);

    printf("task %s inited.\n",name);

    for(;;){
        // 获取通知结果
        //ulBitsToClearOnEntry 在函数开始出将消息对应的位清零；
        //ulBitsToClearOnExit在函数退出时将消息对应的位清零，只有在接收到通知才有效
        result = xTaskNotifyWait(0x0000000,0xFFFFFFFF,&value,max_wait_time);
        if(result == pdPASS){
            printf("task %s received value %d from notification.\n",name,value);
        }
        
    }
}

void task2_func(void* params){
 const char* name = (const char*)params;
    uint32_t value = 0;
    BaseType_t result;
    // 等待通知最大时间
    TickType_t max_wait_time = pdMS_TO_TICKS(500);

    printf("task %s inited.\n",name);

    for(;;){
         // 获取通知结果
        result = xTaskNotifyWait(0x000000000,0xFFFFFFFF,&value,max_wait_time);
        if(result == pdPASS){
            printf("task %s received value %d from notification.\n",name,value);
        }
        
    }
}

void task3_func(void* params){
    const char* name = (const char*)params;
    int value = 1;
    printf("task %s inited.\n",name);

    for(;;){
        // 向任务发送通知
        xTaskNotify(task1_handle,value,eSetValueWithoutOverwrite);
        printf("task %s sent value %d to task1\n",name,value);
        value++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
         // 向任务发送通知
        xTaskNotify(task2_handle,value,eSetValueWithoutOverwrite);
        printf("task %s sent value %d to task2\n",name,value);
        value++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
    }
}
