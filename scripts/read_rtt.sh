#!/bin/bash
# https://github.com/jonlamb-gh/rtt-reader

set -euo pipefail

export RUST_LOG=rtt_reader=debug,warn

rtt-reader --chip STM32F429ZITx --speed 40000 --reset --up-channel 1 --thumb --breakpoint main --elf-file build/firmware --output /tmp/trc.psf

exit 0
