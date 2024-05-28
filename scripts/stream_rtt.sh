#!/bin/bash

set -euo pipefail

#export RUST_LOG=modality_trace_recorder=info
export RUST_LOG=modality_trace_recorder=debug,modality_trace_recorder_rtt_collector=debug,warn
#export RUST_LOG=modality_trace_recorder=debug,modality_trace_recorder_rtt_collector=trace,warn
#export RUST_LOG=modality_trace_recorder=error,modality_trace_recorder_rtt_collector=info

modality-reflector run --config config/reflector_config.toml --collector trace-recorder-rtt

modality workspace sync-indices

exit 0
