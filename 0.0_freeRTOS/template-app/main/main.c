#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_task_wdt.h"
void task1(void)
{
    printf("TASK1 begin \n");
    // vTaskSuspendAll();
    for (int i = 0; i < 65535; i++)
    {
        for (int j = 0; j < 65535; j++)
        {
            // printf("TASK1\n");
            vTaskDelay(1);
        }
    }
    
    printf("TASK1 end \n");

    // while(true)
    // {
    //     vTaskDelay(10);//喂狗，防止任务看门狗报错
    // };//任务不能自行结束，所以要加个循环卡住，或者调用协程函数删除任务
    vTaskDelete(NULL);//参数为NULL时删除自己
}

void task2(void )
{
    TickType_t xLastTickWakeTime;
    esp_task_wdt_add(NULL);//把当前任务加入任务看门狗
    while (1)
    {
        xLastTickWakeTime = xTaskGetTickCount();
        esp_task_wdt_reset();//喂狗，默认5s触发看门狗，只管当前任务
        printf("TASK2\n");//占用栈内存大小288Byte
        vTaskDelay(1000/portTICK_PERIOD_MS);
        // vTaskDelayUntil(&xLastTickWakeTime,1000/portTICK_PERIOD_MS);//这个函数不会喂狗
    }
    
    
    // while(true)
    // {
    //     vTaskDelay(10);//喂狗，防止任务看门狗报错
    // };//任务不能自行结束，所以要加个循环卡住，或者调用协程函数删除任务
    vTaskDelete(NULL);//参数为NULL时删除自己
}
int a = 1; 
void app_main(void)
{
    TaskHandle_t task2H;
    UBaseType_t iStack;
    xTaskCreate(task1,"task1",1024,NULL,1,NULL);//1024传参调用printf会堆栈溢出，改成2048就不报错了
    xTaskCreate(task2,"task2",1024,NULL,1,&task2H);//1024传参调用printf会堆栈溢出，改成2048就不报错了
    while (1)
    {
        iStack = uxTaskGetStackHighWaterMark(task2H);
        printf("%d\n",iStack);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    
}
