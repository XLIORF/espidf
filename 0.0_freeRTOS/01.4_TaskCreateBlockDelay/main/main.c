#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 任务句柄对象
TaskHandle_t task_handle;
// 任务名称
const char* task_name = "task_1";
// 任务函数
void task_func(void* params);

void app_main(void)
{
    printf("main task inited.\n");
    // 创建任务
    xTaskCreate(task_func,          /*任务函数*/
    task_name,                      /*任务名称*/
    2048,                           /*任务堆栈大小*/
    (void*)task_name,               /*任务参数*/
    1,                              /*任务优先级*/
    &task_handle                    /*任务句柄*/
    );
}

void task_func(void* params){
    const char* task_name = (const char*)params;
    int counts = 1;

    // 获取当前滴答数
    portTickType xLastTick = xTaskGetTickCount();

    printf("task:%s inited.\n",task_name);
    for(;;){
        printf("task:%s is running.%d\n",task_name,counts);
        counts++;
        // 阻塞延时固定的滴答周期数
        vTaskDelayUntil(&xLastTick,1000 / portTICK_PERIOD_MS);

    }
}
