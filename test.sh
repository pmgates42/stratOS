#!/usr/bin/env bash

set -euo pipefail

APP_NAME="bcm2xx_sched_rate"

if [[ $# -gt 0 ]]; then
	APP_NAME="$1"
	shift
fi

python3 strat_build.py "${APP_NAME}" --app-config test_apps/apps.json "$@"

SRC_DIR="build/bcm2xxx"
IMG_PATH="${SRC_DIR}/kernel8.img"
ELF_PATH="${SRC_DIR}/kernel8.elf"

if [[ ! -f "${IMG_PATH}" || ! -f "${ELF_PATH}" ]]; then
	echo "Expected build outputs not found in ${SRC_DIR}" >&2
	exit 1
fi

WIN_IMG_PATH="$(wslpath -w "${IMG_PATH}")"
WIN_ELF_PATH="$(wslpath -w "${ELF_PATH}")"
PUTTY_SESSION="rpi_3b_plus__bcm2xxx_115200"

powershell.exe -NoProfile -Command "if (-not (Test-Path 'D:\\')) { Write-Error 'D:\\ is not available'; exit 1 }"
powershell.exe -NoProfile -Command "Copy-Item -LiteralPath '${WIN_IMG_PATH}' -Destination 'D:\\kernel8.img' -Force"
powershell.exe -NoProfile -Command "Copy-Item -LiteralPath '${WIN_ELF_PATH}' -Destination 'D:\\kernel8.elf' -Force"

echo "Copied kernel8.img and kernel8.elf to D:\\"

read -r -p "Press Enter to launch PuTTY session '${PUTTY_SESSION}'... " _

powershell.exe -NoProfile -Command "if (-not (Get-Command putty.exe -ErrorAction SilentlyContinue)) { Write-Error 'putty.exe not found in PATH'; exit 1 }"
powershell.exe -NoProfile -Command "Start-Process -FilePath 'putty.exe' -ArgumentList @('-load', '${PUTTY_SESSION}')"

echo "PuTTY launched with session '${PUTTY_SESSION}'."