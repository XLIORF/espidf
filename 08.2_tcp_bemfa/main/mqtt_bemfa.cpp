#include "mqtt_bemfa.h"
#include "esp_log.h"
#include "esp_err.h"
using namespace std; 
static char TAG[] = "mqtt_bemfa";

mqtt_bemfa::mqtt_bemfa(char *uid, const char *host, int port)
{
    while (!this->create_socket());
    this->bind_server(host, port);
    sprintf(this->_uid, "%s", uid);
    ESP_LOGI(TAG, "uid=%s", this->_uid);
}

mqtt_bemfa::~mqtt_bemfa()
{
    close(this->_socket);
}

bool mqtt_bemfa::connect()
{
    esp_err_t err = ::connect(this->_socket, (struct sockaddr *)&(this->_server_addr), sizeof(this->_server_addr));
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "连接成功。");
        return true;
    }
    else
    {
        ESP_LOGI(TAG, "连接失败，错误代码：%d", errno);
        return false;
    }
}

mqtt_bemfa_err_t mqtt_bemfa::subscribe(const char *topic)
{
    char format_data[200];
    sprintf(this->_raw,
            "cmd=1&uid=%s&topic=%s\r\n",
            this->_uid,
            topic);
    esp_err_t err = ::sendto(this->_socket, this->_raw, sizeof(this->_raw), true,
                             (struct sockaddr *)&(this->_server_addr),
                             sizeof(this->_server_addr));
    if (err == -1)
    {
        ESP_LOGI(TAG, "send failed!");
        return MQTT_BEMFA_ERROR_SEND_FAILD;
    }
    ESP_LOGI(TAG, "send finish!");
    ::recv(this->_socket, this->_recv_buffer.raw, sizeof(this->_recv_buffer.raw), false);
    if (sscanf(this->_recv_buffer.raw, "%[0-9a-zA-Z=&]\r\n", format_data) != -1)
        ESP_LOGI(TAG, "first Recv:%s", format_data);
    if (strcmp(format_data, "cmd=1&res=1") == 0)
        return MQTT_BEMFA_OK;
    return MQTT_BEMFA_FAILD;
}

void mqtt_bemfa::reset_socket(void)
{
    close(this->_socket);
    this->create_socket();
}

bool mqtt_bemfa::create_socket()
{
    this->_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (this->_socket < 0)
    {
        ESP_LOGE(TAG, "套接字创建失败: errno %d", errno);
        close(this->_socket);
        return false;
    }
    else
    {
        ESP_LOGI(TAG, "套接字创建成功.");
        return true;
    }
}

void mqtt_bemfa::bind_server(const char *host, int port)
{
    // 2.绑定地址端口
    //  this->_dest_addr = {
    //      .sin_family = AF_INET,
    //      .sin_addr.s_addr = inet_addr(host),//这行位置原因报错
    //      .sin_port = htons(port)
    //  };
    this->_server_addr.sin_family = AF_INET;
    this->_server_addr.sin_addr.s_addr = inet_addr(host),
    this->_server_addr.sin_addr.s_addr = inet_addr(host);
}

mqtt_bemfa_err_t mqtt_bemfa::push(const char *topic, char *msg, bool notice)
{
    char format_data[200];
    sprintf(this->_raw,
            "cmd=2&uid=%s&topic=%s/%s&msg=%s\r\n",
            this->_uid,
            topic,
            (notice ? "up":"set"),
            msg);
    esp_err_t err = ::sendto(this->_socket, this->_raw, sizeof(_raw), true,
                             (struct sockaddr *)&(this->_server_addr),
                             sizeof(this->_server_addr));
    if (err == -1)
    {
        ESP_LOGI(TAG, "send failed!");
        return MQTT_BEMFA_ERROR_SEND_FAILD;
    }
    ESP_LOGI(TAG, "send finish!");

    ::recv(this->_socket, this->_recv_buffer.raw, sizeof(this->_recv_buffer.raw), false);
    if (sscanf(this->_recv_buffer.raw, "%[0-9a-zA-Z=&]\r\n", format_data) != -1)
        ESP_LOGI(TAG, "first Recv:%s", format_data);
    if (strcmp(format_data, "cmd=2&res=1") == 0)
        return MQTT_BEMFA_OK;
    return MQTT_BEMFA_FAILD;
}

void mqtt_bemfa::loop()
{
    char format_data[200];
    bool loop_flag = true;
    while (loop_flag)
    {
        esp_err_t err = ::sendto(this->_socket, "ping\r\n", sizeof("ping\r\n"), true,
                                (struct sockaddr *)&(this->_server_addr),
                                sizeof(this->_server_addr));
        if (err == -1)
        {
            ESP_LOGI(TAG, "send failed!");
            continue;
        }
        ESP_LOGI(TAG, "send finish!");

        ::recv(this->_socket, this->_recv_buffer.raw, sizeof(this->_recv_buffer.raw), false);
        if (sscanf(this->_recv_buffer.raw, "%[0-9a-zA-Z=&]\r\n", format_data) != -1)
            ESP_LOGI(TAG, "first Recv:%s", format_data);
        if (strcmp(format_data, "cmd=0&res=1") == 0)
            break;
    }
}

void mqtt_bemfa::recv()
{
    //请参照事件循环完成mqtt事件处理函数的编写
    char format_data[200];
    ::recv(this->_socket, this->_recv_buffer.raw, sizeof(this->_recv_buffer.raw), false);
    if(sscanf(this->_recv_buffer.raw,"%[0-9a-zA-Z=&]\r\n",format_data) != -1)
        ESP_LOGI(TAG,"Recv:%s",format_data);
    if(sscanf(format_data,"cmd=%d&uid=%[a-z0-9A-Z]&topic=%[a-z0-9A-Z]&msg=%[a-z0-9A-Z]",
            &this->_recv_buffer.cmd,this->_recv_buffer.uid,this->_recv_buffer.topic,this->_recv_buffer.msg) != -1)
        ESP_LOGI(TAG,"topic:%s,msg:%s",this->_recv_buffer.topic,this->_recv_buffer.msg);
}

bool mqtt_bemfa::get_recv_flag()
{
    
    if (this->_recv_flag )
    {
        this->_recv_flag = false;
        return true;
    }
    return false;
}