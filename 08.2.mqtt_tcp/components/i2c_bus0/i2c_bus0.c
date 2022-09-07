#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

static int i2c_bus0_state = 0;
// static i2c_cmd_handle_t handle;

/**
 * @brief i2c master initialization
 */
void i2c_bus0_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    if(i2c_bus0_state == 0)
    {
        i2c_config_t conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = I2C_MASTER_SDA_IO,
            .scl_io_num = I2C_MASTER_SCL_IO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = I2C_MASTER_FREQ_HZ,
        };

        i2c_param_config(i2c_master_port, &conf);
        ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));
        i2c_bus0_state = 1;
    }
}

void i2c_bus0_Deinit(void)
{
    if(i2c_bus0_state != 0)
    {
        ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
        i2c_bus0_state = 0;
        ESP_LOGI("I2C BUS 0", "I2C unitialized successfully");
    }
}

/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
esp_err_t i2c_bus0_register_read(uint8_t i2c_bus0_device_address,uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, i2c_bus0_device_address, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
esp_err_t i2c_bus0_register_write_byte(uint8_t i2c_bus0_device_address,uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, i2c_bus0_device_address, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);

    return ret;
}

// void i2c_bus0_cmd_link_start(uint8_t i2c_bus0_device_address)
// {
// 	esp_err_t err = ESP_OK;
//     handle = i2c_cmd_link_create();
//     assert (handle != NULL);

//     err = i2c_master_start(handle);
//     if (err != ESP_OK) {
//         // goto end;
//         i2c_cmd_link_delete(handle);
//     }

//     err = i2c_master_write_byte(handle, i2c_bus0_device_address << 1 | I2C_MASTER_WRITE, true);
//     if (err != ESP_OK) {
//         // goto end;
//         i2c_cmd_link_delete(handle);
//     }
// }

// void i2c_bus0_cmd_link_write(uint8_t data)
// {
//     esp_err_t err = ESP_OK;
//     err = i2c_master_write_byte(handle, data, true);
//     if (err != ESP_OK) {
//         // goto end;
//         i2c_cmd_link_delete(handle);
//     }
// }

// void i2c_bus0_cmd_link_read(uint8_t data)
// {
//     esp_err_t err = ESP_OK;
//     err = i2c_master_read_byte(handle, data, true);
//     if (err != ESP_OK) {
//         // goto end;
//         i2c_cmd_link_delete(handle);
//     }
// }

// void i2c_bus0_cmd_link_end(void)
// {
//     esp_err_t err = ESP_OK;
//     i2c_master_stop(handle);
//     err = i2c_master_cmd_begin(I2C_MASTER_NUM, handle, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
//     i2c_cmd_link_delete(handle);
//     // return err;
// }