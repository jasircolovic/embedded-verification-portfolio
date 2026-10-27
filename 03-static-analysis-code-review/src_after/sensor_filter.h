#ifndef SENSOR_FILTER_H
#define SENSOR_FILTER_H

#include <stdint.h>

#define SF_BUF_SIZE        ((uint8_t)8U)
#define SF_ADC_MAX         ((uint16_t)4095U)
#define SF_OVERCURRENT_RAW ((uint16_t)3500U)

typedef enum { SF_OK = 0, SF_ERR_EMPTY, SF_ERR_RANGE, SF_ERR_NULL } sf_status_t;

void        sf_reset(void);
sf_status_t sf_push(uint16_t raw);
sf_status_t sf_average(uint16_t *out_avg);
sf_status_t sf_to_percent(uint16_t raw, uint8_t *out_pct);
uint8_t     sf_over_current(uint16_t avg);
const char *sf_level_name(uint8_t pct);

#endif
