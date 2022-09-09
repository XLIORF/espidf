#ifndef __I2C_BUS0_H
#define __I2C_BUS0_H
void i2c_bus0_init(void);
void i2c_bus0_Deinit(void);
int i2c_bus0_register_read(uint8_t i2c_bus0_device_address,uint8_t reg_addr, uint8_t *data, size_t len);
int i2c_bus0_register_write_byte(uint8_t i2c_bus0_device_address,uint8_t reg_addr, uint8_t data);
#endif
