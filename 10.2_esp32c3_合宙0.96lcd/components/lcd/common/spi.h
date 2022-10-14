#ifndef SPI_H
#define SPI_H

#include "stdint.h"
#include "string.h"
#include "pinmap.h"


#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "gpio.h"


#define MSB 1
#define LSB 0

typedef struct spi
{
    /* data */
    int  id;      // id
    int  CPHA;    // CPHA
    int  CPOL;    // CPOL
    int  dataw;   // 数据宽度
    int  bit_dict;// 高低位顺序    可选，默认高位在前
    int  master;  // 主模式     可选，默认主
    int  mode;    // 全双工       可选，默认全双工
    int bandrate;// 最大频率20M
    int cs;      // cs控制引脚
} spi_t;

typedef struct spi_device
{
    uint8_t  bus_id;
    spi_t spi_config;
    void* user_data;
} spi_device_t;

typedef struct fatfs_spi
{
	uint8_t type;
	uint8_t spi_id;
	uint8_t spi_cs;
	uint8_t nop;
	uint32_t fast_speed;
	spi_device_t * spi_device;
}fatfs_spi_t;

// 软件spi
typedef struct espi {
    uint8_t cs;
    uint8_t mosi;
    uint8_t miso;
    uint32_t clk;
    uint8_t CPHA;
    uint8_t CPOL;
    uint8_t dataw;
    uint8_t bit_dict;
    uint8_t master;
    uint8_t mode;
} espi_t;

/**
    spiId,--串口id
    cs,
    0,--CPHA
    0,--CPOL
    8,--数据宽度
    20000000,--最大频率20M
    spi.MSB,--高低位顺序    可选，默认高位在前
    spi.master,--主模式     可选，默认主
    spi.full,--全双工       可选，默认全双工
*/

spi_device_t* spi_init(int bus_id, int cs, int cpha, int cpol, int dataw, int bandrate, int bit_dict, int master, int mode);

//初始化配置SPI各项参数，并打开SPI
//成功返回0
int spi_setup(spi_t* spi);
//收发SPI数据尝试启动DMA模式
int spi_config_dma(int spi_id, uint32_t tx_channel, uint32_t rx_channel);
//关闭SPI，成功返回0
int spi_close(int spi_id);
//收发SPI数据，返回接收字节数
int spi_transfer(int spi_id, const char* send_buf, size_t send_length, char* recv_buf, size_t recv_length);
//收SPI数据，返回接收字节数
int spi_recv(int spi_id, char* recv_buf, size_t length);
//发SPI数据，返回发送字节数
int spi_send(int spi_id, const char* send_buf, size_t length);

//非阻塞SPI收发数据
int spi_no_block_transfer(int spi_id, uint8_t *tx_buff, uint8_t *rx_buff, size_t len, void *CB, void *pParam);

// 初始化总线
int spi_bus_setup(spi_device_t* spi_dev);
// 初始化设备
int spi_device_setup(spi_device_t* spi_dev);
// 配置设备
int spi_device_config(spi_device_t* spi_dev);
//关闭SPI设备，成功返回0
int spi_device_close(spi_device_t* spi_dev);
//收发SPI数据，返回接收字节数
int spi_device_transfer(spi_device_t* spi_dev, const char* send_buf, size_t send_length, char* recv_buf, size_t recv_length);
//收SPI数据，返回接收字节数
int spi_device_recv(spi_device_t* spi_dev, char* recv_buf, size_t length);
//发SPI数据，返回发送字节数
int spi_device_send(spi_device_t* spi_dev, const char* send_buf, size_t length);

#endif
