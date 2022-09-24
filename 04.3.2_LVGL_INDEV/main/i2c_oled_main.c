/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "oled_main";

#define I2C_HOST 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OLED_PIXEL_CLOCK_HZ (400 * 1000)
#define PIN_NUM_SDA 1
#define PIN_NUM_SCL 0
#define PIN_NUM_RST -1
#define OLED_I2C_HW_ADDR 0x3C

// The pixel number in horizontal and vertical
#define OLED_H_RES 128
#define OLED_V_RES 64
// Bit number used to represent command and parameter
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
//输入按键引脚号
#define KEY_LEFT GPIO_NUM_17
#define KEY_RIGHT GPIO_NUM_16

#define LVGL_TICK_PERIOD_MS 2

extern void example_lvgl_demo_ui(lv_disp_t *disp);

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

uint key_read_filter()
{
    uint id = 0,id2=0;
    id = (!gpio_get_level(KEY_LEFT)) << 1 | (!gpio_get_level(KEY_RIGHT));
    vTaskDelay(5/portTICK_PERIOD_MS);//消抖
    id2 = (!gpio_get_level(KEY_LEFT)) << 1 | (!gpio_get_level(KEY_RIGHT));
    if (id != id2)
    {
        vTaskDelay(20/portTICK_PERIOD_MS);//消抖
        return (!gpio_get_level(KEY_LEFT)) << 1 | (!gpio_get_level(KEY_RIGHT));
    }
    return id;
}

static bool hardware_keyin_read_cb(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    uint id = 0;
    id = key_read_filter();
    if (data->key != id)
    {
        if (id == 0)
        {
            data->key = 0;
            data->state = LV_INDEV_STATE_RELEASED;
        }
        else
        {
            data->state = LV_INDEV_STATE_PRESSED;
            ESP_LOGI(TAG, "id=%d |LEFT:%d |RIGHT:%d |state: %d", id, id >> 1, id & 0x01,data->state);
        }
        data->key = id;
    }
    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

static void lvgl_set_px_cb(lv_disp_drv_t *disp_drv, uint8_t *buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
                           lv_color_t color, lv_opa_t opa)
{
    uint16_t byte_index = x + ((y >> 3) * buf_w);
    uint8_t bit_index = y & 0x7;

    if ((color.full == 0) && (LV_OPA_TRANSP != opa))
    {
        buf[byte_index] |= (1 << bit_index);
    }
    else
    {
        buf[byte_index] &= ~(1 << bit_index);
    }
}

static void lvgl_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    area->y1 = area->y1 & (~0x7);
    area->y2 = area->y2 | 0x7;
}

static void lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void app_main(void)
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions
    static lv_indev_drv_t hardware_key_dev;

    ESP_LOGI(TAG, "Initialize key-in gpio");
    gpio_config_t keyin_left = {
        .pin_bit_mask = 1ULL << KEY_LEFT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&keyin_left);
    gpio_config_t keyin_right = {
        .pin_bit_mask = 1ULL << KEY_RIGHT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&keyin_right);

    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_NUM_SDA,
        .scl_io_num = PIN_NUM_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = OLED_PIXEL_CLOCK_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_HOST, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_HOST, I2C_MODE_MASTER, 0, 0, 0));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = OLED_I2C_HW_ADDR,
        .control_phase_bytes = 1,       // According to SSD1306 datasheet
        .dc_bit_offset = 6,             // According to SSD1306 datasheet
        .lcd_cmd_bits = LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = LCD_CMD_BITS, // According to SSD1306 datasheet
        .on_color_trans_done = notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = PIN_NUM_RST,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = malloc(OLED_H_RES * 20 * sizeof(lv_color_t));
    assert(buf1);
    lv_color_t *buf2 = malloc(OLED_H_RES * 20 * sizeof(lv_color_t));
    assert(buf2);
    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, OLED_H_RES * 20);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = OLED_H_RES;
    disp_drv.ver_res = OLED_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    disp_drv.rounder_cb = lvgl_rounder;
    disp_drv.set_px_cb = lvgl_set_px_cb;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Register input driver to LVGL");
    lv_indev_drv_init(&hardware_key_dev);
    hardware_key_dev.type = LV_INDEV_TYPE_KEYPAD;
    hardware_key_dev.read_cb = hardware_keyin_read_cb;
    // hardware_key_dev.feedback_cb =
    // hardware_key_dev.user_data =
    // hardware_key_dev.disp = disp_drv;
    // hardware_key_dev.read_timer = keyin_timer;
    hardware_key_dev.long_press_time = 2500;
    hardware_key_dev.long_press_repeat_time = 300;
    lv_indev_t *indev = lv_indev_drv_register(&hardware_key_dev);
    lv_group_t *hardware_key_group = lv_group_create();
    lv_indev_set_group(indev, hardware_key_group);
    lv_group_set_default(hardware_key_group); //设置成默认组
    // lv_point_t key_map_point[2] = {
    //     {6, 5}, {122, 5}};
    // lv_indev_set_button_points(&indev, key_map_point);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "Display LVGL Scroll Text");
    xTaskCreate(example_lvgl_demo_ui,"example_lvgl_demo_ui",1024*4,(void *)disp,1,NULL);

    while (1)
    {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}

