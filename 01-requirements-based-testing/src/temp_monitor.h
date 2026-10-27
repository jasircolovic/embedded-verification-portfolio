#ifndef TEMP_MONITOR_H
#define TEMP_MONITOR_H

#include <stdint.h>

#define TM_TEMP_MIN_DC        ((int16_t)-400)  /* -40.0 C, sensor lower limit  */
#define TM_TEMP_MAX_DC        ((int16_t)1500)  /* 150.0 C, sensor upper limit  */
#define TM_WARNING_DC         ((int16_t)850)   /*  85.0 C                      */
#define TM_SHUTDOWN_DC        ((int16_t)1050)  /* 105.0 C                      */
#define TM_HYSTERESIS_DC      ((int16_t)800)   /*  80.0 C, release threshold   */
#define TM_MAX_INVALID        ((uint8_t)3U)

typedef enum { TM_NORMAL = 0, TM_WARNING, TM_SHUTDOWN, TM_SENSOR_FAULT } tm_state_t;
typedef enum { TM_OK = 0, TM_ERR_NULL, TM_ERR_REJECTED } tm_status_t;

typedef struct {
    tm_state_t state;          /* reported state                         */
    tm_state_t last_valid;     /* state from the last valid sample       */
    uint8_t    invalid_count;  /* consecutive invalid samples            */
    uint8_t    latched;        /* 1 when shutdown is latched             */
} tm_ctx_t;

tm_status_t tm_init(tm_ctx_t *ctx);
tm_status_t tm_update(tm_ctx_t *ctx, int16_t temp_dc, tm_state_t *out_state);
tm_status_t tm_reset(tm_ctx_t *ctx, int16_t temp_dc);
uint8_t     tm_power_limit_pct(tm_state_t state);

#endif
