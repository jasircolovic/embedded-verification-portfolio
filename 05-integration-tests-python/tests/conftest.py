import os
import pytest
from ecu_client import EcuClient

ECU_BINARY = os.environ.get(
    "ECU_BINARY",
    os.path.join(os.path.dirname(__file__), "..", "build", "motor_ecu"))


@pytest.fixture
def ecu():
    """Fresh ECU per test = power cycle, so tests are independent."""
    client = EcuClient(ECU_BINARY)
    yield client
    assert client.close() == 0, "ECU exited with an error code"
