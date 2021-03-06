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

const char urlEncodeSpecials[] = "$&+,/:;=?@ <>#%{}|~[]`";

char hexDigit(char c) {
  return "01234567890ABCDEF"[c & 0x0F];
}

String urlencode(const String& src) {
  String out;
  for (unsigned int i = 0; i < src.length(); i++) {
    char c = src[i];
     if (strchr(urlEncodeSpecials, c)) {
       out += '%';
       out += hexDigit(c >> 4);
       out += hexDigit(c);
     } else {
       out += c;
     }
   }
   return out;
}

// Pins.

int PinPWM::set(int v) {
  if (v <= 0) {
    analogWrite(pin, 0);
    value_ = 0;
  } else if (v >= PWMRANGE) {
    analogWrite(pin, PWMRANGE);
    value_ = PWMRANGE;
  } else {
    analogWrite(pin, v);
    value_ = v;
  }
  return value_;
}

int PinTone::set(int freq, int duration) {
  if (freq <= 0) {
    noTone(pin);
    freq_ = 0;
  } else if (freq >= 10000) {
    tone(pin, freq, duration);
    freq_ = 10000;
  } else {
    tone(pin, freq, duration);
    freq_ = freq;
  }
  return freq_;
}

// Homie nodes.

bool PinOutNode::_from_mqtt(const String &value) {
  Homie.getLogger() << getId() << "._from_mqtt(" << value << ")" << endl;
  int v = isBool(value);
  if (v == -1) {
    Homie.getLogger() << "  bad value" << endl;
    return false;
  }
  pin_.set(v != 0);
  if (onSet_ != NULL) {
    onSet_(v);
  }
  setProperty("on").send(value);
  return true;
}

bool PinPWMNode::_from_mqtt(const String &value) {
  Homie.getLogger() << getId() << "._from_mqtt(" << value << ")" << endl;
  int v = toInt(value, 0, PWMRANGE);
  set(v);
  if (onSet_ != NULL) {
    onSet_(v);
  }
  setProperty("pwm").send(value);
  return true;
}

bool PinToneNode::_from_mqtt(const String &value) {
  Homie.getLogger() << getId() << "._from_mqtt(" << value << ")" << endl;
  int v = toInt(value, 0, 20000);
  set(v);
  if (onSet_ != NULL) {
    onSet_(v);
  }
  setProperty("freq").send(value);
  return true;
}
