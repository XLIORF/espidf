#ifndef __TOUCH_H
#define __TOUCH_H

#include <stdio.h>
#include "driver/touch_pad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

//定义触摸按键序号
#define Touch0  0
#define Touch1  1
#define Touch2  2
#define Touch3  3
#define Touch4  4
#define Touch5  5
#define Touch6  6
#define Touch7  7
#define Touch8  8
#define Touch9  9


//定义触摸按键的GPIO
#define T0 GPIO_NUM_4
#define T1 GPIO_NUM_1
#define T2 GPIO_NUM_2
#define T3 GPIO_NUM_15
#define T4 GPIO_NUM_13
#define T5 GPIO_NUM_12
#define T6 GPIO_NUM_14
#define T7 GPIO_NUM_27
#define T8 GPIO_NUM_33
#define T9 GPIO_NUM_32


//定义触摸按键的通道
#define Touch_Channel0  TOUCH_PAD_NUM0
#define Touch_Channel1  TOUCH_PAD_NUM1
#define Touch_Channel2  TOUCH_PAD_NUM2
#define Touch_Channel3  TOUCH_PAD_NUM3
#define Touch_Channel4  TOUCH_PAD_NUM4
#define Touch_Channel5  TOUCH_PAD_NUM5
#define Touch_Channel6  TOUCH_PAD_NUM6
#define Touch_Channel7  TOUCH_PAD_NUM7
#define Touch_Channel8  TOUCH_PAD_NUM8
#define Touch_Channel9  TOUCH_PAD_NUM9

//定义相关的存储数据结构
gpio_num_t		Touch_Gpio[10];
touch_pad_t		Touch_Channel[10];
uint16_t		Threshold[10];
//函数声明
void touch_init();
void touch_config(uint8_t Touchx);
static void touch_mode_config(uint8_t Touchx);
static void touch_gpio_config(uint8_t Touchx);
static void touch_thresh_config(uint8_t Touchx);
bool touch_state(uint8_t Touchx);
#endif