#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 任务句柄
TaskHandle_t send_task_handle,receive_task_handle;

// 任务名称
const char* send_task_name = "send_task";
const char* receive_task_name = "receive_task";

// 消息队列
xQueueHandle task_queue;

// 消息发送函数
void send_task_func(void* params);

// 消息接收函数
void receive_task_func(void* params);

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

    printf("main task inited.\n");

    // 创建一个容量为5*sizeof(long)大小的消息队列
    task_queue = xQueueCreate(5,sizeof(long));

    if(task_queue != NULL){
        // 创建消息发送任务
        xTaskCreate(send_task_func,     /*任务函数*/
            send_task_name,             /*任务名称*/
            2048,                       /*任务堆栈大小*/
            (void*)send_task_name,      /*任务参数*/
            1,                          /*任务优先级*/
            &send_task_handle           /*任务句柄*/
        );
        // 创建消息接收任务
        xTaskCreate(receive_task_func,       /*任务函数*/
            receive_task_name,               /*任务名称*/
            2048,                            /*任务堆栈大小*/
            (void*)receive_task_name,        /*任务参数*/
            1,                               /*任务优先级*/
            &receive_task_handle             /*任务句柄*/
        );
    }else{
        printf("cannot create task queue.\n");
    }
}

void send_task_func(void* params){
    long value_to_send = 1;
    const char* task_name = (const char*)params;
    portBASE_TYPE status;
    printf("task %s inited.\n",task_name);

    for(;;){
        printf("task %s send data %ld to queue.\n",task_name,value_to_send);
        // 将消息发送到消息队列，不用等待
        status = xQueueSendToBack(task_queue,&value_to_send,0);//最后一个是等待时间0不等待tickToWait一直等

        if(status != pdPASS){
            printf("task %s cannot send data to queue.\n",task_name);
        }
        value_to_send++;
        // allow the other sender task to execute
        // 允许其他发送任务执行
        taskYIELD();
        // 任务延时阻塞
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void receive_task_func(void* params){
    long recv_data = -1;
    const char* task_name = (const char*)params;
    portBASE_TYPE status;
    // 消息接收等待超时时长
    const portBASE_TYPE ticksToWaite = 500 / portTICK_PERIOD_MS;
    printf("task %s inited.\n",task_name);
    for(;;){
        printf("task %s is running.\n",task_name);
        // 判断当前等待消息是否为空
        if(uxQueueMessagesWaiting(task_queue) != 0){
            printf("task queue is empty\n");
        }
        // 阻塞等待接收消息
        status = xQueueReceive(task_queue,&recv_data,ticksToWaite);
        if(status == pdPASS){
            printf("-------task %s received data:%ld-------\n",task_name,recv_data);
        }else{
            printf("task %s cannot receive data from queue.\n",task_name);
        }
    }
}
