// Copyright 2016 Marc-Antoine Ruel. All rights reserved.
// Use of this source code is governed under the Apache License, Version 2.0
// that can be found in the LICENSE file.

// This file uses WeMos pin numbers but they are not special, just defines to
// the actual GPIO number.

// Interfere with boot:
// - RST -> buyton
// - D3 (GPIO0) HIGH run, LOW flash via UART
// - TX (GPIO1)
// - D4 (GPIO2) HIGH; LED
// - RX (GPIO3)
// - D8 (GPIO15) LOW (or boot to SDIO)
// - D0 (GPIO16)-> pulse signal to RST to wake up from wifi

// Left:
// - TX (GPIO1)
// - RX (GPIO3)
// - D1 (GPIO5) Motor R Polarity
// - D2 (GPIO4) Motor L Polarity
// - D3 (GPIO0) Motor PWM
// - D4 (GPIO2)
// - GND
// - 5V

// Right:
// - RST button
// - A0 void
// - D0 (GPIO16)
// - D5 (GPIO14) Button
// - D6 (GPIO12) LED
// - D7 (GPIO13) Buzzer
// - D8 (GPIO15)
// - 3v3

#include <Arduino.h>
#include <Homie.h>
#include <ESPAsyncWebServer.h>

#include "nodes.h"

const int MOTOR_PWM = D3;   // GPIO0
const int MOTOR_LEFT = D2;  // GPIO4
const int MOTOR_RIGHT = D1; // GPIO5
const int BUTTON = D5;      // GPIO14
const int LED = D6;         // GPIO12
const int BUZZER = D7;      // GPIO13

// Overall direction of the car.
enum Direction {
  STOP = 0,
  FORWARD = 1,
  LEFT = 2,
  RIGHT = 3,
  BACKWARD = 4,
  NB_DIRECTION = 5,
};

const char *dirToStr(Direction d) {
  switch (d) {
  case STOP:
    return "STOP";
  case FORWARD:
    return "FORWARD";
  case LEFT:
    return "LEFT";
  case RIGHT:
    return "RIGHT";
  case BACKWARD:
    return "BACKWARD";
  default:
    return "<INVALID DIRECTION>";
  }
}

// Node to control the two motors as a whole.
class Motors : public HomieNode {
public:
  Motors(PinPWMNode &pwm, PinOutNode &left, PinOutNode &right)
      : HomieNode("car", "car"), pwm(pwm), left(left), right(right),
        speed(511), dir(STOP) {
    advertise("direction")
        .settable([&](const HomieRange &range, const String &value) {
          if (value == "stop") {
            motor(STOP);
            return true;
          }
          if (value == "forward") {
            motor(FORWARD);
            return true;
          }
          if (value == "left") {
            motor(LEFT);
            return true;
          }
          if (value == "right") {
            motor(RIGHT);
            return true;
          }
          if (value == "backward") {
            motor(BACKWARD);
            return true;
          }
          Homie.getLogger() << getId() << ": Bad value: " << value << endl;
          return true;
        });
    setProperty("direction").send("stop");
  }

  void motor(Direction d) {
    Homie.getLogger() << dirToStr(d) << endl;
    dir = d;
    switch (d) {
    case STOP:
      set(0, false, false);
      break;
    case FORWARD:
      set(speed, true, true);
      break;
    case LEFT:
      set(speed, false, true);
      break;
    case RIGHT:
      set(speed, true, false);
      break;
    case BACKWARD:
      set(speed, false, false);
      break;
    case NB_DIRECTION:
    default:
      break;
    }
  }

  void toggle() { motor(Direction((dir + 1) % NB_DIRECTION)); }

  void set(int v_pwm, bool v_left, bool v_right) {
    if (v_pwm != 0) {
      speed = v_pwm;
    }
    pwm.set(v_pwm);
    left.set(v_left);
    right.set(v_right);
  }

  PinPWMNode &pwm;
  PinOutNode &left;
  PinOutNode &right;
  int speed;
  Direction dir;
};

// Homie nodes accessible through MQTT.
PinPWMNode Speed("speed", MOTOR_PWM);
PinOutNode Left("left", MOTOR_LEFT);
PinOutNode Right("right", MOTOR_RIGHT);
Motors carNode(Speed, Left, Right);
PinInNode buttonNode("button",
                     [](bool v) {
                       if (v) {
                         carNode.toggle();
                       }
                     },
                     BUTTON);
PinOutNode ledNode("led", LED);
PinToneNode buzzerNode("buzzer", BUZZER);

// Web server to serve the MQTT web UI. This is NOT the web server when in
// configuration mode.
AsyncWebServer httpSrv(80);

void setup() {
  Serial.begin(115200);
  // Increase debug output to maximum level:
  //Serial.setDebugOutput(true);
  // Remove all debug output:
  // Homie.enableLogging(false);
  // Homie.disableLedFeedback(); -> use LED as button.

  // Holding this button for 10s will forcibly reset the device.
  Homie.setResetTrigger(BUTTON, LOW, 10000);
  Homie_setFirmware("bolide", "1.0.0");
  Homie_setBrand("bolide");
  Serial.println();
  Homie.setup();

  if (Homie.isConfigured()) {
    httpSrv.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String url("/index.html?device=");
        const HomieInternals::ConfigStruct& cfg = Homie.getConfiguration();
        // TODO(maruel): Escaping!!
        url += cfg.deviceId;
        url += "&host=";
        url += cfg.mqtt.server.host;
        // TODO(maruel): The websocket port number != cfg.mqtt.server.port.
        url += "&port=9001";
        //cfg.mqtt.username;
        //cfg.mqtt.password;
        request->redirect(url);
    });
    httpSrv.serveStatic("/", SPIFFS, "/html/").setCacheControl("public; max-age=600");
    httpSrv.begin();
  }
}

void loop() {
  Homie.loop();
  buttonNode.update();
}
