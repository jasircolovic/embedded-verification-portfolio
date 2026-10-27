/* MC/DC test set for: A && (B || C) && !D
 *   A = train stopped, B = platform side ok, C = driver override, D = brake fault
 * N conditions need at least N+1 tests for MC/DC: 4 conditions -> 5 vectors.
 * See MCDC_ANALYSIS.md for the independence pairs.
 * Build with -DPARTIAL_SUITE to run only the "obvious" tests and watch
 * gcov report the conditions whose independent effect is not shown. */
#include "unity.h"
#include "door_interlock.h"

void setUp(void) {}
void tearDown(void) {}

static bool permit(uint16_t speed, bool b, bool c, bool d)
{
    di_inputs_t in = { speed, b, c, d };
    return di_door_release_permitted(&in);
}

#define STOPPED  ((uint16_t)0U)
#define MOVING   ((uint16_t)50U)   /* 5.0 km/h */

/*            A        B      C      D       result */
void test_V1_all_ok(void)                 { TEST_ASSERT_TRUE (permit(STOPPED, true,  false, false)); }
void test_V2_moving(void)                 { TEST_ASSERT_FALSE(permit(MOVING,  true,  false, false)); }
#ifndef PARTIAL_SUITE
void test_V3_wrong_side_no_override(void) { TEST_ASSERT_FALSE(permit(STOPPED, false, false, false)); }
void test_V4_wrong_side_override(void)    { TEST_ASSERT_TRUE (permit(STOPPED, false, true,  false)); }
#endif
void test_V5_brake_fault(void)            { TEST_ASSERT_FALSE(permit(STOPPED, true,  false, true )); }

/* Boundary tests for the standstill threshold and NULL robustness. */
void test_standstill_boundary(void)
{
    TEST_ASSERT_TRUE (di_train_stopped(2U));   /* 0.2 km/h */
    TEST_ASSERT_FALSE(di_train_stopped(3U));   /* 0.3 km/h */
}
void test_null_input_keeps_doors_locked(void)
{
    TEST_ASSERT_FALSE(di_door_release_permitted(NULL));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_V1_all_ok);
    RUN_TEST(test_V2_moving);
#ifndef PARTIAL_SUITE
    RUN_TEST(test_V3_wrong_side_no_override);
    RUN_TEST(test_V4_wrong_side_override);
#endif
    RUN_TEST(test_V5_brake_fault);
    RUN_TEST(test_standstill_boundary);
    RUN_TEST(test_null_input_keeps_doors_locked);
    return UNITY_END();
}
