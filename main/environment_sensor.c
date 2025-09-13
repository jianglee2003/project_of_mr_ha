#include "environment_sensor.h"

i2c_master_bus_handle_t i2c_bus_handle = NULL;
i2c_master_dev_handle_t aht10_dev = NULL;
i2c_master_dev_handle_t bh1750_dev = NULL;

static float read_sensor(uint8_t temp_or_hum);

// Commands to trigger measurement
static uint8_t trig_cmd[3] = {AHT10_TRIG_MEAS, AHT10_DAT1_CMD, AHT10_DAT2_CMD};
// Calibration commands
static uint8_t calib_cmd[3] = {0xE1, 0x08, 0x00};

int i2c_master_port = I2C_MASTER_PORT;
static uint8_t data_len;

/**
 * Only comment this function if you use multiple I2C devices, that each device has their own library.
 */
void i2c_master_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));
}

void i2c_addr_check(void) {
    for (uint8_t addr = 1; addr < 127; addr++) {
        esp_err_t ret = i2c_master_probe(i2c_bus_handle, addr, 1000 / portTICK_PERIOD_MS);
        if (ret == ESP_OK) {
            printf("Found device at 0x%02X\n", addr);
        }
    }
}

static void i2c_master_send(uint8_t *data, uint8_t len, uint8_t addr) {
    i2c_master_transmit(aht10_dev, data, len, 1000 / portTICK_PERIOD_MS);
}

static void i2c_master_receive_(uint8_t *rx_buf, uint8_t len, uint8_t addr) {
    i2c_master_receive(aht10_dev, rx_buf, len, 1000 / portTICK_PERIOD_MS);
}

static float read_sensor(uint8_t temp_or_hum) {
    uint8_t rx_buf[6];
    i2c_master_send(trig_cmd, 3, AHT10_ADDR);
    vTaskDelay(75 / portTICK_PERIOD_MS);
    i2c_master_receive_(rx_buf, 6, AHT10_ADDR);

    while (rx_buf[0] & (1 << 7)) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        i2c_master_receive_(rx_buf, 6, AHT10_ADDR);
    }

    uint32_t humidity_raw = ((uint32_t)rx_buf[1] << 12) | ((uint16_t)rx_buf[2] << 4) | (rx_buf[3] >> 4);
    uint32_t temp_raw = (((uint32_t)rx_buf[3] & 0x0F) << 16) | ((uint16_t)rx_buf[4] << 8) | (rx_buf[5]);

    float humidity = ((float)humidity_raw / 1048576.0f) * 100.0f;
    float temperature = (((float)temp_raw / 1048576.0f) * 200.0f) - 50.0f;

    if (temp_or_hum == HUMIDITY) return humidity;
    else if (temp_or_hum == TEMPERATURE) return temperature;
    return NAN;
}

float read_humidity(void) {
    return read_sensor(HUMIDITY);
}

float read_temperature(void) {
    return read_sensor(TEMPERATURE);
}

int I2C_Transmit(int i2c_num, uint8_t dev_addr, uint8_t mem_address, uint16_t timeout) {
    uint8_t data[1] = {mem_address};
    return i2c_master_transmit(bh1750_dev, data, 1, timeout / portTICK_PERIOD_MS);
}

uint8_t sensor_data_h = 0, sensor_data_l = 0;

int I2C_Receive(int i2c_num, uint8_t dev_addr, uint16_t timeout) {
    uint8_t data[2];
    esp_err_t ret = i2c_master_receive(bh1750_dev, data, 2, timeout / portTICK_PERIOD_MS);
    if (ret == ESP_OK) {
        sensor_data_h = data[0];
        sensor_data_l = data[1];
    }
    return ret;
}

void ahtxx_init(void) {
    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AHT10_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &aht10_dev));

    data_len = 3;
    i2c_master_send(calib_cmd, data_len, AHT10_ADDR);
    ESP_LOGI("AHTxx_Series", "AHTxx configured successfully.");
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

void bh1750_init(void) {
    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BH1750_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &bh1750_dev));
    ESP_LOGI("BH1750_Sensor", "BH1750 configured successfully.");
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

float read_bh1750(void) {
    I2C_Transmit(i2c_master_port, BH1750_ADDR, 0x23, 1000);
    vTaskDelay(30 / portTICK_PERIOD_MS);
    I2C_Receive(i2c_master_port, BH1750_ADDR, 1000);

    uint16_t lux = (sensor_data_h << 8) | sensor_data_l;
    return lux / 1.2f;
}
