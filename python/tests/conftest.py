"""Shared fixtures for the pyspla test suite."""

import sys
from pathlib import Path

import pytest

# The algorithm demo modules live in python/algorithms and are not part of
# the installed package; make them importable from the tests.
ALGORITHMS_DIR = Path(__file__).resolve().parent.parent / "algorithms"
if str(ALGORITHMS_DIR) not in sys.path:
    sys.path.insert(0, str(ALGORITHMS_DIR))


@pytest.fixture(scope="session")
def data_dir() -> Path:
    """Directory with the committed Matrix Market test fixtures."""
    return Path(__file__).resolve().parent / "data"
