#include <stdio.h>
// #include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "esp_system.h"
// #include "esp_spi_flash.h"

// 任务句柄
TaskHandle_t task_helloworld_handler;

// 任务函数
void task_helloworld_func(void* params);

// ESP32应用程序入口
void app_main(void)
{
   printf("main task inited.\n");
   // 创建任务
   xTaskCreate(task_helloworld_func,    /*任务函数*/
   "task_helloworld",                   /*任务名称*/
   2048,                                /*任务堆栈大小*/
   NULL,                                /*任务参数*/
   1,                                   /*任务优先级*/
   &task_helloworld_handler             /*任务句柄对象*/
   );
}

void task_helloworld_func(void* params){
    int counter = 1;
    printf("task_helloworld inited.\n");
    for(;;){
        printf("this is the helloworld task:%d.\n",counter);
        counter++;
        /*
        当cpu没有任务时freeRTOS自动切换到IDLE Task（空闲任务），任务优先级为0（最低），freeRTOS任务调度器会优先执行优先级高的任务，如果在高优先级中不喂狗就会触发任务看门口错误。
        所以要调用喂狗语句，或者调用freeRTOS的延时函数（它延时任务本质是挂起任务，执行其他任务），让其他程序喂狗（如IDLE Task）；
        */
        // 任务延时
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);//参数为空时删除自己，这条语句不是必须的，任务结束时最好有
}
