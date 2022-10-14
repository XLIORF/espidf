#ifndef GPIO_H
#define GPIO_H


#include "stdint.h"

typedef int (*gpio_irq_cb)(int pin, void* args);

#define GPIO_LOW                 0x00
#define GPIO_HIGH                0x01

#define GPIO_OUTPUT         0x00
#define GPIO_INPUT          0x01
#define GPIO_IRQ            0x02

#define GPIO_DEFAULT        0x00
#define GPIO_PULLUP         0x01
#define GPIO_PULLDOWN       0x02

#define GPIO_RISING             0x00
#define GPIO_FALLING            0x01
#define GPIO_BOTH               0x02
#define GPIO_HIGH_IRQ			0x03	//高电平中断
#define GPIO_LOW_IRQ			0x04	//低电平中断

#define GPIO_MAX_ID             255

typedef struct gpio
{
    int pin;
    int mode;
    int pull;
    int irq;
    int lua_ref;
    gpio_irq_cb irq_cb;
    void* irq_args;
} gpio_t;


void gpio_mode(int pin, int mode, int pull, int initOutput);
int gpio_setup(gpio_t* gpio);
int gpio_set(int pin, int level);
int gpio_get(int pin);
void gpio_close(int pin);

int gpio_irq_default(int pin, void* args);

int gpio_set_irq_cb(int pin, gpio_irq_cb cb, void* args);

// 在同一个GPIO输出一组脉冲, 注意, len的单位是bit, 高位在前.
void gpio_pulse(int pin, uint8_t *level, uint16_t len, uint16_t delay_ns);

#endif
