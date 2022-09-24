/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static char TAG[] = "UI";
char text[64];

/*
lvgl的事件回调里添加了设置对象风格和获得对象参数的语句，而这会触发新一轮的事件回调，由于此时函数还没有退出并且没有加预防的代码，回调不断递归，导致内存溢出
*/
static void example_event_cb(lv_event_t *e)
{
    unsigned int * key_id = lv_event_get_param(e);
    // lv_obj_t *obj = lv_event_get_target(e);                  // 获取触发事件的对象
    // lv_obj_t *parent = lv_event_get_current_target(e);       // 获取触发事件对象的父对象(事件冒泡才有)
    lv_event_code_t code = lv_event_get_code(e);             // 获取当前部件触发的事件代码
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e); // 获取添加事件时传递的用户数据, 通过lv_obj_add_event_cb的第4个参数传入
    ESP_LOGI(TAG, "KEY_ID:%u", *key_id);
    if (code != LV_EVENT_STYLE_CHANGED && code != LV_EVENT_GET_SELF_SIZE)
    {
        if (*key_id == 1U)
        {
            lv_label_set_text(label, "Right");
            lv_obj_align(label, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        }
        else if (*key_id == 2U)
        {
            lv_label_set_text(label, "Left");
            lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        }
        else if (*key_id == 3U)
        {
            lv_label_set_text(label, "Left and Right");
            lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);
        }
        else
            lv_label_set_text(label, " ");
    }
}

void example_lvgl_demo_ui(void *paragrame)
{
    lv_disp_t *disp = (lv_disp_t *)paragrame;
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_t *label2 = lv_label_create(scr);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */

    lv_group_add_obj(lv_group_get_default(), scr);
    lv_obj_add_event_cb(scr, example_event_cb, LV_EVENT_KEY, (void *)label2);

    lv_label_set_text(label, "Hello Espressif, Hello LVGL.");
    lv_obj_set_width(label, 150);
    lv_obj_set_height(label, 32);
    lv_obj_add_flag(label, LV_OBJ_FLAG_EVENT_BUBBLE); //冒泡传递
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    lv_label_set_text(label2, "Unset");
    lv_obj_align(label2, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_width(label, 128);//加这行显示不完全
    vTaskDelete(NULL);
}
