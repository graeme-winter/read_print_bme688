/*
 * read_print_bme688
 *
 * very much work in progress... currently ->
 *
 * scan pico i2c busses for devices - checks devices connected to GPIO0/2 and
 * 2/3 (i.e. i2c 0 and 1)
 *
 */

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define I2C0_PORT i2c0
#define I2C0_SDA 0
#define I2C0_SCL 1

#define I2C1_PORT i2c1
#define I2C1_SDA 2
#define I2C1_SCL 3

const uint LED = 25;

// certain addresses are reserved

bool address_reserved(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main() {
  stdio_init_all();

  // initialise LED
  gpio_init(LED);
  gpio_set_dir(LED, GPIO_OUT);
  gpio_put(LED, 1);

  printf("Startup\n");

  // I2C0 Initialisation. Using it at 400Khz.
  i2c_init(I2C0_PORT, 400 * 1000);
  gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C0_SDA);
  gpio_pull_up(I2C0_SCL);

  printf("Initialised i1c 0\n");

  // I2C0 Initialisation. Using it at 400Khz.
  i2c_init(I2C1_PORT, 400 * 1000);
  gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C1_SDA);
  gpio_pull_up(I2C1_SCL);

  printf("Initialised i1c 1\n");

  while (true) {

    gpio_put(LED, 0);
    printf("Scanning i2c 0\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < 0x80; addr++) {
      if (addr % 0x10 == 0) {
        printf("%02x ", addr);
      }
      int result;
      uint8_t received;
      if (address_reserved(addr)) {
        result = PICO_ERROR_GENERIC;
      } else {
        result = i2c_read_blocking(I2C0_PORT, addr, &received, 1, false);
      }
      if (result < 0) {
        printf(".");
      } else {
        printf("X");
      }
      if (addr % 0x10 == 0xf) {
        printf("\n");
      } else {
        printf("  ");
      }
    }
    sleep_ms(1000);
    gpio_put(LED, 1);
    printf("Scanning i2c 1\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < 0x80; addr++) {
      if (addr % 0x10 == 0) {
        printf("%02x ", addr);
      }
      int result;
      uint8_t received;
      if (address_reserved(addr)) {
        result = PICO_ERROR_GENERIC;
      } else {
        result = i2c_read_blocking(I2C1_PORT, addr, &received, 1, false);
      }
      if (result < 0) {
        printf(".");
      } else {
        printf("X");
      }
      if (addr % 0x10 == 0xf) {
        printf("\n");
      } else {
        printf("  ");
      }
    }
    sleep_ms(1000);
  }
  return 0;
}
