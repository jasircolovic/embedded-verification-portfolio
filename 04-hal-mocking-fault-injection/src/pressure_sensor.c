#include <stddef.h>
#include "hal_i2c.h"
#include "pressure_sensor.h"

uint8_t ps_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFFU;
    uint8_t i;
    uint8_t bit;

    for (i = 0U; i < len; i++) {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((uint8_t)(crc << 1U) ^ 0x31U);
            } else {
                crc = (uint8_t)(crc << 1U);
            }
        }
    }
    return crc;
}

/* Reads the sensor with up to PS_MAX_ATTEMPTS attempts. A bus error or a CRC
 * mismatch triggers a retry; an out-of-range value does not, because the
 * frame was transferred correctly and retrying would hide a sensor fault. */
ps_status_t ps_read_mbar(uint16_t *out_mbar)
{
    uint8_t frame[3];
    uint8_t attempt;
    ps_status_t result = PS_ERR_COMM;
    uint16_t value;

    if (out_mbar == NULL) {
        return PS_ERR_NULL;
    }

    for (attempt = 0U; attempt < PS_MAX_ATTEMPTS; attempt++) {
        if (hal_i2c_read(PS_I2C_ADDR, PS_REG_PRESSURE, frame, 3U) != HAL_OK) {
            result = PS_ERR_COMM;
            continue;
        }
        if (ps_crc8(frame, 2U) != frame[2]) {
            result = PS_ERR_CRC;
            continue;
        }
        value = (uint16_t)(((uint16_t)frame[0] << 8U) | frame[1]);
        if ((value < PS_MIN_MBAR) || (value > PS_MAX_MBAR)) {
            return PS_ERR_RANGE;
        }
        *out_mbar = value;
        return PS_OK;
    }
    return result;
}
