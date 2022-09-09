#include <stdio.h>
#include "esp_log.h"
#include "esp_timer.h"


//定时器回调函数
void Timer1Callback(void *args)
{
    ESP_LOGI("Soft Timer","Timer1 Tragger!");//打印日志提示
}

void app_main(void)
{
    esp_timer_handle_t Timer1_Handler;//定时器操作句柄初始化
    esp_timer_init();//软件定时器初始化
    esp_timer_create_args_t timer_arg = {
        .callback = Timer1Callback,//回调函数
        .arg = NULL, //传给回调函数的参数
        .dispatch_method = ESP_TIMER_TASK,//中断调用还是任务调用
        .name = "Soft Timer1",//定时器名字
        .skip_unhandled_events = false//是否跳过未处理的事件，不懂啥意思
    };
    esp_timer_create(&timer_arg,&Timer1_Handler);//创建定时器
    //预装载值的单位是us
    esp_timer_start_periodic(Timer1_Handler,1e6);//这里的句柄不要传地址！！！
}
