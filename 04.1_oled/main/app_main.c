#include <stdio.h>
#include "OLED.h"
#include "i2c_bus0.h"

void app_main()
{
    OLED_Init();
    OLED_ShowString(2,1,"      Hello     ");
    OLED_ShowString(3,1,"      World     ");
}
