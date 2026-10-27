#ifndef DOOR_INTERLOCK_H
#define DOOR_INTERLOCK_H

#include <stdbool.h>
#include <stdint.h>

#define DI_STANDSTILL_MAX_KMH_X10  ((uint16_t)2U)   /* <= 0.2 km/h counts as stopped */

typedef struct {
    uint16_t speed_kmh_x10;      /* measured speed, 0.1 km/h units          */
    bool     platform_side_ok;   /* doors are on the platform side          */
    bool     driver_override;    /* driver confirmed release manually       */
    bool     brake_fault;        /* emergency brake system reports a fault  */
} di_inputs_t;

bool di_train_stopped(uint16_t speed_kmh_x10);
bool di_door_release_permitted(const di_inputs_t *in);

#endif
