#!/usr/bin/env bash

# Usage:
#   ./connection.sh                 # auto-detect serial device
#   ./connection.sh /dev/ttyS4      # explicit serial device

set -euo pipefail

PORT="${1:-}"

if [ -z "$PORT" ]; then
	# WSL/Linux serial candidates:
	# - /dev/ttyUSB* and /dev/ttyACM* for USB serial adapters
	# - /dev/ttyS* for Windows COM ports mapped into WSL
	PORT=$(ls /dev/ttyUSB* /dev/ttyACM* /dev/ttyS* 2>/dev/null | head -n 1 || true)
fi

if [ -z "$PORT" ]; then
	echo "No serial device found."
	echo "Try: ./connection.sh /dev/ttyS4"
	exit 1
fi

echo "Connecting to $PORT at 115200 baud..."
screen "$PORT" 115200
