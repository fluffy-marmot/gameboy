import pytest

@pytest.fixture(scope="session")
def overview():
    yield {
        "blargg": {
            "pass": 0,
            "skip": 0,
            "fail": 0,
        },
    }