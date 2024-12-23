#!/bin/bash
# https://github.com/jonlamb-gh/rtt-reader

set -euo pipefail

export RUST_LOG=rtt_reader=debug,warn

mkdir -p /tmp/test-system-trace
cp ./tracing/trace-schema/schema/metadata /tmp/test-system-trace/

rtt-reader --chip STM32F429ZITx --speed 40000 --reset --up-channel 2 --thumb --breakpoint main --elf-file build/firmware --output /tmp/test-system-trace/stream

exit 0
