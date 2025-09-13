/**
 * All the AHTxx sensor series can be used with this library.
 */
#ifndef ENVIRONMENT_SENSOR_H
#define ENVIRONMENT_SENSOR_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "esp_log.h"

#include "driver/i2c_master.h"  // <-- use new driver header

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// I2C address (shifted left to match 8-bit usage removed in driver_ng)
#define AHT10_ADDR          0x38
#define BH1750_ADDR         0x23

// AHT10 commands
#define AHT10_INIT_CMD      0xE1 
#define AHT10_SOFT_RESET    0xBA
#define AHT10_TRIG_MEAS     0xAC
#define AHT10_DAT1_CMD      0x33
#define AHT10_DAT2_CMD      0x00

// General macros
#define HUMIDITY            0
#define TEMPERATURE         1
#define SEND                0
#define RECEIVE             1

// I2C configuration
#define I2C_MASTER_SDA_IO   GPIO_NUM_2
#define I2C_MASTER_SCL_IO   GPIO_NUM_15
#define I2C_MASTER_FREQ_HZ  400000
#define I2C_MASTER_PORT     1

__attribute__((weak)) extern i2c_master_bus_handle_t i2c_bus_handle;
extern i2c_master_dev_handle_t aht10_dev;
extern i2c_master_dev_handle_t bh1750_dev;

/**
 * Function prototype.
 * for ESP IDF version 5.3.1, user have to init I2C master and device bus,and this action can not be done more than once.
 * So this function must be defined with "weak" keyword, 
 * if you use multiple I2C devices, that each device has their own master bus init function.
 */
__attribute__((weak)) void i2c_master_init(void);
__attribute__((weak)) void i2c_addr_check(void);

int I2C_Transmit(int i2c_num, uint8_t dev_addr, uint8_t mem_addr, uint16_t timeout);
int I2C_Receive(int i2c_num, uint8_t dev_addr, uint16_t timeout);

void ahtxx_init(void);
float read_humidity(void);
float read_temperature(void);

void bh1750_init(void);
float read_bh1750(void);

#endif
