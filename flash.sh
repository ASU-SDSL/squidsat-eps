#!/usr/bin/env bash

set -euo pipefail

BUILD_DIR="${1:-build}"
BIN_FILE="${BUILD_DIR}/zephyr/zephyr.bin"

if ! command -v st-flash >/dev/null 2>&1; then
  echo "error: st-flash not found in PATH" >&2
  echo "install stlink tools first (for macOS: brew install stlink)" >&2
  exit 1
fi

if [ ! -f "$BIN_FILE" ]; then
  echo "Firmware binary not found at $BIN_FILE"
  echo "Build first: .venv/bin/west build -p always -b nucleo_f103rb app" >&2
  exit 1
fi

echo "Flashing $BIN_FILE with st-flash..."
st-flash --connect-under-reset write "$BIN_FILE" 0x08000000

echo "Flash complete."
