#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "string.h"

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
                // ESP_LOGI("event handler", "----> IP_EVENT_STA_GOT_IP ip:");
                // ESP_LOGI("event handler", "----> sta ip:"IPSTR, IP2STR(&event->ip_info.ip));
                // ESP_LOGI("event handler", "----> mask:"IPSTR, IP2STR(&event->ip_info.netmask));
                // ESP_LOGI("event handler", "----> gw:"IPSTR, IP2STR(&event->ip_info.gw));
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


void simplewificonnect(void)
{
    //初始化NVS
    

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
//连接后再扫描wifi会断开当前连接
#if 0 //wifi扫描
    //开始扫描WiFi ap，第二个参数表示否阻塞
    esp_wifi_scan_start(NULL,true);
    //获取扫描到的wifi ap数量
    uint16_t wifi_num;
    esp_wifi_scan_get_ap_num(&wifi_num);
    if (wifi_num > 10)
    {
        wifi_num = 10;//如果环境有太多ap而不做限制就会溢出
    }
    //拿到WiFi ap的信息
    wifi_ap_record_t wifi_ap[wifi_num];//通常是这个变量占用内存（wifi驱动分配到的）过大溢出
    
    esp_wifi_scan_get_ap_records(&wifi_num,wifi_ap);
    //处理wifi ap信息
     for(int i = 0; i < wifi_num; i++){
        //WiFi强度等级划分
        uint8_t rssi_level = 0;
        switch (wifi_ap[i].rssi) {
            case -100 ... -88:
                rssi_level = 1; break;
            case -87 ... -77:
                rssi_level = 2; break;
            case -76 ... -66:
                rssi_level = 3; break;
            case -65 ... -55:
                rssi_level = 4; break;
            default:
                if(wifi_ap[i].rssi < -100){
                    rssi_level = 0;
                }else{
                    rssi_level = 5;
                }
                break;
        }

	// 逐条打印扫描到的WiFi
        printf("—————【第 %2d 个WiFi】———————\n", i+1);
        printf("WiFi名称: %s\n", wifi_ap[i].ssid);
        //判断要连接的wifi是否存在
        if(strcmp((char *)wifi_ap[i].ssid, my_wifi_ssid))wifi_exist_flag = true;
        printf("信号强度: %d格\n", rssi_level);
        printf("WiFi: 安全类型: %d\n\n", wifi_ap[i].authmode);
    }
    if(wifi_exist_flag)
        esp_wifi_connect();//开始连接
    else
        printf("%s 不存在！",my_wifi_ssid);
#endif
}

