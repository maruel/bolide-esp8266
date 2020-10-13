#!/bin/bash
# Copyright 2019 Marc-Antoine Ruel. All rights reserved.
# Use of this source code is governed under the Apache License, Version 2.0
# that can be found in the LICENSE file.

# Only update code, not data.

set -eu

cd "$(dirname $0)"

./setup.sh

source .venv/bin/activate

echo "- Flashing code"
pio run --target upload -s

echo ""
echo "Congratulations!"
echo "If you want to use debugging, enable it in platform.ini,"
echo "then you can monitor the device over the serial port:"
echo "  .venv/bin/pio device monitor"
