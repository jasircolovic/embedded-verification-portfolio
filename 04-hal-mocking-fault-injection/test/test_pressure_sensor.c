/* Driver tests with a faked I2C bus: nominal behaviour plus fault injection
 * (timeouts, NACKs, corrupted data, impossible values) to prove robustness. */
#include "unity.h"
#include "fake_i2c.h"
#include "pressure_sensor.h"

static uint16_t mbar;

void setUp(void)    { fake_i2c_reset(); mbar = 0U; }
void tearDown(void) { }

/* --- CRC against a published reference vector --- */
void test_crc8_reference_vector(void)
{
    const uint8_t data[2] = { 0xBEU, 0xEFU };   /* Sensirion datasheet example */
    TEST_ASSERT_EQUAL_HEX8(0x92U, ps_crc8(data, 2U));
}

/* --- Nominal --- */
void test_nominal_read(void)
{
    fake_i2c_queue_value(1013U);
    TEST_ASSERT_EQUAL(PS_OK, ps_read_mbar(&mbar));
    TEST_ASSERT_EQUAL_UINT16(1013U, mbar);
    TEST_ASSERT_EQUAL_UINT8(1U, fake_i2c_call_count());
}

void test_driver_uses_correct_address_register_and_length(void)
{
    fake_i2c_queue_value(1013U);
    (void)ps_read_mbar(&mbar);
    TEST_ASSERT_EQUAL_HEX8(PS_I2C_ADDR, fake_i2c_last_addr());
    TEST_ASSERT_EQUAL_HEX8(PS_REG_PRESSURE, fake_i2c_last_reg());
    TEST_ASSERT_EQUAL_UINT8(3U, fake_i2c_last_len());
}

/* --- Fault injection: bus errors --- */
void test_one_timeout_then_success_retries(void)
{
    fake_i2c_queue(HAL_ERR_TIMEOUT, 0U, 0U, 0U);
    fake_i2c_queue_value(950U);
    TEST_ASSERT_EQUAL(PS_OK, ps_read_mbar(&mbar));
    TEST_ASSERT_EQUAL_UINT16(950U, mbar);
    TEST_ASSERT_EQUAL_UINT8(2U, fake_i2c_call_count());
}

void test_nack_then_success_retries(void)
{
    fake_i2c_queue(HAL_ERR_NACK, 0U, 0U, 0U);
    fake_i2c_queue_value(950U);
    TEST_ASSERT_EQUAL(PS_OK, ps_read_mbar(&mbar));
}

void test_dead_bus_gives_comm_error_after_exactly_3_attempts(void)
{
    /* nothing queued: every call times out */
    TEST_ASSERT_EQUAL(PS_ERR_COMM, ps_read_mbar(&mbar));
    TEST_ASSERT_EQUAL_UINT8(PS_MAX_ATTEMPTS, fake_i2c_call_count());
    TEST_ASSERT_EQUAL_UINT16(0U, mbar);                  /* output untouched */
}

void test_persistent_nack_gives_comm_error_after_exactly_3_attempts(void)
{
    /* sensor absent or wrong address: every attempt is NACKed */
    fake_i2c_queue(HAL_ERR_NACK, 0U, 0U, 0U);
    fake_i2c_queue(HAL_ERR_NACK, 0U, 0U, 0U);
    fake_i2c_queue(HAL_ERR_NACK, 0U, 0U, 0U);
    TEST_ASSERT_EQUAL(PS_ERR_COMM, ps_read_mbar(&mbar));
    TEST_ASSERT_EQUAL_UINT8(PS_MAX_ATTEMPTS, fake_i2c_call_count());
    TEST_ASSERT_EQUAL_UINT16(0U, mbar);                  /* output untouched */
}

void test_success_on_third_attempt(void)
{
    fake_i2c_queue(HAL_ERR_TIMEOUT, 0U, 0U, 0U);
    fake_i2c_queue(HAL_ERR_NACK, 0U, 0U, 0U);
    fake_i2c_queue_value(700U);
    TEST_ASSERT_EQUAL(PS_OK, ps_read_mbar(&mbar));
    TEST_ASSERT_EQUAL_UINT8(3U, fake_i2c_call_count());
}

/* --- Fault injection: corrupted data --- */
void test_bad_crc_then_good_frame_retries(void)
{
    fake_i2c_queue_bad_crc(1000U);
    fake_i2c_queue_value(1000U);
    TEST_ASSERT_EQUAL(PS_OK, ps_read_mbar(&mbar));
    TEST_ASSERT_EQUAL_UINT8(2U, fake_i2c_call_count());
}

void test_crc_always_bad_gives_crc_error(void)
{
    fake_i2c_queue_bad_crc(1000U);
    fake_i2c_queue_bad_crc(1000U);
    fake_i2c_queue_bad_crc(1000U);
    TEST_ASSERT_EQUAL(PS_ERR_CRC, ps_read_mbar(&mbar));
    TEST_ASSERT_EQUAL_UINT16(0U, mbar);
}

/* --- Range: boundary values, no retry on a correctly received bad value --- */
void test_range_boundaries(void)
{
    fake_i2c_queue_value(300U);
    TEST_ASSERT_EQUAL(PS_OK, ps_read_mbar(&mbar));
    fake_i2c_reset();
    fake_i2c_queue_value(1100U);
    TEST_ASSERT_EQUAL(PS_OK, ps_read_mbar(&mbar));
}

void test_below_range_rejected_without_retry(void)
{
    fake_i2c_queue_value(299U);
    TEST_ASSERT_EQUAL(PS_ERR_RANGE, ps_read_mbar(&mbar));
    TEST_ASSERT_EQUAL_UINT8(1U, fake_i2c_call_count());
}

void test_above_range_rejected(void)
{
    fake_i2c_queue_value(1101U);
    TEST_ASSERT_EQUAL(PS_ERR_RANGE, ps_read_mbar(&mbar));
}

void test_null_output_pointer(void)
{
    TEST_ASSERT_EQUAL(PS_ERR_NULL, ps_read_mbar(NULL));
    TEST_ASSERT_EQUAL_UINT8(0U, fake_i2c_call_count());  /* no bus access */
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_crc8_reference_vector);
    RUN_TEST(test_nominal_read);
    RUN_TEST(test_driver_uses_correct_address_register_and_length);
    RUN_TEST(test_one_timeout_then_success_retries);
    RUN_TEST(test_nack_then_success_retries);
    RUN_TEST(test_dead_bus_gives_comm_error_after_exactly_3_attempts);
    RUN_TEST(test_persistent_nack_gives_comm_error_after_exactly_3_attempts);
    RUN_TEST(test_success_on_third_attempt);
    RUN_TEST(test_bad_crc_then_good_frame_retries);
    RUN_TEST(test_crc_always_bad_gives_crc_error);
    RUN_TEST(test_range_boundaries);
    RUN_TEST(test_below_range_rejected_without_retry);
    RUN_TEST(test_above_range_rejected);
    RUN_TEST(test_null_output_pointer);
    return UNITY_END();
}
