// Copyright 2019 Marc-Antoine Ruel. All rights reserved.
// Use of this source code is governed under the Apache License, Version 2.0
// that can be found in the LICENSE file.

// See https://homieiot.github.io/specification/spec-core-develop/ for the
// MQTT convention.

#include <Homie.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                     \
  TypeName(const TypeName &) = delete;                                         \
  void operator=(const TypeName &) = delete

int isBool(const String &v);
int toInt(const String &v, int min, int max);
String urlencode(const String& src);

// Wrapper for an input pin without real debouncing.
//
// It samples the GPIO at every update (which should be called inside loop())
// and that's it.
//
// If idle is true, idles on PULLUP, if false, assumes a pull down. This is
// useful to not cause a "blip" on pins that default to pull high on boot.
class PinInRaw {
public:
  explicit PinInRaw(int pin, bool idle) : pin(pin), idle_(idle) {
    if (idle) {
      // Assert not D0.
      pinMode(pin, INPUT_PULLUP);
    } else {
      // GPIO16 is a bit one-off.
      if (pin == D0) {
        pinMode(pin, INPUT_PULLDOWN_16);
      } else {
        pinMode(pin, INPUT);
      }
    }
    last_ = raw_get();
  }

  // Returns the logical value.
  bool get() {
    return last_;
  }

  bool update() {
    bool cur = raw_get();
    if (cur != last_) {
      last_ = cur;
      return true;
    }
    return false;
  }

  const int pin;

private:
  bool raw_get() {
    return digitalRead(pin) != idle_;
  }

  const bool idle_;
  bool last_;

  DISALLOW_COPY_AND_ASSIGN(PinInRaw);
};

// Wrapper for a debounced input pin.
//
// It samples the GPIO at every update (which should be called inside loop())
// and wait for at least 25ms before reacting.
//
// If idle is true, the values are reversed. This is useful to not cause a
// "blip" on pins that default to pull high on boot.
class PinInDebounced {
public:
  explicit PinInDebounced(int pin, bool idle) : pin(pin), idle_(idle) {
    debouncer_.interval(25);
    if (idle) {
      debouncer_.attach(pin, INPUT_PULLUP);
    } else {
      debouncer_.attach(pin, INPUT);
    }
  }

  // Returns the logical value.
  bool get() {
    return debouncer_.read() != idle_;
  }

  bool update() {
    return debouncer_.update();
  }

  const int pin;

private:
  Bounce debouncer_;
  const bool idle_;

  DISALLOW_COPY_AND_ASSIGN(PinInDebounced);
};

// Wrapper for an output pin.
//
// If idle is true, the values are reversed. This is useful to not cause a
// "blip" on pins that default to pull high on boot.
class PinOut {
public:
  explicit PinOut(int pin, bool idle) : pin(pin), idle_(idle) {
    pinMode(pin, OUTPUT);
    set(false);
  }

  // Sets the logical value.
  void set(bool l) {
    // Enabling this here is very noisy, but useful when deep down into
    // debugging. Hence it's commented out by default but feel free to
    // temporarily enable it if you are having a hard time.
    //Homie.getLogger() << pin << ".set(" << l << ")" << endl;
    digitalWrite(pin, l != idle_ ? HIGH : LOW);
    value_ = l;
  }

  // Returns the logical value.
  bool get() { return value_; }

  const int pin;

private:
  bool value_;
  const bool idle_;

  DISALLOW_COPY_AND_ASSIGN(PinOut);
};

// Wrapper for a PWM output pin.
class PinPWM {
public:
  explicit PinPWM(int pin) : pin(pin) {
    pinMode(pin, OUTPUT);
    set(0);
  }

  int set(int v);
  int get() { return value_; }

  const int pin;

private:
  int value_;

  DISALLOW_COPY_AND_ASSIGN(PinPWM);
};

// Wrapper for a PWM pin meant to be used as a buzzer using the tone() function.
class PinTone {
public:
  explicit PinTone(int pin) : pin(pin) {
    pinMode(pin, OUTPUT);
    set(0, -1);
  }

  // Use -1 for duration for infinite duration.
  int set(int freq, int duration);
  int get() { return freq_; }

  const int pin;

private:
  int freq_;

  DISALLOW_COPY_AND_ASSIGN(PinTone);
};

//
// Homie nodes.
//

