#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

// 软件计时器周期
#define SFT_TIMER_PERIOD_MS (1000 / portTICK_PERIOD_MS)

// 任务名称
const char* tasak_name = "sft_timer_task";
// 软件计时器名称
const char* timer_name = "sft_timer";
// 软件计时器
TimerHandle_t sft_timer_handle;

int counts = 1;

void software_timer_callback(TimerHandle_t timer);

void app_main(void)
{
    printf("Hello world!\n");

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


    // 创建软件计时器
    sft_timer_handle = xTimerCreate(timer_name,     /*计时器名称*/
                        SFT_TIMER_PERIOD_MS,        /*计时器周期*/
                        pdTRUE,                     /*自动重载*/
                        0,                          /*计时器ID,默认不使用*/
                        software_timer_callback     /*计时器回调函数*/
                        );
    // 启动计时器
    xTimerStart(sft_timer_handle,0);
}

void software_timer_callback(TimerHandle_t timer){
    printf("timer task %s is running.%d\n",tasak_name,counts);
    counts++;
}
