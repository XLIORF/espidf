#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 任务句柄
TaskHandle_t task1_handle,task2_handle;
// 任务名称
const char* task1_name = "task_1";
const char* task2_name = "task_2";

// 任务函数
void task_func(void* params);

void app_main(void)
{
   printf("main task initd.\n");
   
   // 创建任务
   xTaskCreate(task_func,          /*任务函数*/
   task1_name,                     /*任务名称*/
   2048,                           /*任务堆栈大小*/
   (void*)task1_name,              /*任务参数*/
   1,                              /*任务优先级*/
   &task1_handle                   /*任务句柄对象*/
   );
    xTaskCreate(task_func,          /*任务函数*/
   task2_name,                     /*任务名称*/
   2048,                           /*任务堆栈大小*/
   (void*)task2_name,              /*任务参数*/
   2,                              /*任务优先级*/
   &task2_handle                   /*任务句柄对象*/
   );
}

void task_func(void* params){
    const char* task_name = (const char*)params;
    int counts = 1;

    printf("task %s inited.\n",task_name);
    for(;;){
        printf("task %s is running.%d\n",task_name,counts);
        counts++;
        // 任务延时阻塞
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
