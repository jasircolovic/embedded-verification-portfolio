#include <stddef.h>
#include "temp_monitor.h"

static uint8_t is_valid(int16_t temp_dc)
{
    return (uint8_t)((temp_dc >= TM_TEMP_MIN_DC) && (temp_dc <= TM_TEMP_MAX_DC));
}

/* REQ-TM-01..03, REQ-TM-05 */
static tm_state_t classify(tm_state_t previous, int16_t temp_dc)
{
    tm_state_t next;

    if (temp_dc >= TM_SHUTDOWN_DC) {
        next = TM_SHUTDOWN;
    } else if (temp_dc >= TM_WARNING_DC) {
        next = TM_WARNING;
    } else if ((previous == TM_WARNING) && (temp_dc >= TM_HYSTERESIS_DC)) {
        next = TM_WARNING;
    } else {
        next = TM_NORMAL;
    }
    return next;
}

tm_status_t tm_init(tm_ctx_t *ctx)
{
    if (ctx == NULL) {
        return TM_ERR_NULL;
    }
    ctx->state = TM_NORMAL;
    ctx->last_valid = TM_NORMAL;
    ctx->invalid_count = 0U;
    ctx->latched = 0U;
    return TM_OK;
}

tm_status_t tm_update(tm_ctx_t *ctx, int16_t temp_dc, tm_state_t *out_state)
{
    if ((ctx == NULL) || (out_state == NULL)) {          /* REQ-TM-10 */
        return TM_ERR_NULL;
    }

    if (is_valid(temp_dc) == 0U) {                         /* REQ-TM-04 */
        if (ctx->invalid_count < TM_MAX_INVALID) {
            ctx->invalid_count++;
        }
        if (ctx->invalid_count >= TM_MAX_INVALID) {        /* REQ-TM-08 */
            ctx->latched = 1U;
        }
        ctx->state = (ctx->latched != 0U) ? TM_SHUTDOWN : TM_SENSOR_FAULT;
    } else {
        ctx->invalid_count = 0U;
        ctx->last_valid = classify(ctx->last_valid, temp_dc);
        if (ctx->last_valid == TM_SHUTDOWN) {
            ctx->latched = 1U;
        }
        ctx->state = (ctx->latched != 0U) ? TM_SHUTDOWN : ctx->last_valid; /* REQ-TM-06 */
    }

    *out_state = ctx->state;
    return TM_OK;
}

/* REQ-TM-07 */
tm_status_t tm_reset(tm_ctx_t *ctx, int16_t temp_dc)
{
    if (ctx == NULL) {
        return TM_ERR_NULL;
    }
    if ((is_valid(temp_dc) == 0U) || (temp_dc >= TM_HYSTERESIS_DC)) {
        return TM_ERR_REJECTED;
    }
    return tm_init(ctx);
}

/* REQ-TM-09 */
uint8_t tm_power_limit_pct(tm_state_t state)
{
    uint8_t pct;

    switch (state) {
    case TM_NORMAL:
        pct = 100U;
        break;
    case TM_WARNING:
        pct = 50U;
        break;
    case TM_SHUTDOWN:
    case TM_SENSOR_FAULT:
    default:                       /* unknown value: fail safe */
        pct = 0U;
        break;
    }
    return pct;
}
