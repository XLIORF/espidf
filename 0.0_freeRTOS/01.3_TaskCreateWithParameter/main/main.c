#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

// 任务句柄对象
TaskHandle_t task1_handler,task2_handler;

// 定义任务参数
typedef struct {
    char name[32]; /*任务名称*/
    int value; /*任务初始值*/
} task_config_t;

// 任务参数实例化
task_config_t task1_cfg = {
    "task1",
    100
};

task_config_t task2_cfg = {
    "task2",
    200
};

// 任务函数
void task_func(void* params);

void app_main(void)
{
    printf("main task inited.\n");

    // 创建任务
    xTaskCreate(
        task_func,              /*任务函数*/
        task1_cfg.name,         /*任务名称*/
        2048,                   /*任务堆栈大小*/
        (void*)&task1_cfg,      /*任务参数，传递参数对象指针*/
        1,                      /*任务优先级*/
        &task1_handler          /*任务句柄对象*/
        );

     xTaskCreate(
        task_func,              /*任务函数*/
        task2_cfg.name,         /*任务名称*/
        2048,                   /*任务堆栈大小*/
        (void*)&task2_cfg,      /*任务参数，传递参数对象指针*/
        1,                      /*任务优先级*/
        &task2_handler          /*任务句柄对象*/
        );
}

void task_func(void* params){
    // 将参数转换
    task_config_t* cfg = (task_config_t*)params;
    int counts = 1;
    printf("task:%s inited,value:%d.,\n",cfg->name,cfg->value);
    for(;;){
        printf("task:%s is running!%d\n",cfg->name,counts);
        counts++;
        // 任务阻塞延时
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
