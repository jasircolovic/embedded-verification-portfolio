#ifndef FAKE_I2C_H
#define FAKE_I2C_H

/* Scriptable fake for hal_i2c_read(): tests queue the responses the "bus"
 * will give, then inspect how the driver called it. This is the same idea as
 * the stubs/mocks Tessy or Cantata generate for hardware-dependent code. */
#include <stdint.h>
#include "hal_i2c.h"

#define FAKE_I2C_MAX_RESPONSES 8U

void    fake_i2c_reset(void);
void    fake_i2c_queue(hal_status_t status, uint8_t b0, uint8_t b1, uint8_t b2);
void    fake_i2c_queue_value(uint16_t mbar);               /* frame with valid CRC   */
void    fake_i2c_queue_bad_crc(uint16_t mbar);             /* frame with wrong CRC   */
uint8_t fake_i2c_call_count(void);
uint8_t fake_i2c_last_addr(void);
uint8_t fake_i2c_last_reg(void);
uint8_t fake_i2c_last_len(void);

#endif
