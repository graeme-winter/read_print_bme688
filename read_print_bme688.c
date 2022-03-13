/*
 * read_print_bme688
 *
 * very much work in progress... currently ->
 *
 * scan pico i2c busses for devices - checks devices connected to GPIO0/2 and
 * 2/3 (i.e. i2c 0 and 1)
 *
 * will assume that the bme68x is connected to i2c 1 on GPIO 2, 3 (i.e.
 * physical pins 4, 5 counting in people numbers) at address 0x76.
 *
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

typedef struct i2c_config {
  i2c_inst_t *channel;
  uint8_t addr;
} i2c_config;

// helper functions - map from bme68x API to pico SDK i2c

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

  i2c_write_blocking(config->channel, config->addr, buffer, len + 1, false);
  // FIXME some status check would be good
  return 0;
}

BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data,
                                     uint32_t len, void *intf_addr) {
  i2c_config *config = (i2c_config *)intf_addr;

  // push one byte to indicate register then read back content
  i2c_write_blocking(config->channel, config->addr, &reg_addr, 1, true);
  i2c_read_blocking(config->channel, config->addr, reg_data, len, false);
  // FIXME some status check please
  return 0;
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

  printf("Startup\n");

  // I2C0 Initialisation. Using it at 400Khz.
  i2c_init(I2C1_PORT, 400 * 1000);
  gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C1_SDA);
  gpio_pull_up(I2C1_SCL);

  config.channel = I2C1_PORT;
  config.addr = BME688_ADDR;

  printf("Initialised i2c 1\n");

  // initialise device -

  // set up input structure
  bme688.read = bme68x_i2c_read;
  bme688.write = bme68x_i2c_write;
  bme688.delay_us = bme68x_delay_us;
  bme688.intf = BME68X_I2C_INTF;
  bme688.intf_ptr = (void *)&config;
  bme688.amb_temp = 21;

  // call init
  result = bme68x_init(&bme688);
  printf("init %d %d\n", result, BME68X_OK);

  // TODO configure device: heater off; 1x filtering etc.
  dev_config.filter = BME68X_FILTER_OFF;
  dev_config.odr = BME68X_ODR_NONE;
  dev_config.os_hum = BME68X_OS_16X;
  dev_config.os_pres = BME68X_OS_1X;
  dev_config.os_temp = BME68X_OS_2X;
  result = bme68x_set_conf(&dev_config, &bme688);
  printf("device init %d %d\n", result, BME68X_OK);

  // disable heater
  heater_config.enable = BME68X_DISABLE;
  heater_config.heatr_temp = 0;
  heater_config.heatr_dur = 0;
  result = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heater_config, &bme688);
  printf("heater init %d %d\n", result, BME68X_OK);

  int j = 0;

  while (true) {

    // read from device - does this need me to poke something into a register
    // first? seems to be handled by get_data with forced mode

    printf("iter %d\n", j);
    j++;
    uint8_t ndata = 1;
    result = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme688);
    printf("set op mode %d %d\n", result, BME68X_OK);
    result = bme68x_get_data(BME68X_FORCED_MODE, &data, &ndata, &bme688);
    printf("get data %d %d\n", result, BME68X_OK);
    // print data

    printf("%.1f %.1f %.1f\n", data.temperature, data.pressure, data.humidity);

    // blink LED
    gpio_put(LED, 0);
    sleep_ms(1000);
    gpio_put(LED, 1);
    sleep_ms(1000);
  }
  return 0;
}
