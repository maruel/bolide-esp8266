// Copyright 2019 Marc-Antoine Ruel. All rights reserved.
// Use of this source code is governed under the Apache License, Version 2.0
// that can be found in the LICENSE file.

#include "nodes.h"

// Respects https://homieiot.github.io/specification/#boolean
int isBool(const String& value) {
  if (value.equals("true")) {
    return 1;
  }
  if (value.equals("false")) {
    return 0;
  }
  return -1;
}

int toInt(const String& value, int min, int max) {
  int v = value.toInt();
  if (v < min) {
    return min;
  }
  if (v > max) {
    return max;
  }
  return v;
}
