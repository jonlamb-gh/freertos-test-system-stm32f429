#!/bin/bash

set -euo pipefail

export RUST_LOG=modality_trace_recorder=info
#export RUST_LOG=modality_trace_recorder=debug,warn

modality-reflector import --config config/reflector_config.toml trace-recorder /tmp/rtt.bin

modality workspace sync-indices

exit 0
