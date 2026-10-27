/* Unit tests for temp_monitor. Each test names the requirement(s) it verifies;
 * TEST_SPEC.md holds the full traceability matrix. */
#include "unity.h"
#include "temp_monitor.h"

static tm_ctx_t ctx;
static tm_state_t st;

void setUp(void)    { (void)tm_init(&ctx); }
void tearDown(void) { }

static tm_state_t feed(int16_t t)
{
    TEST_ASSERT_EQUAL(TM_OK, tm_update(&ctx, t, &st));
    return st;
}

/* --- Classification, boundary value analysis (REQ-TM-01/02/03) --- */
void test_TC01_normal_mid_partition(void)        { TEST_ASSERT_EQUAL(TM_NORMAL,   feed(250));  }
void test_TC02_normal_upper_boundary_849(void)   { TEST_ASSERT_EQUAL(TM_NORMAL,   feed(849));  }
void test_TC03_warning_lower_boundary_850(void)  { TEST_ASSERT_EQUAL(TM_WARNING,  feed(850));  }
void test_TC04_warning_upper_boundary_1049(void) { TEST_ASSERT_EQUAL(TM_WARNING,  feed(1049)); }
void test_TC05_shutdown_lower_boundary_1050(void){ TEST_ASSERT_EQUAL(TM_SHUTDOWN, feed(1050)); }

/* --- Sensor range (REQ-TM-04) --- */
void test_TC06_min_valid_minus400(void)          { TEST_ASSERT_EQUAL(TM_NORMAL,       feed(-400)); }
void test_TC07_below_range_minus401(void)        { TEST_ASSERT_EQUAL(TM_SENSOR_FAULT, feed(-401)); }
void test_TC08_max_valid_1500(void)              { TEST_ASSERT_EQUAL(TM_SHUTDOWN,     feed(1500)); }
void test_TC09_above_range_1501(void)            { TEST_ASSERT_EQUAL(TM_SENSOR_FAULT, feed(1501)); }
void test_TC10_robustness_int16_extremes(void)
{
    TEST_ASSERT_EQUAL(TM_SENSOR_FAULT, feed(INT16_MIN));
    TEST_ASSERT_EQUAL(TM_SENSOR_FAULT, feed(INT16_MAX));
}

/* --- Hysteresis (REQ-TM-05) --- */
void test_TC11_hysteresis_holds_warning_at_800(void)
{
    feed(900);
    TEST_ASSERT_EQUAL(TM_WARNING, feed(800));
}
void test_TC12_hysteresis_releases_at_799(void)
{
    feed(900);
    TEST_ASSERT_EQUAL(TM_NORMAL, feed(799));
}
void test_TC13_no_hysteresis_when_rising_from_normal(void)
{
    feed(500);
    TEST_ASSERT_EQUAL(TM_NORMAL, feed(820));
}

/* --- Latch and reset (REQ-TM-06/07) --- */
void test_TC14_shutdown_latches_when_cooling(void)
{
    feed(1100);
    TEST_ASSERT_EQUAL(TM_SHUTDOWN, feed(200));
}
void test_TC15_reset_accepted_below_800(void)
{
    feed(1100);
    TEST_ASSERT_EQUAL(TM_OK, tm_reset(&ctx, 799));
    TEST_ASSERT_EQUAL(TM_NORMAL, feed(500));
}
void test_TC16_reset_rejected_at_800(void)
{
    feed(1100);
    TEST_ASSERT_EQUAL(TM_ERR_REJECTED, tm_reset(&ctx, 800));
    TEST_ASSERT_EQUAL(TM_SHUTDOWN, feed(500));
}
void test_TC17_reset_rejected_on_invalid_temp(void)
{
    feed(1100);
    TEST_ASSERT_EQUAL(TM_ERR_REJECTED, tm_reset(&ctx, -401));
    TEST_ASSERT_EQUAL(TM_SHUTDOWN, feed(500));
}

