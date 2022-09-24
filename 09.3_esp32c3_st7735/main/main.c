#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "AHT10.h"
#include "lvgl.h"
#include "lvgl_helpers.h"

// static const char TAG[] = "demo";
#define LV_TICK_PERIOD_MS 1

static void lv_tick_task(void *arg);//lvgl需要的，告诉gui过了多久
static void guiTask(void *pvParameter);//处理绘图任务
static void data_server(void *pvParameter); //需要图像显示的函数，拿到温湿度信息
void test_event_cb(lv_obj_t* obj, lv_event_t event);

bool disp_flag = 1;
const char key_disp_str[4][5] = {"UP","DOWN","OK","Back"};
const char *key_disp = NULL;

void app_main() 
{
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
}


SemaphoreHandle_t xGuiSemaphore;
lv_indev_t *keyreg;
static void guiTask(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();
    lvgl_driver_init();
   
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    /* Use double buffered when not working with monochrome displays */
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);
#else
    static lv_color_t *buf2 = NULL;
#endif

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

#if defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_IL3820         \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_JD79653A    \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_UC8151D     \
    || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_SSD1306

    /* Actual size in pixels, not bytes. */
    size_in_px *= 8;
#endif

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    /* When using a monochrome display we need to register the callbacks:
     * - rounder_cb
     * - set_px_cb */
#ifdef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
#endif

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Register an input device when enabled on the menuconfig */
#if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

#if 0
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = esp32_board_touch_read;
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    keyreg = lv_indev_drv_register(&indev_drv);
#endif

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */
    xTaskCreatePinnedToCore(data_server, "data_server", 2048, NULL, 0, NULL, 1);
    // create_demo_application();

    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    free(buf1);
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    free(buf2);
#endif
    vTaskDelete(NULL);
}

static void data_server(void *pvParameter)
{
    (void) pvParameter;
    float humity,temperature;
    char htdata[] = "Temp:00.00 C\nHumi:00.00 ";

    AHT10_Soft_Reset();
    AHT10_Init();

    lv_obj_t * scr = lv_disp_get_scr_act(NULL);//获取当前活动屏幕

    lv_obj_t * obj = lv_obj_create(scr,NULL);//创建基础组件
    lv_obj_set_size(obj,128,64);//大小铺满屏幕
    lv_obj_set_pos(obj,0,0);//设置组件相对父组件的位置

    // lv_group_t * keyingroup = lv_group_create();//初始化输入组
    // lv_indev_set_group(keyreg,keyingroup);//关联输入设备和组
    // lv_group_add_obj(keyingroup,obj);//添加组件到组中
    // lv_obj_set_event_cb(obj,test_event_cb);//设置组件的回调函数

    /*Create a Label on the currently active screen*/
    lv_obj_t * label1 =  lv_label_create(obj, NULL);//创建一个标签
    lv_label_set_text(label1, "Hello\nworld");
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);//对其似乎根据文本内容调整
    // lv_group_add_obj(keyingroup,label1);//添加组件到组中
    lv_obj_set_event_cb(label1,test_event_cb);//设置组件的回调函数
    lv_task_handler();//刷新显示
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    
    lv_label_set_text(label1, htdata);
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);//对其似乎根据文本内容调整
    lv_task_handler();//刷新显示

    
    while(true)
    {
        // lv_event_send(label1,LV_EVENT_PRESSED,NULL);
        vTaskDelay(pdMS_TO_TICKS(10));
        if(disp_flag == 1)
        {
            AHT10_Get_Humity_Tempareture(&humity,&temperature);
            htdata[5] = (int)(temperature) / 10 % 10 + 0x30;
            htdata[6] = (int)(temperature) % 10 + 0x30;
            htdata[8] = (int)(temperature * 10 ) % 10 + 0x30;
            htdata[9] = (int)(temperature * 100) % 10 + 0x30;
            
            htdata[18] = (int)(humity) / 10 % 10 + 0x30;
            htdata[19] = (int)(humity) % 10 + 0x30;
            htdata[21] = (int)(humity * 10) % 10 + 0x30;
            htdata[22] = (int)(humity * 100) % 10 + 0x30;

            lv_label_set_text(label1, htdata);//先设置文本再设置对齐
        }
        else
        {
            lv_label_set_text(label1,key_disp);//先设置文本再设置对齐
        }
        
        // lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);//对其似乎根据文本内容调整
    }
    vTaskDelete(NULL);
}

void test_event_cb(lv_obj_t* obj, lv_event_t event)
{
    const uint32_t *key_id = lv_event_get_data();
    switch (*key_id)
    {
    case 0x00000001:
        disp_flag = 0;
        key_disp = key_disp_str[0];
        break;
    
    case 32:
        disp_flag = 0;
        key_disp = key_disp_str[1];
        break;
    case 3:
        disp_flag = 0;
        key_disp = key_disp_str[2];
        break;
    case 4:
        disp_flag = 0;
        key_disp = key_disp_str[3];
        break;
    
    default:
        disp_flag = 1;
        break;
    }
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}
