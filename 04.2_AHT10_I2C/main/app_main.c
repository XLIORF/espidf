#include <stdio.h>
#include "esp_log.h"
#include "OLED.h"
#include "AHT10.h"

float humity=0,tempareture=0;

void app_main()
{
    OLED_Init();
    OLED_Clear();				//OLED清屏
    OLED_ShowString(1,1,"OLED Init.....OK");
    while (AHT10_State())
    {
        OLED_ShowString(2,1,"AHT10 not Found");
    }//如果AHT10不存在就停在这里
    OLED_Clear();
    AHT10_Soft_Reset();
    AHT10_Init();
    OLED_ShowString(2,1,"Temp:  .  ");
    OLED_Show2103(2,12);//显示摄氏度符号
    OLED_ShowString(3,1,"Humi:  .   %");
    
    while (true)
    {
        AHT10_Get_Humity_Tempareture(&humity,&tempareture);
    
        OLED_ShowNum(2,6,((unsigned int)tempareture)%100,2);
        OLED_ShowNum(2,9,((unsigned int)(tempareture*100))%100,2);
        
        OLED_ShowNum(3,6,((unsigned int)humity)%100,2);
        OLED_ShowNum(3,9,((unsigned int)(humity*100))%100,2);
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    
    
}