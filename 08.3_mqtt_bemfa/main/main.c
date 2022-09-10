#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "string.h"

bool wifi_exist_flag = false;
#define campus_env 1
#if home_env
    #define my_wifi_ssid "ChinaUnicom-E5DFNM"
    #define my_wifi_password "123456789"
#elif campus_env
    #define my_wifi_ssid "TP-LINK_221"
    #define my_wifi_password "221000221x"
#endif
static void net_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ESP_LOGI("event handler", "----> event_base: %s event_id: %d", event_base, event_id);
    if(event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
            ESP_LOGI("event handler", "----> %s", "WIFI_EVENT_STA_START");
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI("event handler", "----> %s", "WIFI_EVENT_STA_DISCONNECTED");
            break;
        default:
            break;
        }
    }
    else if(event_base == IP_EVENT) 
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t*)event_data;
        switch(event_id) 
        {
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI("event handler", "----> IP_EVENT_STA_GOT_IP ip:");
                ESP_LOGI("event handler", "----> sta ip:"IPSTR, IP2STR(&event->ip_info.ip));
                ESP_LOGI("event handler", "----> mask:"IPSTR, IP2STR(&event->ip_info.netmask));
                ESP_LOGI("event handler", "----> gw:"IPSTR, IP2STR(&event->ip_info.gw));
                break;
            default:
                break;
        }
    } else 
    {
        ESP_LOGI("event handler", "----> %s", "unknown event_base");
    }
    return;
}

static void wifi_connetc()
{
    //初始化NVS
    esp_err_t err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

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
    wifi_config_t wifi_config ={
        .sta.ssid = my_wifi_ssid,
        .sta.password = my_wifi_password
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA,&wifi_config);
    //设置wifi模式
    esp_wifi_set_mode(WIFI_MODE_STA);
    //启动WiFi
    esp_wifi_start();
    //开始连接
    esp_wifi_connect();
}

//概率触发掉电检测
void app_main(void)
{
    wifi_connetc();
}