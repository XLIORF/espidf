extern "C"
{
#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/sockets.h"
#include "OLED.h"
#include "string.h"
#include "freertos/event_groups.h"
#include "mqtt_bemfa.h"


#define wifi_ssid  "TP-LINK_221"
#define wifi_password  "221000221x"

static char TAG[] = "main";

EventGroupHandle_t user_EventGroup = NULL; 

static void net_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    // ESP_LOGI("event handler", "----> event_base: %s event_id: %d", event_base, event_id);
    if(event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "event_handler:开始wifi连接");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "event_handler:wifi连接断开");
            break;
        default:
            ESP_LOGI(TAG, "event_handler:未知wifi事件");
            break;
        }
    }
    else if(event_base == IP_EVENT) 
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t*)event_data;
        switch(event_id) 
        {
            case IP_EVENT_STA_GOT_IP:
                xEventGroupSetBits(user_EventGroup,1);
                break;
            default:
                break;
        }
    } 
    else 
        ESP_LOGI(TAG, "未知事件");
}

void nvs_init(void)
{
    esp_err_t err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void wifi_connect(void)
{
    //事件循环,不加连不上网
    esp_event_loop_create_default();
    esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&net_event_handler,NULL);//注册wifi事件处理函数
    esp_event_handler_register(IP_EVENT,ESP_EVENT_ANY_ID,&net_event_handler,NULL);  //注册ip时间处理函数
    //网络接口初始化
    esp_netif_init();
    esp_netif_create_default_wifi_sta();
    //wifi初始化
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    //目标WiFi配置
    wifi_config_t wifi_config = {
        .sta.ssid = wifi_ssid,
        .sta.password = wifi_password
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA,&wifi_config);
    //设置wifi模式
    esp_wifi_set_mode(WIFI_MODE_STA);
    //启动WiFi
    esp_wifi_start();
    //开始连接
    esp_wifi_connect();
}


void app_main(void)
{
    OLED_Init();
    // 初始化NVS
    nvs_init();
    user_EventGroup = xEventGroupCreate();
    wifi_connect();//概率触发掉电检测
    mqtt_bemfa mqtt;
    while(mqtt.connect() == false)
    {
        mqtt = mqtt_bemfa("c8c9d678bd2847e5e270c094286291a5","119.91.109.180",8344);
    }
    mqtt.subscribe("test");
    int counter = 3000;
    while (1)
    {
        mqtt.recv();
        counter--;
        if (counter == 0)
        {
             mqtt.loop();
             counter = 3000;
        }
    }
}
}