#include <stdio.h>
#include "AHT10.h"
#include "driver/i2c.h"

#define AHT10_ADDR 0x38	//AHT10写数据地址

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              CONFIG_I2C_MASTER_NUM                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          CONFIG_I2C_MASTER_FREQ_HZ                     /*!< I2C master clock frequency */
#define I2C_MASTER_TIMEOUT_MS       CONFIG_I2C_MASTER_TIMEOUT_MS 

uint8_t aht10_data[8];

/*********************
 * 如果AHT10存在则返回0
 * ******************/
unsigned char AHT10_State(void)
{
	esp_err_t err = ESP_OK;
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    assert (handle != NULL);

    err = i2c_master_start(handle);
    if (err != ESP_OK) {
        goto end;
    }

    err = i2c_master_write_byte(handle, AHT10_ADDR << 1 | I2C_MASTER_WRITE, true);
    if (err != ESP_OK) {
        goto end;
    }

    i2c_master_stop(handle);
    err = i2c_master_cmd_begin(I2C_MASTER_NUM, handle, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);

end:
    i2c_cmd_link_delete(handle);
    return err;
}

unsigned char AHT10_Init(void)
{
	const unsigned char init_cmd[3] = {0xE1, 0x08, 0x00};
	unsigned char ret;
	ret = i2c_master_write_to_device(I2C_MASTER_NUM, AHT10_ADDR, init_cmd, 3, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
	vTaskDelay(40/portTICK_PERIOD_MS);
	return ret;//成功返回ESP_OK或0
}

unsigned char AHT10_Soft_Reset(void)
{
	unsigned char ret;
    const unsigned char rst_cmd = 0xBA;
	ret = i2c_master_write_to_device(I2C_MASTER_NUM, AHT10_ADDR, &rst_cmd, 1, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
	vTaskDelay(20/portTICK_PERIOD_MS);
	return ret;
}

unsigned char AHT10_Get_Humity_Tempareture(float *humity, float *tempareture)
{
	esp_err_t err = ESP_OK;
	unsigned int humi = 0, temp = 0;
	const uint8_t transf_cmd[3] = {0xAC, 0x33, 0x00};
	err = i2c_master_write_to_device(I2C_MASTER_NUM, AHT10_ADDR, transf_cmd, 3, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
	vTaskDelay(80/portTICK_PERIOD_MS);

    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    assert (handle != NULL);

    err = i2c_master_start(handle);
    if (err != ESP_OK) {
        goto end;
    }

    err = i2c_master_write_byte(handle, AHT10_ADDR << 1 | I2C_MASTER_READ, true);
    if (err != ESP_OK) {
        goto end;
    }

    err = i2c_master_read(handle, aht10_data, 6, I2C_MASTER_LAST_NACK);
    if (err != ESP_OK) {
        goto end;
    }

	i2c_master_stop(handle);
    err = i2c_master_cmd_begin(I2C_MASTER_NUM, handle, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);

	if((aht10_data[0] & 0x08) == 0)//第三位为0说明未校准，执行初始化校准
	{
		AHT10_Init();
	}
	if((aht10_data[0] & 0X80) == 0)//第七位为1说明设备忙碌，为零说明可读
	{
		humi = ((aht10_data[1]<<12) | (aht10_data[2]<<4) | (aht10_data[3]>>4));
		temp = (((aht10_data[3]&0x0F)<<16) | (aht10_data[4]<<8) | (aht10_data[5]));

		*humity = (humi *100.0 / 1024/ 1024 + 0.5);
		*tempareture = (temp * 2000.0 / 1024 / 1024 + 0.5) / 10.0 - 50;
	}
end:
    i2c_cmd_link_delete(handle);
    return 1;
}
