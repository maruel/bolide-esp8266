#!/bin/sh
# Copyright 2016 Marc-Antoine Ruel. All rights reserved.
# Use of this source code is governed under the Apache License, Version 2.0
# that can be found in the LICENSE file.

set -eu

# Append platformio tool from Atom's package as the default if it wasn't
# installed system wide.
PATH="$PATH:$HOME/.atom/packages/platformio-ide/penv/bin"

hash -r
PLATFORMIO="$(which platformio)"

if [ -f lib/homie-esp8266/README.md ]; then
  echo "You had forgot to git clone with --recurse; doing it for you"
  git submodule init
  git submodule update
fi

platformio run --target buildfs
echo ""
echo "Flashing SPIFFS"
echo "This erases all saved settings"
echo ""
platformio run --target uploadfs

echo ""
echo "Flashing code"
platformio run --target upload

echo ""
echo "Congratulations!"
echo "Run the following to monitor the device over the serial port:"
echo ""
echo "  $PLATFORMIO serialports monitor --baud 115200"
