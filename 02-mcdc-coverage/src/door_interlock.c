/* Railway door interlock: doors may only be released when the train is at
 * standstill, on the correct side (or the driver overrides), and the brake
 * system is healthy. Any NULL input fails safe (doors stay locked). */
#include <stddef.h>
#include "door_interlock.h"

bool di_train_stopped(uint16_t speed_kmh_x10)
{
    return speed_kmh_x10 <= DI_STANDSTILL_MAX_KMH_X10;
}

bool di_door_release_permitted(const di_inputs_t *in)
{
    if (in == NULL) {
        return false;
    }
    /* Decision under MC/DC analysis: 4 conditions A && (B || C) && !D */
    return di_train_stopped(in->speed_kmh_x10)          /* A */
        && (in->platform_side_ok || in->driver_override) /* B, C */
        && !in->brake_fault;                              /* D */
}
