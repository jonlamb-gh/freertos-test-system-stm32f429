#!/bin/bash

set -euo pipefail

export RUST_LOG=modality_trace_recorder=info
#export RUST_LOG=modality_trace_recorder=debug,warn

modality-reflector run --config config/reflector_config.toml --collector trace-recorder-rtt

modality workspace sync-indices

exit 0
