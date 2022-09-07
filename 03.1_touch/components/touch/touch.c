#include <stdio.h>
#include "touch.h"

//定义相关的存储数据结构
extern gpio_num_t		Touch_Gpio[10] ={T0,T1,T2,T3,T4,T5,T6,T7,T8,T9}; //引脚
extern touch_pad_t		Touch_Channel[10] ={Touch_Channel0,Touch_Channel1,Touch_Channel2,Touch_Channel3,Touch_Channel4,Touch_Channel5,Touch_Channel6,Touch_Channel7,Touch_Channel8,Touch_Channel9}; //通道
extern uint16_t		Threshold[10] = { 0,0,0,0,0,0,0,0,0,0 }; //用来存储阈值

/**
*  @brief 开启触摸按键功能
**/
void touch_init()
{
	//初始化传感器驱动程序
	touch_pad_init();
}



/**
*  @brief 配置触摸按键通道，一共T0-T9 10个通道，这里只定义了T0和T7
*  @param[in] Touchx: Touch[0..9] 使用第几个触摸按键
**/
void touch_config(uint8_t Touchx)
{
	touch_mode_config(Touchx); //初始化触摸按键
	touch_gpio_config(Touchx); //初始化相关的gpio
	touch_thresh_config(Touchx); //初始化阈值

}


//设置触摸按键模式
static void touch_mode_config(uint8_t Touchx)
{
	

	//01 设置检测电压范围0-2.7V
	touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V);

	//02 设置触发模式是硬件定时器触发
	touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);

	//03 载入配置,设置阈值0就是不设置中断
	touch_pad_config(Touch_Channel[Touchx], 0);

	//04 开启滤波器，如果不开滤波器，读取脉冲数方法不一样
	touch_pad_filter_start(10); 

}

//设置触摸按键相关的gpio
static void touch_gpio_config(uint8_t Touchx)
{
	gpio_pad_select_gpio(Touch_Gpio[Touchx]);
	gpio_set_direction(Touch_Gpio[Touchx], GPIO_MODE_OUTPUT);
}

//设置触摸按键的检测阈值
//手摸按键周期会变长，脉冲数会减少
static void touch_thresh_config(uint8_t Touchx)
{
	//读取初始化没有触摸时候的脉冲数
	uint16_t value;
	
	touch_pad_read_filtered(Touch_Channel[Touchx], &value);
	

	//这个脉冲数的2/3作为阈值
	Threshold[Touchx] = value * 2 / 3;

	
}

/**
*  @brief 检测触摸按键是否被按键
*  @param[in] Touchx: Touch[0..9] 使用第几个触摸按键
*  @retval true:按下  false:没有按下
**/
bool touch_state(uint8_t Touchx)
{
	//用轮询的方法进行检测
	uint16_t value;
	touch_pad_read_filtered(Touch_Channel[Touchx], &value);
	if (value < Threshold[Touchx])
	{
		vTaskDelay(2/ portTICK_PERIOD_MS);
		touch_pad_read_filtered(Touch_Channel[Touchx], &value);
		if (value < Threshold[Touchx])
		{
			while (value < Threshold[Touchx])
			{
				touch_pad_read_filtered(Touch_Channel[Touchx], &value);
			}

			return true;
		}
		
			
		
	}

	return false;
}

