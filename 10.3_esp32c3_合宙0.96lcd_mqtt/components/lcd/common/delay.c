#include "delay.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

int get_sys_ms(void)
{
    return xTaskGetTickCount() * portTICK_RATE_MS;
}

int delay_ms(int ms){
    if (ms > 0) {
        vTaskDelay(ms / portTICK_RATE_MS);
    }
    return 0;
}

