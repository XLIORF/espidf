#ifndef __I2C_BUS0_H
#define __I2C_BUS0_H

void i2c_bus0_init(void);
void i2c_bus0_Deinit(void);
int i2c_bus0_register_read(uint8_t i2c_bus0_device_address,uint8_t reg_addr, uint8_t *data, size_t len);
int i2c_bus0_register_write_byte(uint8_t i2c_bus0_device_address,uint8_t reg_addr, uint8_t data);

// void i2c_bus0_cmd_link_create(uint8_t i2c_bus0_device_address);
// void i2c_bus0_cmd_link_start(void);
// void i2c_bus0_cmd_link_write(uint8_t data);
// void i2c_bus0_cmd_link_read(uint8_t data,bool ack);
// void i2c_bus0_cmd_link_send(void);

#endif
