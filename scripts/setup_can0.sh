#!/bin/bash
# sudo apt-get install can-utils

set -euo pipefail

ip link set can0 type can bitrate 500000
ip link set can0 type can restart-ms 100
#ip link set can0 type can restart

# UP->DOWN->UP to clear all internal buffers/state
ip link set can0 up
ip link set can0 down
ip link set can0 up

ip -details link show can0

exit 0
