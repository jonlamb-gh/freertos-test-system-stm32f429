#!/bin/bash

set -euo pipefail

candump can0 | python3 -m cantools decode dbc/canproto.dbc

exit 0
