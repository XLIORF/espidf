#include "lcd.h"
#include "gpio.h"
#include "spi.h"
#include "malloc.h"
#include "delay.h"

#define LOG_TAG "gc9106l"

#define LCD_W 128
#define LCD_H 160
#define LCD_DIRECTION 0

static int gc9106l_init(lcd_conf_t* conf) {
    if (conf->w == 0)
        conf->w = LCD_W;
    if (conf->h == 0)
        conf->h = LCD_H;
    if (conf->direction == 0)
        conf->direction = LCD_DIRECTION;

    if (conf->pin_pwr != 255)
        gpio_mode(conf->pin_pwr, GPIO_OUTPUT, GPIO_DEFAULT, GPIO_LOW); // POWER
    gpio_mode(conf->pin_dc, GPIO_OUTPUT, GPIO_DEFAULT, GPIO_HIGH); // DC
    gpio_mode(conf->pin_rst, GPIO_OUTPUT, GPIO_DEFAULT, GPIO_LOW); // RST

    if (conf->pin_pwr != 255)
        gpio_set(conf->pin_pwr, GPIO_LOW);
    gpio_set(conf->pin_rst, GPIO_LOW);
    delay_ms(100);
    gpio_set(conf->pin_rst, GPIO_HIGH);
    delay_ms(120);
    // 发送初始化命令
    //------------------------------------gc9106lS Frame Rate-----------------------------------------//
    lcd_write_cmd(conf,0xFE);
    lcd_write_cmd(conf,0xEF);
    lcd_write_cmd(conf,0xB3);
    lcd_write_data(conf,0x03);

    lcd_write_cmd(conf,0x21);
    lcd_write_cmd(conf,0x36);
    if(conf->direction==0)lcd_write_data(conf,0xC8);
    else if(conf->direction==1)lcd_write_data(conf,0x08);
    else if(conf->direction==2)lcd_write_data(conf,0x68);
    else lcd_write_data(conf,0xA8);

    lcd_write_cmd(conf,0x3A);
    lcd_write_data(conf,0x05);
    lcd_write_cmd(conf,0xB4);
    lcd_write_data(conf,0x21);

    lcd_write_cmd(conf,0xF0);
    lcd_write_data(conf,0x2D);
    lcd_write_data(conf,0x54);
    lcd_write_data(conf,0x24);
    lcd_write_data(conf,0x61);
    lcd_write_data(conf,0xAB);
	lcd_write_data(conf,0x2E);
    lcd_write_data(conf,0x2F);
    lcd_write_data(conf,0x00);
    lcd_write_data(conf,0x20);
    lcd_write_data(conf,0x10);
    lcd_write_data(conf,0X10);
    lcd_write_data(conf,0x17);
    lcd_write_data(conf,0x13);
    lcd_write_data(conf,0x0F);

    lcd_write_cmd(conf,0xF1);
    lcd_write_data(conf,0x02);
    lcd_write_data(conf,0x22);
    lcd_write_data(conf,0x25);
    lcd_write_data(conf,0x35);
    lcd_write_data(conf,0xA8);
	lcd_write_data(conf,0x08);
    lcd_write_data(conf,0x08);
    lcd_write_data(conf,0x00);
    lcd_write_data(conf,0x00);
    lcd_write_data(conf,0x09);
    lcd_write_data(conf,0X09);
    lcd_write_data(conf,0x17);
    lcd_write_data(conf,0x18);
    lcd_write_data(conf,0x0F);

    lcd_write_cmd(conf,0xFE);
    lcd_write_cmd(conf,0xFF);

    /* Sleep Out */
    lcd_write_cmd(conf,0x11);
    /* wait for power stability */
    delay_ms(100);
    lcd_clear(conf,WHITE);
    /* display on */
    lcd_display_on(conf);
    return 0;
};

const lcd_opts_t lcd_opts_gc9106l = {
    .name = "gc9106l",
    .init = gc9106l_init,
};

