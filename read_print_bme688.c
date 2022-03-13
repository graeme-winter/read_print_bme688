/*
 * read_print_bme688
 *
 * Simple code to read data from a bme688 over i2c (bus etc. defined below)
 * and print to the stdout over UART.
 */

#include "api/bme68x.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define I2C1_PORT i2c1
#define I2C1_SDA 2
#define I2C1_SCL 3

#define BME688_ADDR 0x76

const uint LED = 25;

// struct to pass around the i2c channel information (over void *)

typedef struct i2c_config {
  i2c_inst_t *channel;
  uint8_t addr;
} i2c_config;

// helper functions - write to i2c, read from i2c, sleep in µs

BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data,
                                      uint32_t len, void *intf_addr) {
  i2c_config *config = (i2c_config *)intf_addr;

  // copy message to prepend register - or could I just make
  // 2 calls to write with the first keeping the channel open?
  uint8_t buffer[len + 1];
  buffer[0] = reg_addr;
  for (uint32_t j = 0; j < len; j++) {
    buffer[j + 1] = reg_data[j];
  }

  int result =
      i2c_write_blocking(config->channel, config->addr, buffer, len + 1, false);

  if (result == (len + 1)) {
    return BME68X_INTF_RET_SUCCESS;
  } else {
    return result;
  }
}

BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data,
                                     uint32_t len, void *intf_addr) {
  i2c_config *config = (i2c_config *)intf_addr;
  int result;

  // push one byte to indicate register then read back content
  result =
      i2c_write_blocking(config->channel, config->addr, &reg_addr, 1, true);

  if (result != 1) {
    return result;
  }

  result =
      i2c_read_blocking(config->channel, config->addr, reg_data, len, false);

  if (result == len) {
    return BME68X_INTF_RET_SUCCESS;
  } else {
    return result;
  }
}

void bme68x_delay_us(uint32_t period, void *intf_addr) {
  sleep_us((uint64_t)period);
}

int main() {
  stdio_init_all();

  struct bme68x_dev bme688;
  struct bme68x_data data;
  struct bme68x_conf dev_config;
  struct bme68x_heatr_conf heater_config;
  int8_t result;
  i2c_config config;

  // initialise LED
  gpio_init(LED);
  gpio_set_dir(LED, GPIO_OUT);
  gpio_put(LED, 1);

  // I2C0 Initialisation. Using it at 400Khz.
  i2c_init(I2C1_PORT, 400 * 1000);
  gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C1_SDA);
  gpio_pull_up(I2C1_SCL);

  config.channel = I2C1_PORT;
  config.addr = BME688_ADDR;

  // initialise - set up input structure
  bme688.read = bme68x_i2c_read;
  bme688.write = bme68x_i2c_write;
  bme688.delay_us = bme68x_delay_us;
  bme688.intf = BME68X_I2C_INTF;
  bme688.intf_ptr = (void *)&config;
  bme688.amb_temp = 21;

  result = bme68x_init(&bme688);

  dev_config.filter = BME68X_FILTER_OFF;
  dev_config.odr = BME68X_ODR_NONE;
  dev_config.os_hum = BME68X_OS_4X;
  dev_config.os_pres = BME68X_OS_4X;
  dev_config.os_temp = BME68X_OS_4X;

  result = bme68x_set_conf(&dev_config, &bme688);

  // disable heater
  heater_config.enable = BME68X_DISABLE;
  heater_config.heatr_temp = 0;
  heater_config.heatr_dur = 0;

  result = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heater_config, &bme688);

  while (true) {

    uint8_t ndata = 1;
    result = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme688);
    result = bme68x_get_data(BME68X_FORCED_MODE, &data, &ndata, &bme688);
    printf("%.1f %.1f %.1f\n", data.temperature, data.pressure, data.humidity);

    // blink LED
    gpio_put(LED, 1);
    sleep_ms(100);
    gpio_put(LED, 0);

    // wait for a spell
    sleep_ms(9900);
  }

  return 0;
}
