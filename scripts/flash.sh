#!/bin/bash

set -euo pipefail

st-flash write build/firmware.bin 0x08000000
st-flash reset

exit 0
