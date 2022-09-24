/**
 *  @filename   :   epdif.cpp
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "epdif.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "string.h"
#include "esp_log.h"

static char TAG[] = "epdif";

esp_err_t ret;
spi_device_handle_t epd_handle;
static QueueHandle_t rdySem;

EpdIf::EpdIf(){};

EpdIf::~EpdIf(){};

void EpdIf::DigitalWrite(gpio_num_t pin, int value)
{
    gpio_set_level(pin, value);
}

int EpdIf::DigitalRead(gpio_num_t pin)
{
    return gpio_get_level(pin);
}

void EpdIf::DelayMs(unsigned int delaytime)
{
    vTaskDelay(delaytime / portTICK_PERIOD_MS);
}

void EpdIf::SpiTransfer(unsigned char data)
{

    char sendbuf[128] = {0};
    char recvbuf[128] = {0};
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    int res = snprintf(sendbuf, sizeof(sendbuf), (char *)&data);
    if (res >= sizeof(sendbuf))
    {
        printf("Data truncated\n");
    }
    t.length = sizeof(sendbuf) * 8;
    t.tx_buffer = sendbuf;
    t.rx_buffer = recvbuf;
    // Wait for slave to be ready for next byte before sending
    // xSemaphoreTake(rdySem, portMAX_DELAY); //Wait until slave is ready
    ret = spi_device_transmit(epd_handle, &t);
    // xSemaphoreGive(rdySem);
}

int EpdIf::IfInit(void)
{
    gpio_config_t cs_config = {
        .pin_bit_mask = 1ULL << CS_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cs_config);
    gpio_config_t rst_config = {
        .pin_bit_mask = 1ULL << RST_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&rst_config);
    gpio_config_t dc_config = {
        .pin_bit_mask = 1ULL << DC_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&dc_config);
    gpio_config_t busy_config = {
        .pin_bit_mask = 1ULL << BUSY_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&busy_config);

    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 128, // 50% duty cycle
        .cs_ena_posttrans = 3, // Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .clock_speed_hz = 2000000,
        .spics_io_num = CS_PIN,
        .queue_size = 3,
    };
    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &epd_handle);
    assert(ret == ESP_OK);
    rdySem = xSemaphoreCreateBinary();
    return 0;
}
