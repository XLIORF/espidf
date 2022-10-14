#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "delay.h"
#include "spi.h"
#include "gpio.h"
#include "lcd.h"
#include "u8g2.h"

#define TAG                 "display"

#define BUTTON_LEFT          5  //9
#define BUTTON_UP            8
#define BUTTON_CENTER        4
#define BUTTON_DOWN          13 //5
#define BUTTON_RIGHT         9  //13
#define BUTTON_PIN_SEL  ((1ULL<<BUTTON_UP) | (1ULL<<BUTTON_DOWN) | (1ULL<<BUTTON_LEFT) | (1ULL<<BUTTON_RIGHT) | (1ULL<<BUTTON_CENTER))

#define SPI_BUS_ID          2
#define SPI_CS              7
#define LCD_DC              6
#define LCD_BL              11
#define LCD_RST             10

spi_device_t *spi_device = NULL;
lcd_conf_t *lcd_conf = NULL;
int display_index = 0;

void button_init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = BUTTON_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);
}

void display_init(void)
{
    spi_device = spi_init(SPI_BUS_ID, SPI_CS, 0, 0, 8, 40000000, MSB, 1, 1);

    lcd_conf = lcd_setup(spi_device, "st7735v", LCD_DC, LCD_BL, LCD_RST, 1, 160, 80, 0, 24);

    // 如果显示颜色相反，关闭反色
    // lcd_inv_off(lcd_conf);
}

void display_clear(void)
{
    lcd_clear(lcd_conf, 0xffff);
}

void display_update(void)
{
    switch(display_index)
    {
        case 0:
            ESP_LOGI(TAG, "draw horizontal line");
            lcd_draw_line(lcd_conf, 0, 5, 150, 5, 0x001F);
            break;
        case 1:
            ESP_LOGI(TAG, "draw vertical line");
            lcd_draw_line(lcd_conf, 5, 0, 5, 100, 0x001F);
            break;
        case 2:
            ESP_LOGI(TAG, "draw rectangle");
            lcd_draw_rectangle(lcd_conf, 10, 10, 150, 70, 0xF800);
            break;
        case 3:
            ESP_LOGI(TAG, "draw circle");
            lcd_draw_circle(lcd_conf, 40, 40, 40, 0x0CE0);
            break;
        case 4:
            ESP_LOGI(TAG, "draw str, size 12");
            lcd_set_font(lcd_conf, u8g2_font_opposansm12);
            lcd_draw_str(lcd_conf, 20, 20, "hello", 0x0CE0);
            break;
        case 5:
            ESP_LOGI(TAG, "draw chinese str, size 12");
            lcd_set_font(lcd_conf, u8g2_font_opposansm12_chinese);
            lcd_draw_str(lcd_conf, 20, 40, "hello     小康师兄", 0x0CE0);
            break;
        case 6:
            ESP_LOGI(TAG, "draw chinese str, size 24");
            lcd_set_font(lcd_conf, u8g2_font_opposansm24_chinese);
            lcd_draw_str(lcd_conf, 20, 70, "小康师兄", 0x0CE0);
            break;
        case 7:
            ESP_LOGI(TAG, "draw qr code");
            // lcd_clear(lcd_conf, 0x0);
            lcd_set_font(lcd_conf, u8g2_font_opposansm8);
            lcd_draw_qr_code(lcd_conf, 0, 0, "kangweijian", 80);
            break;
        case 8:
            // lcd_draw_xbm(lcd_conf, 30, 30, ......);
            break;
    }
}

void display_add(void)
{
    display_update();
    display_index++;
    if(display_index>8)
    {
        ESP_LOGI(TAG, "lcd clear");
        lcd_clear(lcd_conf, 0xffff);
        display_index = 0;
    }
}

void display_dec(void)
{
    display_update();
    display_index--;
    if(display_index<0)
    {
        ESP_LOGI(TAG, "lcd clear");
        lcd_clear(lcd_conf, 0xffff);
        display_index = 0;
    }
}

void read_button(int pin, void func(void))
{
    if(gpio_get_level(pin)==0){
        uint32_t tick = get_sys_ms();
        for(int i=0; i<300; i++){
            delay_ms(10);
            if(gpio_get_level(pin)!=0)
                break;
        }
        if(get_sys_ms()>tick+30){
            ESP_LOGI(TAG, "按键短按 %d", pin);
            func();
        }
    }
}

static void main_task(void *pvParameters)
{
    display_init();

    while(1)
    {
        delay_ms(10);
        read_button(BUTTON_UP, display_add);
        read_button(BUTTON_RIGHT, display_add);
        read_button(BUTTON_DOWN, display_dec);
        read_button(BUTTON_LEFT, display_dec);
        read_button(BUTTON_CENTER, display_clear);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    button_init();
    nvs_flash_init();
    xTaskCreate(main_task, "main_task", 4096, NULL, 5, NULL);    
}
