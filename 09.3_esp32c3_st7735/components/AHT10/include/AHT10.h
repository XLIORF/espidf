#ifndef __AHT10_H
#define __AHT10_H

#include <stdio.h>
#include "driver/i2c.h"

unsigned char AHT10_State(void);
unsigned char AHT10_Init(void);
unsigned char AHT10_Soft_Reset(void);
unsigned char AHT10_Get_Humity_Tempareture(float *humity, float *tempareture);

#endif