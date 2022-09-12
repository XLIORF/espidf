#include <stdio.h>
#include "lvgl.h"
#include "lvgl_helpers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/semphr.h"


#define LV_TICK_PERIOD_MS 1

// static const char TAG[] = "main";
//进程同步信号
static xSemaphoreHandle xGuiSemaphore;
// static SemaphoreHandle_t xGuiSemaphore;

static void lv_tick_task(void *arg) 
{
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}


static void guiTask(void *pvParameter)
{
    (void) pvParameter;
    
    lv_init();                                      //初始化lvgl
    lvgl_driver_init();                             //底层驱动初始化

    /*配置显示驱动*/
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    static lv_disp_draw_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;
    /* Actual size in pixels, not bytes. */
    size_in_px *= 8;                                //不理解
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, size_in_px);

    lv_disp_drv_t oled_ssd1306;
    lv_disp_drv_init(&oled_ssd1306);
    oled_ssd1306.flush_cb = disp_driver_flush;      //将缓冲区写入到显示器的具体实现
    oled_ssd1306.draw_buf = &disp_buf;                //lv_disp_buf_t类型的缓冲区
    //如果是单色屏还需要配置这两参数
    oled_ssd1306.rounder_cb = disp_driver_rounder;
    oled_ssd1306.clear_cb = disp_driver_set_px;

    lv_disp_drv_register(&oled_ssd1306);

    /*配置输入驱动*/
#if 0

#endif

    /*配置lvgl的时钟驱动*/
     /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
    xSemaphoreGive(xGuiSemaphore);


    /*定期调用lv_task_handler处理图形刷新任务*/
    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }
}

void app_main(void)
{
    // xGuiSemaphore = xSemaphoreCreateMutex();        //初始化同步信号
    vSemaphoreCreateBinary(xGuiSemaphore);

    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
    
    xSemaphoreTake(xGuiSemaphore,portMAX_DELAY);
    lv_obj_t * scr = lv_disp_get_scr_act(NULL);//获取当前活动屏幕

    lv_obj_t * obj = lv_obj_create(scr);//创建基础组件
    lv_obj_set_size(obj,128,64);//大小铺满屏幕
    lv_obj_set_pos(obj,0,0);//设置组件相对父组件的位置

    /*Create a Label on the currently active screen*/
    lv_obj_t * label1 =  lv_label_create(obj);//创建一个标签
    lv_label_set_text(label1, "Hello\nworld");
    lv_obj_align(label1,LV_ALIGN_CENTER, 0, 0);//对其似乎根据文本内容调整
}