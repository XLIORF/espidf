#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
int a=1;
void task(void * pvParameter)
{
    int * ppa;
    ppa = (int *)pvParameter;
    *ppa +=1;
    
    printf("TASK%d Hello \n",*ppa);
    
    while(true)vTaskDelay(1000);//任务不能自行结束，所以要加个循环卡住，或者调用协程函数删除任务
    // vTaskDelete(NULL);
}

void app_main(void)
{
    xTaskCreate(task,"task",2048,(void *)&a,1,NULL);
}
