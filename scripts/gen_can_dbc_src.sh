#!/bin/bash

set -euo pipefail

python3 -m cantools generate_c_source \
    --no-floating-point-numbers \
    -o generated \
    dbc/canproto.dbc

exit 0
