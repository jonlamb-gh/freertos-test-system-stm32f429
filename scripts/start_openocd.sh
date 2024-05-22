#!/bin/bash

set -euo pipefail

openocd -f /usr/share/openocd/scripts/board/st_nucleo_f4.cfg

exit 0
