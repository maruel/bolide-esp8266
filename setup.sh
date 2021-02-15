#!/bin/bash
# Copyright 2020 Marc-Antoine Ruel. All rights reserved.
# Use of this source code is governed under the Apache License, Version 2.0
# that can be found in the LICENSE file.

set -eu

cd "$(dirname $0)"

# Set it to -v for verbosity.
QUIET=-q

if [ -f .venv/bin/esphome ]; then
  exit 0
fi

if [ ! -d .venv ]; then
  mkdir .venv
fi

if [ ! -f ./.venv/bin/activate ]; then
  VIRTUALENV="$(which virtualenv || true)"
  if [ "$VIRTUALENV" = "" ]; then
    echo "- Getting virtualenv"
    wget --quiet https://bootstrap.pypa.io/virtualenv.pyz -O .venv/virtualenv.pyz
    #curl -SLs https://bootstrap.pypa.io/virtualenv.pyz > .venv/virtualenv.pyz
    echo "- Creating virtualenv"
    # Fails on python 3.4 to 3.6 is the path is not absolute.
    python3 "$(pwd)/.venv/virtualenv.pyz" $QUIET .venv
    rm .venv/virtualenv.pyz
  else
    # Reuse the preinstalled virtualenv tool if it exists.
    echo "- Creating virtualenv"
    $VIRTUALENV $QUIET .venv
  fi
fi

echo "- Activating virtualenv"
source ./.venv/bin/activate

echo "- Installing requirements"
pip3 install $QUIET -U -r requirements.txt

echo ""
echo "Congratulations! Everything is inside ./.venv/"
echo "To access esphome, run:"
echo "  source ./.venv/bin/activate"
