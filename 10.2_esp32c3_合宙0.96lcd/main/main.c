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
#include "freertos/event_groups.h"
#include "mqtt_client.h"

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
EventGroupHandle_t wifi_notice = NULL;
void simplewificonnect(void);


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "test1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "test0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "test1", 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "test1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "test0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        lcd_set_font(lcd_conf, u8g2_font_opposansm12);
        lcd_draw_str(lcd_conf, 20, 20, event->data, 0x0CE0);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


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

// void display_update(void)
// {
//     switch(display_index)
//     {
//         case 0:
//             ESP_LOGI(TAG, "draw horizontal line");
//             lcd_draw_line(lcd_conf, 0, 5, 150, 5, 0x001F);
//             break;
//         case 1:
//             ESP_LOGI(TAG, "draw vertical line");
//             lcd_draw_line(lcd_conf, 5, 0, 5, 100, 0x001F);
//             break;
//         case 2:
//             ESP_LOGI(TAG, "draw rectangle");
//             lcd_draw_rectangle(lcd_conf, 10, 10, 150, 70, 0xF800);
//             break;
//         case 3:
//             ESP_LOGI(TAG, "draw circle");
//             lcd_draw_circle(lcd_conf, 40, 40, 40, 0x0CE0);
//             break;
//         case 4:
//             ESP_LOGI(TAG, "draw str, size 12");
//             lcd_set_font(lcd_conf, u8g2_font_opposansm12);
//             lcd_draw_str(lcd_conf, 20, 20, "hello", 0x0CE0);
//             break;
//         case 5:
//             ESP_LOGI(TAG, "draw chinese str, size 12");
//             lcd_set_font(lcd_conf, u8g2_font_opposansm12_chinese);
//             lcd_draw_str(lcd_conf, 20, 40, "hello     小康师兄", 0x0CE0);
//             break;
//         case 6:
//             ESP_LOGI(TAG, "draw chinese str, size 24");
//             lcd_set_font(lcd_conf, u8g2_font_opposansm24_chinese);
//             lcd_draw_str(lcd_conf, 20, 70, "小康师兄", 0x0CE0);
//             break;
//         case 7:
//             ESP_LOGI(TAG, "draw qr code");
//             // lcd_clear(lcd_conf, 0x0);
//             lcd_set_font(lcd_conf, u8g2_font_opposansm8);
//             lcd_draw_qr_code(lcd_conf, 0, 0, "kangweijian", 80);
//             break;
//         case 8:
//             // lcd_draw_xbm(lcd_conf, 30, 30, ......);
//             break;
//     }
// }




int8_t read_button(int pin)
{
    if(gpio_get_level(pin)==0)
    {
        uint32_t tick = get_sys_ms();
        for(int i=0; i<300; i++)
        {
            delay_ms(10);
            if(gpio_get_level(pin)!=0)
                break;
        }
        if(get_sys_ms()>tick+30){
            ESP_LOGI(TAG, "按键短按 %d", pin);
            return 1;
        }
        else
            return 2;
    }
    else
        return 0;
}

static void main_task(void *pvParameters)
{
    display_init();

    while(1)
    {
        delay_ms(10);
        if(read_button(BUTTON_UP) != 0)
        {
            esp_mqtt_client_publish(client, "led", "on", 0, 0, 0);
        }
        // read_button(BUTTON_RIGHT);
        if(read_button(BUTTON_DOWN) != 0)
        {
            esp_mqtt_client_publish(client, "led", "off", 0, 0, 0);
        }
        // read_button(BUTTON_LEFT);
        // read_button(BUTTON_CENTER);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    button_init();
    wifi_notice = xEventGroupCreate();
    esp_err_t err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);
    simplewificonnect();
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://bemfa.com:9501",//从例程复制过来能连接上，但是报错
        .client_id = "c8c9d678bd2847e5e270c094286291a5",//巴法云mqtt接入文档要求
        // .host = "bemfa.com",//报一样的错，但是能连接上
        // .port = 9501,
        // .transport = MQTT_TRANSPORT_OVER_TCP,
    };
    xEventGroupWaitBits(wifi_notice,1,pdTRUE,pdTRUE,portMAX_DELAY);
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    xTaskCreate(main_task, "main_task", 4096, NULL, 5, NULL);
}
