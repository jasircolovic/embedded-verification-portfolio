"""Test-bench side of the protocol: frame building, CRC and a client that
talks to the ECU process. On a real bench this class would wrap a serial port
(pyserial) instead of a subprocess; the tests would not change."""
import os
import select
import subprocess

SOF = 0xAA
SET_SPEED, GET_STATUS, ESTOP, RESET = 0x01, 0x02, 0x03, 0x04
OK, CRC_ERR, UNKNOWN, LEN_ERR, RANGE_ERR, REJECTED = range(6)
IDLE, RUNNING, ESTOP_STATE = 0, 1, 2


def crc8(data: bytes) -> int:
    crc = 0xFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ 0x31) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc


def frame(cmd: int, payload: bytes = b"", corrupt_crc: bool = False) -> bytes:
    body = bytes([cmd, len(payload)]) + payload
    crc = crc8(body) ^ (0x01 if corrupt_crc else 0x00)
    return bytes([SOF]) + body + bytes([crc])


def rpm_payload(rpm: int) -> bytes:
    return (rpm & 0xFFFF).to_bytes(2, "big")


class EcuClient:
    def __init__(self, binary: str, timeout: float = 1.0):
        self.timeout = timeout
        self.proc = subprocess.Popen([binary], stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, bufsize=0)

    def write(self, data: bytes) -> None:
        self.proc.stdin.write(data)
        self.proc.stdin.flush()

    def _read_exact(self, n: int) -> bytes:
        out = b""
        while len(out) < n:
            ready, _, _ = select.select([self.proc.stdout], [], [], self.timeout)
            if not ready:
                raise TimeoutError(f"ECU did not respond (got {out.hex()})")
            chunk = os.read(self.proc.stdout.fileno(), n - len(out))
            if not chunk:
                raise ConnectionError("ECU closed its output (crashed?)")
            out += chunk
        return out

    def read_response(self):
        """Returns (cmd, payload) and checks the response frame itself."""
        head = self._read_exact(3)
        assert head[0] == SOF, f"bad start byte {head[0]:#x}"
        rest = self._read_exact(head[2] + 1)
        payload, crc = rest[:-1], rest[-1]
        assert crc == crc8(head[1:] + payload), "ECU response has bad CRC"
        assert head[1] & 0x80, "response bit not set"
        return head[1] & 0x7F, payload

    def request(self, cmd: int, payload: bytes = b"", corrupt_crc: bool = False):
        self.write(frame(cmd, payload, corrupt_crc))
        return self.read_response()

    def status(self):
        cmd, p = self.request(GET_STATUS)
        assert cmd == GET_STATUS and p[0] == OK
        return p[1], int.from_bytes(p[2:4], "big")

    def close(self) -> int:
        self.proc.stdin.close()
        return self.proc.wait(timeout=self.timeout)
