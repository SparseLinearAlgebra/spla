import pathlib
import platform

ROOT = pathlib.Path(__file__).parent.parent
INCLUDE = ROOT / "include"
SOURCES = ROOT / "sources"
PACKAGE = ROOT / "package"

ARCH = {'AMD64': 'x64', 'x86_64': 'x64', 'arm64': 'arm64'}[platform.machine()]
SYSTEM = {'Darwin': 'macos', 'Linux': 'linux', 'Windows': 'windows'}[platform.system()]
EXECUTABLE_EXT = {'macos': '', 'windows': '.exe', 'linux': ''}[SYSTEM]
TARGET_SUFFIX = {'macos': '.dylib', 'linux': '.so', 'windows': '.dll'}[SYSTEM]
TARGET = {'macos': 'libspla', 'linux': 'libspla', 'windows': 'spla'}[SYSTEM] + "_" + ARCH + TARGET_SUFFIX
