#!/bin/sh
# Copyright 2016 Marc-Antoine Ruel. All rights reserved.
# Use of this source code is governed under the Apache License, Version 2.0
# that can be found in the LICENSE file.

set -eu

# Append platformio tool from Atom's package as the default if it wasn't
# installed system wide.
if [ -d $HOME/.atom/packages/platformio-ide/penv/bin ]; then
  PATH="$PATH:$HOME/.atom/packages/platformio-ide/penv/bin"
  hash -r
fi

PLATFORMIO="$(which platformio)"

echo ""
echo "Build once to make sure the firmware is good before flashing"
echo ""
platformio run

echo ""
echo "Building SPIFFS"
echo ""
platformio run --target buildfs

echo ""
echo "Flashing SPIFFS"
echo "This erases all saved settings"
echo ""
platformio run --target uploadfs

echo ""
echo "Flashing code"
echo ""
platformio run --target upload

echo ""
echo "Congratulations!"
echo "Run the following to monitor the device over the serial port:"
echo ""
echo "  $PLATFORMIO serialports monitor --baud 115200"
