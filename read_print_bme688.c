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

int main() {
  stdio_init_all();

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

  printf("Initialised i1c 1\n");

  while (true) {
    gpio_put(LED, 0);
    sleep_ms(100);
    gpio_put(LED, 1);

    int addr = 0x76;

    int result;
    uint8_t received;
    result = i2c_read_blocking(I2C1_PORT, addr, &received, 1, false);
    if (result < 0) {
      printf("Not found\n");
    } else {
      printf("Found device at address %d\n", addr);
    }
    sleep_ms(100);
  }
  return 0;
}
