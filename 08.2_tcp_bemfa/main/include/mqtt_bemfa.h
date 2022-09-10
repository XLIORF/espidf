#pragma once //防止多次导入
#include "string.h"
#include "lwip/sockets.h"

typedef enum {
    MQTT_BEMFA_ERROR_SEND_FAILD = -1,
    MQTT_BEMFA_FAILD = 0,
    MQTT_BEMFA_OK = 1
}mqtt_bemfa_err_t;

typedef struct bemfa_data_t
{
    int cmd;
    char topic[64];
    char uid[32];
    char msg[64];
    char raw[200];
} bemfa_data_t;

#ifdef __cplusplus
class mqtt_bemfa
{
private:
    int _socket;
    char _uid[32];
    char _raw[200];
    struct sockaddr_in _server_addr;
    bemfa_data_t _recv_buffer;
    bool _recv_flag = false;
    bool create_socket();
    void reset_socket(void);
    void bind_server(const char *host, int port);

public:
    mqtt_bemfa(char *uid = NULL, const char *host = "119.91.109.180", int port = 8344);
    ~mqtt_bemfa();
    bool connect();
    mqtt_bemfa_err_t subscribe(const char *topic);
    mqtt_bemfa_err_t push(const char *topic, char *msg, bool notice);
    void loop();
    void recv();
    bool get_recv_flag();
};
#endif