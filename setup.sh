#!/usr/bin/env bash
# Installs everything you need. Run once: ./setup.sh
set -e

need() { command -v "$1" >/dev/null 2>&1; }

echo "== embedded-forge setup =="

if need apt-get; then
    SUDO=""; [ "$(id -u)" -ne 0 ] && SUDO="sudo"
    $SUDO apt-get update
    $SUDO apt-get install -y build-essential gcc-arm-none-eabi qemu-system-arm
elif need brew; then
    brew install --cask gcc-arm-embedded 2>/dev/null || brew install arm-none-eabi-gcc
    brew install qemu
else
    echo "No apt-get or brew found. Install manually:"
    echo "  - a C compiler (gcc/clang)"
    echo "  - arm-none-eabi-gcc  (ARM bare-metal toolchain)"
    echo "  - qemu-system-arm"
    exit 1
fi

echo
echo "Checking:"
for t in gcc make arm-none-eabi-gcc qemu-system-arm; do
    if need "$t"; then echo "  ok  $t"; else echo "  MISSING  $t"; fi
done
echo
echo "Done. Start with:  cd part1_libc && make test"
