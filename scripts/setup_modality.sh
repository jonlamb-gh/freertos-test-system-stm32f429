#!/usr/bin/env bash

set -euo pipefail

modality user create --use admin

modality workspace use default

modality segment use --latest

exit 0
