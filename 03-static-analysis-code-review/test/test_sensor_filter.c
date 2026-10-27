/* Regression tests: one test per review finding, so each defect stays fixed. */
#include <string.h>
#include "unity.h"
#include "sensor_filter.h"

void setUp(void)    { sf_reset(); }
void tearDown(void) { }

/* D1: 9th push must wrap to index 0, not write past the buffer. */
void test_D1_buffer_wraps_after_8_samples(void)
{
    uint16_t avg = 0U;
    for (uint16_t i = 0U; i < 8U; i++) {
        TEST_ASSERT_EQUAL(SF_OK, sf_push(100U));
    }
    TEST_ASSERT_EQUAL(SF_OK, sf_push(900U));          /* overwrites oldest */
    TEST_ASSERT_EQUAL(SF_OK, sf_average(&avg));
    TEST_ASSERT_EQUAL_UINT16(200U, avg);              /* (7*100 + 900) / 8 */
}

/* D2/D3: average of exactly the stored samples, no stale or garbage values. */
void test_D3_average_uses_only_stored_samples(void)
{
    uint16_t avg = 0U;
    TEST_ASSERT_EQUAL(SF_OK, sf_push(1000U));
    TEST_ASSERT_EQUAL(SF_OK, sf_push(2000U));
    TEST_ASSERT_EQUAL(SF_OK, sf_average(&avg));
    TEST_ASSERT_EQUAL_UINT16(1500U, avg);
}

/* D4: empty buffer must not divide by zero. */
void test_D4_empty_buffer_returns_error(void)
{
    uint16_t avg = 0U;
    TEST_ASSERT_EQUAL(SF_ERR_EMPTY, sf_average(&avg));
}

/* D5: input range and conversion. */
void test_D5_percent_conversion_and_range(void)
{
    uint8_t pct = 0U;
    TEST_ASSERT_EQUAL(SF_OK, sf_to_percent(4095U, &pct));
    TEST_ASSERT_EQUAL_UINT8(100U, pct);
    TEST_ASSERT_EQUAL(SF_OK, sf_to_percent(0U, &pct));
    TEST_ASSERT_EQUAL_UINT8(0U, pct);
    TEST_ASSERT_EQUAL(SF_ERR_RANGE, sf_to_percent(4096U, &pct));
    TEST_ASSERT_EQUAL(SF_ERR_RANGE, sf_push(4096U));
}

/* D6: comparison, boundary on both sides. */
void test_D6_over_current_threshold(void)
{
    TEST_ASSERT_EQUAL_UINT8(0U, sf_over_current(3499U));
    TEST_ASSERT_EQUAL_UINT8(1U, sf_over_current(3500U));
}

/* D7/D8: names, no fall-through, 100 % and invalid values handled. */
void test_D8_level_names(void)
{
    TEST_ASSERT_EQUAL_STRING("low",      sf_level_name(0U));
    TEST_ASSERT_EQUAL_STRING("medium",   sf_level_name(25U));
    TEST_ASSERT_EQUAL_STRING("high",     sf_level_name(50U));   /* was "critical" */
    TEST_ASSERT_EQUAL_STRING("critical", sf_level_name(75U));
    TEST_ASSERT_EQUAL_STRING("critical", sf_level_name(100U));  /* was unhandled */
    TEST_ASSERT_EQUAL_STRING("invalid",  sf_level_name(200U));
}

void test_null_pointers_rejected(void)
{
    TEST_ASSERT_EQUAL(SF_ERR_NULL, sf_average(NULL));
    TEST_ASSERT_EQUAL(SF_ERR_NULL, sf_to_percent(10U, NULL));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_D1_buffer_wraps_after_8_samples);
    RUN_TEST(test_D3_average_uses_only_stored_samples);
    RUN_TEST(test_D4_empty_buffer_returns_error);
    RUN_TEST(test_D5_percent_conversion_and_range);
    RUN_TEST(test_D6_over_current_threshold);
    RUN_TEST(test_D8_level_names);
    RUN_TEST(test_null_pointers_rejected);
    return UNITY_END();
}
