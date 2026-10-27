#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

/* Driver for an I2C pressure sensor used in an infusion pump occlusion check.
 * Frame: [MSB][LSB][CRC-8] where the 16-bit value is pressure in mbar.
 * CRC-8: polynomial 0x31, init 0xFF (Sensirion-style). */
#include <stdint.h>

#define PS_I2C_ADDR       ((uint8_t)0x40U)
#define PS_REG_PRESSURE   ((uint8_t)0x10U)
#define PS_MIN_MBAR       ((uint16_t)300U)
#define PS_MAX_MBAR       ((uint16_t)1100U)
#define PS_MAX_ATTEMPTS   ((uint8_t)3U)

typedef enum {
    PS_OK = 0,
    PS_ERR_NULL,
    PS_ERR_COMM,     /* bus error on every attempt            */
    PS_ERR_CRC,      /* data corrupted on every attempt       */
    PS_ERR_RANGE     /* valid frame, physically impossible    */
} ps_status_t;

uint8_t     ps_crc8(const uint8_t *data, uint8_t len);
ps_status_t ps_read_mbar(uint16_t *out_mbar);

#endif
