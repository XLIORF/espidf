#include "lcd.h"
#include "gpio.h"
#include "spi.h"
#include "malloc.h"
#include "delay.h"

#define LOG_TAG "st7735s"

#define LCD_W 128
#define LCD_H 160
#define LCD_DIRECTION 0

static int st7735s_init(lcd_conf_t* conf) {
    if (conf->w == 0) {
        conf->w = LCD_W;
	}
    if (conf->h == 0) {
        conf->h = LCD_H;
	}

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
	
	lcd_write_cmd(conf,0x11);     //Sleep out
	delay_ms(120);                //Delay 120ms
	lcd_write_cmd(conf,0xB1);     //Normal mode
	lcd_write_data(conf,0x05);
	lcd_write_data(conf,0x3C);
	lcd_write_data(conf,0x3C);
	lcd_write_cmd(conf,0xB2);     //Idle mode
	lcd_write_data(conf,0x05);
	lcd_write_data(conf,0x3C);
	lcd_write_data(conf,0x3C);
	lcd_write_cmd(conf,0xB3);     //Partial mode
	lcd_write_data(conf,0x05);
	lcd_write_data(conf,0x3C);
	lcd_write_data(conf,0x3C);
	lcd_write_data(conf,0x05);
	lcd_write_data(conf,0x3C);
	lcd_write_data(conf,0x3C);
	lcd_write_cmd(conf,0xB4);     //Dot inversion
	lcd_write_data(conf,0x03);
	lcd_write_cmd(conf,0xC0);     //AVDD GVDD
	lcd_write_data(conf,0xAB);
	lcd_write_data(conf,0x0B);
	lcd_write_data(conf,0x04);
	lcd_write_cmd(conf,0xC1);     //VGH VGL
	lcd_write_data(conf,0xC5);   //C0
	lcd_write_cmd(conf,0xC2);     //Normal Mode
	lcd_write_data(conf,0x0D);
	lcd_write_data(conf,0x00);
	lcd_write_cmd(conf,0xC3);     //Idle
	lcd_write_data(conf,0x8D);
	lcd_write_data(conf,0x6A);
	lcd_write_cmd(conf,0xC4);     //Partial+Full
	lcd_write_data(conf,0x8D);
	lcd_write_data(conf,0xEE);
	lcd_write_cmd(conf,0xC5);     //VCOM
	lcd_write_data(conf,0x0F);
	lcd_write_cmd(conf,0xE0);     //positive gamma
	lcd_write_data(conf,0x07);
	lcd_write_data(conf,0x0E);
	lcd_write_data(conf,0x08);
	lcd_write_data(conf,0x07);
	lcd_write_data(conf,0x10);
	lcd_write_data(conf,0x07);
	lcd_write_data(conf,0x02);
	lcd_write_data(conf,0x07);
	lcd_write_data(conf,0x09);
	lcd_write_data(conf,0x0F);
	lcd_write_data(conf,0x25);
	lcd_write_data(conf,0x36);
	lcd_write_data(conf,0x00);
	lcd_write_data(conf,0x08);
	lcd_write_data(conf,0x04);
	lcd_write_data(conf,0x10);
	lcd_write_cmd(conf,0xE1);     //negative gamma
	lcd_write_data(conf,0x0A);
	lcd_write_data(conf,0x0D);
	lcd_write_data(conf,0x08);
	lcd_write_data(conf,0x07);
	lcd_write_data(conf,0x0F);
	lcd_write_data(conf,0x07);
	lcd_write_data(conf,0x02);
	lcd_write_data(conf,0x07);
	lcd_write_data(conf,0x09);
	lcd_write_data(conf,0x0F);
	lcd_write_data(conf,0x25);
	lcd_write_data(conf,0x35);
	lcd_write_data(conf,0x00);
	lcd_write_data(conf,0x09);
	lcd_write_data(conf,0x04);
	lcd_write_data(conf,0x10);

	lcd_write_cmd(conf,0xFC);
	lcd_write_data(conf,0x80);

	lcd_write_cmd(conf,0x3A);
	lcd_write_data(conf,0x05);
	lcd_write_cmd(conf,0x36);
    if(conf->direction==0)lcd_write_data(conf,0x08);
    else if(conf->direction==1)lcd_write_data(conf,0xC8);
    else if(conf->direction==2)lcd_write_data(conf,0x78);
    else lcd_write_data(conf,0xA8);
	lcd_write_cmd(conf,0x21);     //Display inversion
	lcd_write_cmd(conf,0x29);     //Display on
	lcd_write_cmd(conf,0x2A);     //Set Column Address
	lcd_write_data(conf,0x00);
	lcd_write_data(conf,conf->xoffset);  //26
	lcd_write_data(conf,0x00);
	lcd_write_data(conf,conf->w+conf->xoffset-1);   //105
	lcd_write_cmd(conf,0x2B);     //Set Page Address
	lcd_write_data(conf,0x00);
	lcd_write_data(conf,conf->yoffset);    //1
	lcd_write_data(conf,0x00);
	lcd_write_data(conf,conf->h+conf->yoffset-1);    //160
	lcd_write_cmd(conf,0x2C);

    lcd_clear(conf,WHITE);
    /* display on */
    lcd_display_on(conf);
    return 0;
};

const lcd_opts_t lcd_opts_st7735s = {
    .name = "st7735s",
    .init = st7735s_init,
};

