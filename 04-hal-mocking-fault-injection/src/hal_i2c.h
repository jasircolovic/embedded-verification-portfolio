#ifndef HAL_I2C_H
#define HAL_I2C_H

/* Hardware abstraction layer for I2C. On target this is implemented by the
 * MCU vendor driver; in unit tests it is replaced at link time by a fake
 * (test/fake_i2c.c) so the driver can be tested on a PC. */
#include <stdint.h>

typedef enum { HAL_OK = 0, HAL_ERR_TIMEOUT, HAL_ERR_NACK } hal_status_t;

hal_status_t hal_i2c_read(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif
