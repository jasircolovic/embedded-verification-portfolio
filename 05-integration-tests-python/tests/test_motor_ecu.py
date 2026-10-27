"""Integration tests for the motor controller ECU (black box, via its serial
protocol). Each test names the interface requirement from PROTOCOL.md."""
import random

import pytest
from ecu_client import (ESTOP, ESTOP_STATE, GET_STATUS, IDLE, LEN_ERR, OK,
                        RANGE_ERR, REJECTED, RESET, RUNNING, SET_SPEED,
                        UNKNOWN, CRC_ERR, frame, rpm_payload)


def test_IF01_boot_state_is_idle(ecu):
    assert ecu.status() == (IDLE, 0)


@pytest.mark.parametrize("rpm", [1, 1500, 3000])
def test_IF02_valid_speed_starts_motor(ecu, rpm):
    assert ecu.request(SET_SPEED, rpm_payload(rpm)) == (SET_SPEED, bytes([OK]))
    assert ecu.status() == (RUNNING, rpm)


def test_IF02_zero_speed_goes_idle(ecu):
    ecu.request(SET_SPEED, rpm_payload(1000))
    ecu.request(SET_SPEED, rpm_payload(0))
    assert ecu.status() == (IDLE, 0)


@pytest.mark.parametrize("rpm", [3001, -1, 32767, -32768])
def test_IF03_out_of_range_rejected_state_unchanged(ecu, rpm):
    ecu.request(SET_SPEED, rpm_payload(500))
    assert ecu.request(SET_SPEED, rpm_payload(rpm)) == (SET_SPEED, bytes([RANGE_ERR]))
    assert ecu.status() == (RUNNING, 500)


def test_IF04_bad_crc_rejected_no_effect(ecu):
    resp = ecu.request(SET_SPEED, rpm_payload(1000), corrupt_crc=True)
    assert resp == (SET_SPEED, bytes([CRC_ERR]))
    assert ecu.status() == (IDLE, 0)


def test_IF05_unknown_command(ecu):
    assert ecu.request(0x7E) == (0x7E, bytes([UNKNOWN]))


@pytest.mark.parametrize("cmd,payload", [
    (SET_SPEED, b"\x01"), (SET_SPEED, b"\x00\x01\x02"),
    (GET_STATUS, b"\x00"), (ESTOP, b"\x00"), (RESET, b"\x00"),
])
def test_IF05_wrong_length(ecu, cmd, payload):
    assert ecu.request(cmd, payload) == (cmd, bytes([LEN_ERR]))


def test_IF05_oversized_length_field_dropped(ecu):
    ecu.write(bytes([0xAA, SET_SPEED, 200]))
    assert ecu.read_response() == (SET_SPEED, bytes([LEN_ERR]))
    assert ecu.status() == (IDLE, 0)          # parser recovered


def test_IF06_estop_stops_running_motor(ecu):
    ecu.request(SET_SPEED, rpm_payload(2500))
    assert ecu.request(ESTOP) == (ESTOP, bytes([OK]))
    assert ecu.status() == (ESTOP_STATE, 0)


def test_IF07_speed_rejected_during_estop(ecu):
    ecu.request(ESTOP)
    assert ecu.request(SET_SPEED, rpm_payload(100)) == (SET_SPEED, bytes([REJECTED]))
    assert ecu.status() == (ESTOP_STATE, 0)


def test_IF08_reset_leaves_estop(ecu):
    ecu.request(ESTOP)
    assert ecu.request(RESET) == (RESET, bytes([OK]))
    assert ecu.status() == (IDLE, 0)
    assert ecu.request(SET_SPEED, rpm_payload(100)) == (SET_SPEED, bytes([OK]))


def test_IF08_reset_outside_estop_rejected(ecu):
    assert ecu.request(RESET) == (RESET, bytes([REJECTED]))


def test_IF09_resync_after_garbage(ecu):
    ecu.write(b"\x00\x13\x37\xff")             # line noise before the frame
    assert ecu.status() == (IDLE, 0)


def test_IF09_two_frames_in_one_write(ecu):
    ecu.write(frame(SET_SPEED, rpm_payload(700)) + frame(GET_STATUS))
    assert ecu.read_response() == (SET_SPEED, bytes([OK]))
    cmd, p = ecu.read_response()
    assert (cmd, p[1], int.from_bytes(p[2:4], "big")) == (GET_STATUS, RUNNING, 700)


def test_IF09_frame_split_byte_by_byte(ecu):
    for byte in frame(SET_SPEED, rpm_payload(1234)):
        ecu.write(bytes([byte]))
    assert ecu.read_response() == (SET_SPEED, bytes([OK]))
    assert ecu.status() == (RUNNING, 1234)


def test_IF10_fuzz_random_frames_never_crash(ecu):
    """Robustness: 500 random well-framed requests (random command, payload,
    sometimes a broken CRC). Every one must get a valid response frame, and
    the ECU must still answer correctly afterwards."""
    rng = random.Random(2026)                  # fixed seed = reproducible
    for _ in range(500):
        cmd = rng.randrange(256)
        payload = bytes(rng.randrange(256) for _ in range(rng.randrange(9)))
        resp_cmd, resp = ecu.request(cmd, payload, corrupt_crc=rng.random() < 0.2)
        assert resp_cmd == cmd & 0x7F
        assert resp[0] in range(6)
    ecu.request(ESTOP)
    ecu.request(RESET)
    assert ecu.status() == (IDLE, 0)
