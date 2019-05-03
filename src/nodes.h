// Copyright 2019 Marc-Antoine Ruel. All rights reserved.
// Use of this source code is governed under the Apache License, Version 2.0
// that can be found in the LICENSE file.

#include <Homie.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                     \
  TypeName(const TypeName &) = delete;                                         \
  void operator=(const TypeName &) = delete

int isBool(const String &v);
int toInt(const String &v, int min, int max);

// Wrapper for an output pin.
class PinOut {
public:
  explicit PinOut(int pin, bool level) : pin(pin) {
    pinMode(pin, OUTPUT);
    set(level);
  }

  void set(bool l) {
    digitalWrite(pin, l ? HIGH : LOW);
    value_ = l;
  }

  bool get() { return value_; }

  const int pin;

protected:
  bool value_;

private:
  DISALLOW_COPY_AND_ASSIGN(PinOut);
};

// Wrapper for a PWM output pin.
class PinPWM {
public:
  explicit PinPWM(int pin, int level = 0) : pin(pin) {
    pinMode(pin, OUTPUT);
    set(level);
  }

  int set(int v) {
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

  int get() { return value_; }

  const int pin;

protected:
  int value_;

private:
  DISALLOW_COPY_AND_ASSIGN(PinPWM);
};

// Wrapper for a PWM pin meant to be used as a buzzer using the tone() function.
class PinTone {
public:
  explicit PinTone(int pin, int freq = 0) : pin(pin) {
    pinMode(pin, OUTPUT);
    set(freq);
  }

  int set(int freq, int duration = -1) {
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

  int get() { return freq_; }

  const int pin;

protected:
  int freq_;

private:
  DISALLOW_COPY_AND_ASSIGN(PinTone);
};

//
// Homie nodes.
//

// Homie node representing an input pin.
//
// Uses a debouncer with a 50ms delay.
class PinInNode : public HomieNode {
public:
  explicit PinInNode(const char *name, void (*onSet)(bool v), int pin,
                     int mode = INPUT_PULLUP, int interval = 50)
      : HomieNode(name, "input"), onSet(onSet) {
    debouncer.attach(pin, mode);
    debouncer.interval(interval);
    advertise("on");
    setProperty("on").send("false");
  }

  void update() {
    debouncer.update();
    if (debouncer.rose()) {
      setProperty("on").send("true");
      onSet(true);
    } else if (debouncer.fell()) {
      setProperty("on").send("false");
      onSet(false);
    }
  }

protected:
  void (*const onSet)(bool v);
  Bounce debouncer;

private:
  DISALLOW_COPY_AND_ASSIGN(PinInNode);
};

// Homie node representing an output pin.
class PinOutNode : public HomieNode {
public:
  explicit PinOutNode(const char *name, int pin, bool level,
                      void (*onSet)(bool v) = NULL)
      : HomieNode(name, "output"), onSet(onSet), pin_(pin, level) {
    advertise("on").settable([&](const HomieRange &range, const String &value) {
      return this->_onPropSet(value);
    });
    set(level);
  }

  void set(bool level) {
    pin_.set(level);
    setProperty("on").send(level ? "true" : "false");
  }

  bool get() {
    return pin_.get();
  }

protected:
  bool _onPropSet(const String &value) {
    int v = isBool(value);
    if (v == -1) {
      Homie.getLogger() << getId() << ": Bad value: " << value << endl;
      return false;
    }
    set(v);
    if (onSet != NULL) {
      onSet(v);
    }
    return true;
  }

  void (*const onSet)(bool v);
  PinOut pin_;

private:
  DISALLOW_COPY_AND_ASSIGN(PinOutNode);
};

// Homie node representing a PWM output.
class PinPWMNode : public HomieNode {
public:
  explicit PinPWMNode(const char *name, int pin, int level = 0,
                      void (*onSet)(int v) = NULL)
      : HomieNode(name, "pwm"), onSet(onSet), pin_(pin, level) {
    advertise("pwm").settable(
        [&](const HomieRange &range, const String &value) {
          return this->_onPropSet(value);
        });
    set(level);
  }

  void set(int level) {
    setProperty("pwm").send(String(pin_.set(level)));
  }

  int get() {
    return pin_.get();
  }

protected:
  bool _onPropSet(const String &value) {
    int v = toInt(value, 0, PWMRANGE);
    set(v);
    if (onSet != NULL) {
      onSet(v);
    }
    return true;
  }

  void (*const onSet)(int v);
  PinPWM pin_;

private:
  DISALLOW_COPY_AND_ASSIGN(PinPWMNode);
};

// Homie node representing a buzzer output.
class PinToneNode : public HomieNode {
public:
  explicit PinToneNode(const char *name, int pin, int freq = 0,
                      void (*onSet)(int v) = NULL)
      : HomieNode(name, "freq"), onSet(onSet), pin_(pin, freq) {
    advertise("freq").settable(
        [&](const HomieRange &range, const String &value) {
          return this->_onPropSet(value);
        });
    set(freq);
  }

  void set(int freq) {
    setProperty("freq").send(String(pin_.set(freq)));
  }

  int get() {
    return pin_.get();
  }

protected:
  bool _onPropSet(const String &value) {
    int v = toInt(value, 0, 20000);
    set(v);
    if (onSet != NULL) {
      onSet(v);
    }
    return true;
  }

  void (*const onSet)(int v);
  PinTone pin_;

private:
  DISALLOW_COPY_AND_ASSIGN(PinToneNode);
};