/* --- Fail-safe on repeated sensor faults (REQ-TM-08) --- */
void test_TC18_two_invalid_samples_do_not_latch(void)
{
    feed(-500);
    feed(-500);
    TEST_ASSERT_EQUAL(TM_NORMAL, feed(300));
}
void test_TC19_three_invalid_samples_latch_shutdown(void)
{
    feed(-500);
    feed(-500);
    TEST_ASSERT_EQUAL(TM_SHUTDOWN, feed(-500));
    TEST_ASSERT_EQUAL(TM_SHUTDOWN, feed(300));
}
void test_TC20_valid_sample_resets_invalid_counter(void)
{
    feed(-500);
    feed(-500);
    feed(300);
    feed(-500);
    TEST_ASSERT_EQUAL(TM_SENSOR_FAULT, feed(-500));
}

/* Added after coverage analysis showed the counter-saturation branch was never
 * exercised: many invalid samples must not overflow the counter or unlatch. */
void test_TC24_invalid_counter_saturates(void)
{
    for (int i = 0; i < 300; i++) {
        feed(-500);
    }
    TEST_ASSERT_EQUAL_UINT8(TM_MAX_INVALID, ctx.invalid_count);
    TEST_ASSERT_EQUAL(TM_SHUTDOWN, feed(300));
}

/* --- Power limit (REQ-TM-09) --- */
void test_TC21_power_limit_per_state(void)
{
    TEST_ASSERT_EQUAL_UINT8(100U, tm_power_limit_pct(TM_NORMAL));
    TEST_ASSERT_EQUAL_UINT8(50U,  tm_power_limit_pct(TM_WARNING));
    TEST_ASSERT_EQUAL_UINT8(0U,   tm_power_limit_pct(TM_SHUTDOWN));
    TEST_ASSERT_EQUAL_UINT8(0U,   tm_power_limit_pct(TM_SENSOR_FAULT));
}
void test_TC22_power_limit_unknown_state_fails_safe(void)
{
    TEST_ASSERT_EQUAL_UINT8(0U, tm_power_limit_pct((tm_state_t)99));
}

/* --- NULL pointer robustness (REQ-TM-10) --- */
void test_TC23_null_pointers_rejected_state_unchanged(void)
{
    feed(900);
    TEST_ASSERT_EQUAL(TM_ERR_NULL, tm_update(NULL, 500, &st));
    TEST_ASSERT_EQUAL(TM_ERR_NULL, tm_update(&ctx, 500, NULL));
    TEST_ASSERT_EQUAL(TM_ERR_NULL, tm_reset(NULL, 500));
    TEST_ASSERT_EQUAL(TM_ERR_NULL, tm_init(NULL));
    TEST_ASSERT_EQUAL(TM_WARNING, ctx.state);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_TC01_normal_mid_partition);
    RUN_TEST(test_TC02_normal_upper_boundary_849);
    RUN_TEST(test_TC03_warning_lower_boundary_850);
    RUN_TEST(test_TC04_warning_upper_boundary_1049);
    RUN_TEST(test_TC05_shutdown_lower_boundary_1050);
    RUN_TEST(test_TC06_min_valid_minus400);
    RUN_TEST(test_TC07_below_range_minus401);
    RUN_TEST(test_TC08_max_valid_1500);
    RUN_TEST(test_TC09_above_range_1501);
    RUN_TEST(test_TC10_robustness_int16_extremes);
    RUN_TEST(test_TC11_hysteresis_holds_warning_at_800);
    RUN_TEST(test_TC12_hysteresis_releases_at_799);
    RUN_TEST(test_TC13_no_hysteresis_when_rising_from_normal);
    RUN_TEST(test_TC14_shutdown_latches_when_cooling);
    RUN_TEST(test_TC15_reset_accepted_below_800);
    RUN_TEST(test_TC16_reset_rejected_at_800);
    RUN_TEST(test_TC17_reset_rejected_on_invalid_temp);
    RUN_TEST(test_TC18_two_invalid_samples_do_not_latch);
    RUN_TEST(test_TC19_three_invalid_samples_latch_shutdown);
    RUN_TEST(test_TC20_valid_sample_resets_invalid_counter);
    RUN_TEST(test_TC24_invalid_counter_saturates);
    RUN_TEST(test_TC21_power_limit_per_state);
    RUN_TEST(test_TC22_power_limit_unknown_state_fails_safe);
    RUN_TEST(test_TC23_null_pointers_rejected_state_unchanged);
    return UNITY_END();
}
