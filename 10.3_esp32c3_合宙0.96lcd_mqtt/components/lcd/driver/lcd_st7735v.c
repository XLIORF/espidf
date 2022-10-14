#include "lcd.h"
#include "gpio.h"
#include "spi.h"
#include "malloc.h"
#include "delay.h"

#define LOG_TAG "st7735v"

#define LCD_W 128
#define LCD_H 160
#define LCD_DIRECTION 0

static int st7735v_init(lcd_conf_t* conf) {
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
    delay_ms(120);//ms

    lcd_write_cmd(conf,0x11);

    delay_ms(120);//ms

    lcd_write_cmd(conf,0x21);

    lcd_write_cmd(conf,0xB1);
    lcd_write_data(conf,0x05);
    lcd_write_data(conf,0x3A);
    lcd_write_data(conf,0x3A);

    lcd_write_cmd(conf,0xB2);
    lcd_write_data(conf,0x05);
    lcd_write_data(conf,0x3A);
    lcd_write_data(conf,0x3A);

    lcd_write_cmd(conf,0xB3);
    lcd_write_data(conf,0x05);
    lcd_write_data(conf,0x3A);
    lcd_write_data(conf,0x3A);
    lcd_write_data(conf,0x05);
    lcd_write_data(conf,0x3A);
    lcd_write_data(conf,0x3A);

    lcd_write_cmd(conf,0xB4);//Dotinversion
    lcd_write_data(conf,0x03);

    lcd_write_cmd(conf,0xC0);
    lcd_write_data(conf,0x62);
    lcd_write_data(conf,0x02);
    lcd_write_data(conf,0x04);

    lcd_write_cmd(conf,0xC1);
    lcd_write_data(conf,0xC0);

    lcd_write_cmd(conf,0xC2);
    lcd_write_data(conf,0x0D);
    lcd_write_data(conf,0x00);

    lcd_write_cmd(conf,0xC3);
    lcd_write_data(conf,0x8D);
    lcd_write_data(conf,0x6A);

    lcd_write_cmd(conf,0xC4);
    lcd_write_data(conf,0x8D);
    lcd_write_data(conf,0xEE);

    lcd_write_cmd(conf,0xC5);//VCOM
    lcd_write_data(conf,0x0E);

    lcd_write_cmd(conf,0xE0);
    lcd_write_data(conf,0x10);
    lcd_write_data(conf,0x0E);
    lcd_write_data(conf,0x02);
    lcd_write_data(conf,0x03);
    lcd_write_data(conf,0x0E);
    lcd_write_data(conf,0x07);
    lcd_write_data(conf,0x02);
    lcd_write_data(conf,0x07);
    lcd_write_data(conf,0x0A);
    lcd_write_data(conf,0x12);
    lcd_write_data(conf,0x27);
    lcd_write_data(conf,0x37);
    lcd_write_data(conf,0x00);
    lcd_write_data(conf,0x0D);
    lcd_write_data(conf,0x0E);
    lcd_write_data(conf,0x10);


    lcd_write_cmd(conf,0xE1);
    lcd_write_data(conf,0x10);
    lcd_write_data(conf,0x0E);
    lcd_write_data(conf,0x03);
    lcd_write_data(conf,0x03);
    lcd_write_data(conf,0x0F);
    lcd_write_data(conf,0x06);
    lcd_write_data(conf,0x02);
    lcd_write_data(conf,0x08);
    lcd_write_data(conf,0x0A);
    lcd_write_data(conf,0x13);
    lcd_write_data(conf,0x26);
    lcd_write_data(conf,0x36);
    lcd_write_data(conf,0x00);
    lcd_write_data(conf,0x0D);
    lcd_write_data(conf,0x0E);
    lcd_write_data(conf,0x10);

    lcd_write_cmd(conf,0x3A);
    lcd_write_data(conf,0x05);

    // lcd_write_cmd(conf,0x36);
    // lcd_write_data(conf,0xC8);

    lcd_write_cmd(conf,0x36);
    if(conf->direction==0)lcd_write_data(conf,0xC8);
    else if(conf->direction==1)lcd_write_data(conf,0x78);
    else if(conf->direction==2)lcd_write_data(conf,0x08);
    else lcd_write_data(conf,0xA8);

    lcd_write_cmd(conf,0x29);

    /* wait for power stability */
    delay_ms(100);
    lcd_clear(conf,WHITE);
    /* display on */
    lcd_display_on(conf);
    return 0;
};

const lcd_opts_t lcd_opts_st7735v = {
    .name = "st7735v",
    .init = st7735v_init,
};

