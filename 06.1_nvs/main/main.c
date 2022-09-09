#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "nvs.h"
#include "esp_system.h"

#include "esp_event.h"
#include "esp_wifi.h"

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
    else if(event_base == IP_EVENT) {

        ip_event_got_ip_t *event = (ip_event_got_ip_t*)event_data;

        switch(event_id) {

            case IP_EVENT_STA_GOT_IP:

                ESP_LOGI("event handler", "----> IP_EVENT_STA_GOT_IP ip:");

                ESP_LOGI("event handler", "----> sta ip:"IPSTR, IP2STR(&event->ip_info.ip));

                ESP_LOGI("event handler", "----> mask:"IPSTR, IP2STR(&event->ip_info.netmask));

                ESP_LOGI("event handler", "----> gw:"IPSTR, IP2STR(&event->ip_info.gw));

                break;

            default:

                break;

        }

    }else {

        ESP_LOGI("event handler", "----> %s", "unknown event_base");

    }

 

    return;
}
void app_main(void)
{
    //nvs
#if 1
    uint8_t ssid[] = "ChinaUnicom-E5DFNM";
    uint8_t password[] = "123456789";

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGE("main","nvs init failed, retry");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvs_handle_t wifi_ssid_nvs_handle;
    err = nvs_open("nvs",NVS_READWRITE,&wifi_ssid_nvs_handle);
    if (err != ESP_OK) return err;

    size_t ssid_size,passwd_size;
    err = nvs_get_blob(wifi_ssid_nvs_handle,"ssid",NULL,&ssid_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_get_blob(wifi_ssid_nvs_handle,"password",NULL,&passwd_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    if(ssid_size == 0)
        printf("Nothing ssid saved!\n");
    else 
    {
        char *nvs_ssid = malloc(ssid_size);
        err = nvs_get_blob(wifi_ssid_nvs_handle,"ssid",nvs_ssid,&ssid_size);
        if(err != ESP_OK) 
        {
            free(nvs_ssid);
            ESP_LOGE("main","nvs get data error!");
            return err;
        }
        printf("%s\n",nvs_ssid);
        // for(;i<ssid_size;i++)//不显示，不明原因
        //     printf("%c",nvs_ssid[i]);
        free(nvs_ssid);
        if(passwd_size != 0)
        {
            char *nvs_passwd = malloc(passwd_size);
            err = nvs_get_blob(wifi_ssid_nvs_handle,"password",nvs_passwd,&passwd_size);
            if(err != ESP_OK) 
            {
                free(nvs_passwd);
                ESP_LOGE("main","nvs get passwd error!");
                return err;
            }
            printf("%s\n",nvs_passwd);
            free(nvs_passwd);
        }
    }
    if(0/*save falg*/)
    {
        nvs_set_blob(wifi_ssid_nvs_handle,"ssid",ssid,sizeof(ssid));
        nvs_set_blob(wifi_ssid_nvs_handle,"password",password,sizeof(password));
        nvs_commit(wifi_ssid_nvs_handle);
    }

    nvs_close(wifi_ssid_nvs_handle);
#endif

}