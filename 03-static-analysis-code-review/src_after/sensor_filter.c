/* sensor_filter.c -- REVIEWED version. Each fix references a finding ID
 * from CODE_REVIEW.md. */
#include <stddef.h>
#include "sensor_filter.h"

static uint16_t buffer[SF_BUF_SIZE];
static uint8_t  head;
static uint8_t  count;

void sf_reset(void)
{
    head = 0U;
    count = 0U;
}

sf_status_t sf_push(uint16_t raw)
{
    if (raw > SF_ADC_MAX) {                         /* D5: validate input range  */
        return SF_ERR_RANGE;
    }
    buffer[head] = raw;
    head = (uint8_t)((head + 1U) % SF_BUF_SIZE);    /* D1: wrap at BUF_SIZE      */
    if (count < SF_BUF_SIZE) {
        count++;
    }
    return SF_OK;
}

sf_status_t sf_average(uint16_t *out_avg)
{
    uint32_t sum = 0U;                              /* D2: initialised, wide type */
    uint8_t i;

    if (out_avg == NULL) {
        return SF_ERR_NULL;
    }
    if (count == 0U) {                              /* D4: no division by zero   */
        return SF_ERR_EMPTY;
    }
    for (i = 0U; i < count; i++) {                  /* D3: < not <=              */
        sum += buffer[i];
    }
    *out_avg = (uint16_t)(sum / count);
    return SF_OK;
}

sf_status_t sf_to_percent(uint16_t raw, uint8_t *out_pct)
{
    if (out_pct == NULL) {
        return SF_ERR_NULL;
    }
    if (raw > SF_ADC_MAX) {                         /* D5: range check           */
        return SF_ERR_RANGE;
    }
    *out_pct = (uint8_t)(((uint32_t)raw * 100U) / SF_ADC_MAX);  /* D5: explicit */
    return SF_OK;
}

uint8_t sf_over_current(uint16_t avg)
{
    return (avg >= SF_OVERCURRENT_RAW) ? 1U : 0U;   /* D6: comparison, not '='   */
}

const char *sf_level_name(uint8_t pct)
{
    const char *name;                               /* D7: string literals have  */
                                                    /*     static storage        */
    switch (pct / 25U) {
    case 0U:
        name = "low";
        break;
    case 1U:
        name = "medium";
        break;
    case 2U:
        name = "high";
        break;                                      /* D8: missing break added   */
    case 3U:
    case 4U:                                        /* D8: pct == 100 handled    */
        name = "critical";
        break;
    default:                                        /* D8: default branch        */
        name = "invalid";
        break;
    }
    return name;
}
