/* Simulated motor controller ECU. See PROTOCOL.md.
 * stdin = UART RX, stdout = UART TX. The frame parser is a byte-wise state
 * machine, as it would be in a UART receive interrupt on a real MCU. */
#include <stdint.h>
#include <stdio.h>

#define SOF            0xAAU
#define MAX_PAYLOAD    8U
#define MAX_RPM        3000

enum { CMD_SET_SPEED = 0x01, CMD_GET_STATUS = 0x02, CMD_ESTOP = 0x03, CMD_RESET = 0x04 };
enum { ST_OK = 0x00, ST_CRC = 0x01, ST_UNKNOWN = 0x02, ST_LEN = 0x03, ST_RANGE = 0x04, ST_REJECTED = 0x05 };
enum { STATE_IDLE = 0, STATE_RUNNING = 1, STATE_ESTOP = 2 };
typedef enum { WAIT_SOF, WAIT_CMD, WAIT_LEN, WAIT_PAYLOAD, WAIT_CRC } rx_state_t;

static uint8_t state = STATE_IDLE;
static int16_t rpm = 0;

static uint8_t crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFFU;
    for (uint8_t i = 0U; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0U; b < 8U; b++) {
            crc = ((crc & 0x80U) != 0U) ? (uint8_t)((uint8_t)(crc << 1U) ^ 0x31U)
                                        : (uint8_t)(crc << 1U);
        }
    }
    return crc;
}

static void send(uint8_t cmd, const uint8_t *payload, uint8_t len)
{
    uint8_t buf[3U + MAX_PAYLOAD + 1U];
    buf[0] = SOF;
    buf[1] = (uint8_t)(cmd | 0x80U);
    buf[2] = len;
    for (uint8_t i = 0U; i < len; i++) {
        buf[3U + i] = payload[i];
    }
    buf[3U + len] = crc8(&buf[1], (uint8_t)(len + 2U));
    (void)fwrite(buf, 1U, (size_t)len + 4U, stdout);
    (void)fflush(stdout);
}

static void send_status(uint8_t cmd, uint8_t status)
{
    send(cmd, &status, 1U);
}

static void handle(uint8_t cmd, const uint8_t *p, uint8_t len)
{
    switch (cmd) {
    case CMD_SET_SPEED: {
        if (len != 2U) { send_status(cmd, ST_LEN); break; }
        int16_t req = (int16_t)(uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
        if (state == STATE_ESTOP)          { send_status(cmd, ST_REJECTED); break; }
        if ((req < 0) || (req > MAX_RPM))  { send_status(cmd, ST_RANGE); break; }
        rpm = req;
        state = (rpm > 0) ? STATE_RUNNING : STATE_IDLE;
        send_status(cmd, ST_OK);
        break;
    }
    case CMD_GET_STATUS: {
        if (len != 0U) { send_status(cmd, ST_LEN); break; }
        uint8_t out[4] = { ST_OK, state, (uint8_t)((uint16_t)rpm >> 8U), (uint8_t)((uint16_t)rpm & 0xFFU) };
        send(cmd, out, 4U);
        break;
    }
    case CMD_ESTOP:
        if (len != 0U) { send_status(cmd, ST_LEN); break; }
        rpm = 0;
        state = STATE_ESTOP;
        send_status(cmd, ST_OK);
        break;
    case CMD_RESET:
        if (len != 0U)              { send_status(cmd, ST_LEN); break; }
        if (state != STATE_ESTOP)   { send_status(cmd, ST_REJECTED); break; }
        state = STATE_IDLE;
        send_status(cmd, ST_OK);
        break;
    default:
        send_status(cmd, ST_UNKNOWN);
        break;
    }
}

int main(void)
{
    rx_state_t rx = WAIT_SOF;
    uint8_t frame[2U + MAX_PAYLOAD];   /* CMD, LEN, PAYLOAD -- CRC is computed over this */
    uint8_t idx = 0U;
    int c;

    while ((c = getchar()) != EOF) {
        uint8_t byte = (uint8_t)c;
        switch (rx) {
        case WAIT_SOF:
            if (byte == SOF) { rx = WAIT_CMD; }            /* else: discard, resync */
            break;
        case WAIT_CMD:
            frame[0] = byte;
            rx = WAIT_LEN;
            break;
        case WAIT_LEN:
            frame[1] = byte;
            idx = 0U;
            if (byte > MAX_PAYLOAD) {                       /* cannot be valid: drop */
                send_status(frame[0], ST_LEN);
                rx = WAIT_SOF;
            } else {
                rx = (byte == 0U) ? WAIT_CRC : WAIT_PAYLOAD;
            }
            break;
        case WAIT_PAYLOAD:
            frame[2U + idx] = byte;
            idx++;
            if (idx == frame[1]) { rx = WAIT_CRC; }
            break;
        case WAIT_CRC:
        default:
            if (crc8(frame, (uint8_t)(frame[1] + 2U)) != byte) {
                send_status(frame[0], ST_CRC);
            } else {
                handle(frame[0], &frame[2], frame[1]);
            }
            rx = WAIT_SOF;
            break;
        }
    }
    return 0;
}