// Homie node representing an input pin. It is read only.
//
// onSet is called with true being the non-idle value. So if idle is true, the
// value sent to onSet() are reversed.
//
// Uses a debouncer with a 25ms delay.
class PinInNode : public HomieNode {
public:
  explicit PinInNode(const char *name, void (*onSet)(bool v), int pin,
                     bool idle)
      : HomieNode(name, name, "input"), onSet_(onSet), pin_(pin, idle) {
    advertise("on");
    // datatype = "boolean"
  }

  void init() {
    broadcast();
  }

  // Returns the logical value of the pin.
  bool get() {
    return pin_.get();
  }

  // Must be called at every loop.
  bool update() {
    if (!pin_.update()) {
      return false;
    }
    broadcast();
    return true;
  }

private:
  void broadcast() {
    bool level = pin_.get();
    const char* value = level ? "true" : "false";
    Homie.getLogger() << getId() << ".broadcast(" << value << ")" << endl;
    setProperty("on").send(value);
    onSet_(level);
  }

  void (*const onSet_)(bool v);
  PinInRaw pin_;

  DISALLOW_COPY_AND_ASSIGN(PinInNode);
};

// Homie node representing an output pin.
//
// If idle is true, acts in reverse. This is important for pins that are pull
// high, thus default to high upon boot which lasts ~600ms. This is most of the
// pins.
class PinOutNode : public HomieNode {
public:
  explicit PinOutNode(const char *name, int pin, bool idle,
                      void (*onSet)(bool v))
      : HomieNode(name, name, "output"), onSet_(onSet), pin_(pin, idle) {
    advertise("on").settable(
        [&](const HomieRange &range, const String &value) {
          return _from_mqtt(value);
        });
    // datatype = "boolean"
  }

  void init() {
    setProperty("on").send("false");
  }

  // Overiddes the value and broadcast it.
  void set(bool level) {
    pin_.set(level);
    const char* value = level ? "true" : "false";
    Homie.getLogger() << getId() << ".set(" << value << ")" << endl;
    setProperty("on").send(value);
  }

  bool get() {
    return pin_.get();
  }

private:
  bool _from_mqtt(const String &value);

  void (*const onSet_)(bool v);
  PinOut pin_;

  DISALLOW_COPY_AND_ASSIGN(PinOutNode);
};

// Homie node representing a PWM output.
//
// For most pins idle should be true since most pins have a pull up.
class PinPWMNode : public HomieNode {
public:
  explicit PinPWMNode(const char *name, int pin,
                      void (*onSet)(int v))
      : HomieNode(name, name, "pwm"), onSet_(onSet), pin_(pin) {
    advertise("pwm").settable(
        [&](const HomieRange &range, const String &value) {
          return _from_mqtt(value);
        });
    // datatype = "integer"
    // format = 0:PWMRANGE
    // or
    // datatype = "float"
    // format = 0:100
    // unit: %
  }

  void init() {
    setProperty("pwm").send("0");
  }

  void set(int level) {
    String value(pin_.set(level));
    Homie.getLogger() << getId() << ".set(" << value << ")" << endl;
    setProperty("pwm").send(value);
  }

  int get() {
    return pin_.get();
  }

private:
  bool _from_mqtt(const String &value);

  void (*const onSet_)(int v);
  PinPWM pin_;

  DISALLOW_COPY_AND_ASSIGN(PinPWMNode);
};

// Homie node representing a buzzer output.
class PinToneNode : public HomieNode {
public:
  explicit PinToneNode(const char *name, int pin,
                      void (*onSet)(int v))
      : HomieNode(name, name, "freq"), onSet_(onSet), pin_(pin) {
    advertise("freq").settable(
        [&](const HomieRange &range, const String &value) {
          return _from_mqtt(value);
        });
    // datatype = "integer"
    // format = 0:20000
    // unit = Hz
  }

  void init() {
    setProperty("freq").send("0");
  }

  void set(int freq) {
    String value(pin_.set(freq, -1));
    Homie.getLogger() << getId() << ".set(" << value << ")" << endl;
    setProperty("freq").send(value);
  }

  int get() {
    return pin_.get();
  }

private:
  bool _from_mqtt(const String &value);

  void (*const onSet_)(int v);
  PinTone pin_;

  DISALLOW_COPY_AND_ASSIGN(PinToneNode);
};
