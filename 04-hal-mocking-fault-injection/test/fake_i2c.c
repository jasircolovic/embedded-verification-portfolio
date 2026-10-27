#include "fake_i2c.h"
#include "pressure_sensor.h"

typedef struct {
    hal_status_t status;
    uint8_t      bytes[3];
} response_t;

static response_t queue[FAKE_I2C_MAX_RESPONSES];
static uint8_t queued;
static uint8_t calls;
static uint8_t last_addr;
static uint8_t last_reg;
static uint8_t last_len;

void fake_i2c_reset(void)
{
    queued = 0U;
    calls = 0U;
    last_addr = 0U;
    last_reg = 0U;
    last_len = 0U;
}

void fake_i2c_queue(hal_status_t status, uint8_t b0, uint8_t b1, uint8_t b2)
{
    if (queued < FAKE_I2C_MAX_RESPONSES) {
        queue[queued].status = status;
        queue[queued].bytes[0] = b0;
        queue[queued].bytes[1] = b1;
        queue[queued].bytes[2] = b2;
        queued++;
    }
}

void fake_i2c_queue_value(uint16_t mbar)
{
    uint8_t d[2] = { (uint8_t)(mbar >> 8U), (uint8_t)(mbar & 0xFFU) };
    fake_i2c_queue(HAL_OK, d[0], d[1], ps_crc8(d, 2U));
}

void fake_i2c_queue_bad_crc(uint16_t mbar)
{
    uint8_t d[2] = { (uint8_t)(mbar >> 8U), (uint8_t)(mbar & 0xFFU) };
    fake_i2c_queue(HAL_OK, d[0], d[1], (uint8_t)(ps_crc8(d, 2U) ^ 0x01U));
}

/* Link-time substitute for the real HAL function. */
hal_status_t hal_i2c_read(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    response_t r = { HAL_ERR_TIMEOUT, { 0U, 0U, 0U } };  /* nothing queued: bus dead */
    uint8_t i;

    last_addr = dev_addr;
    last_reg = reg;
    last_len = len;
    if (calls < queued) {
        r = queue[calls];
    }
    calls++;
    if (r.status == HAL_OK) {
        for (i = 0U; (i < len) && (i < 3U); i++) {
            buf[i] = r.bytes[i];
        }
    }
    return r.status;
}

uint8_t fake_i2c_call_count(void) { return calls; }
uint8_t fake_i2c_last_addr(void)  { return last_addr; }
uint8_t fake_i2c_last_reg(void)   { return last_reg; }
uint8_t fake_i2c_last_len(void)   { return last_len; }
