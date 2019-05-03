# A 40$ wifi enabled race car 🚗

| Idealized | Actual |
| ------------------------ | --------------------- |
| ![Bolide logo](https://github.com/maruel/bolide-esp8266/raw/master/resources/logo.png) | ![Bolide actual](https://github.com/maruel/bolide-esp8266/raw/master/resources/reallife.jpg) |

*No* internet connectivity is needed, this isn't
[InternetOfShit](https://twitter.com/internetofshit).


## Design

An ESP8266 connects to a MQTT server and waits for commands. It also runs a web
server serving a MQTT client web UI ([try it
online!](https://maruel.ca/bolide-esp8266/?host=127.0.0.1&port=9001&device=bolide))
that you can install as an App on your iOS or Android phone or tablet.

`bolide-esp8266` uses the [Homie MQTT
convention](https://github.com/homieiot/convention/blob/develop/convention.md)
so you can control it via any MQTT enabled automation service like
[Home Assistant](https://www.home-assistant.io/), [openHAB](http://openhab.org),
or internet based services like [IFTTT](https://ifttt.com/), if you really feel
the urge to connect stuff to the internet. ¯\\\_(ツ)\_/¯


## Hardware

- [Car kit](https://www.aliexpress.com/wholesale?SearchText=robot+4wd)
  (20$USD) including motors but the ones with an Arduino Uno is unnecessary.
  You may buy one with a power converter and/or batteries and/or batteries
  holders already included.
- [WeMos D1 mini](https://wemoscc.aliexpress.com/store/1331105) (4$USD) or any
  ESP8266 based board. One with a micro USB port is recommended.
- [L293D](https://www.aliexpress.com/wholesale?SearchText=l293d+dip&SortType=price_asc)
  quadruple half-H driver (1.75$USD for 5pcs). Buy more than one, just in case
  you bend a pin.
- [Passive
  buzzer](https://www.aliexpress.com/wholesale?SearchText=passive+buzzer&SortType=price_asc)
  (0.50$USD) because it's more annoying when it beeps and we love annoying toys;
  optional.
- [LEDs](https://www.aliexpress.com/wholesale?SearchText=200pcs+led+box&SortType=price_asc)
  (3$USD for 200pcs) for additional feedback; optional.
- [Resistor
  ~300Ohms](https://www.aliexpress.com/w/wholesale-metal-film.html?SearchText=metal+film&SortType=price_asc&CatId=400503)
  for the LED (0.58$USD for 100pcs; 2.37$USD for 500pcs); optional.
- [400 points
  Breadboard](https://www.aliexpress.com/wholesale?SearchText=breadboard+400&SortType=price_asc)
  (1$USD) or [2x
  SYD-170](https://www.aliexpress.com/wholesale?SearchText=syb-170) (0.38$USD
  each).
- [Breadboard
  wires](https://www.aliexpress.com/wholesale?SearchText=140+breadboard+wires&SortType=price_asc)
  (1.75$USD) or whatever wire you have around.
- [Screen terminal
  block](https://www.aliexpress.com/wholesale?SearchText=Screw+Terminal+Block)
  (0.80USD for 10pcs).
- [Tactile
  button switch](https://www.aliexpress.com/wholesale?SearchText=tactile+button)
  (2$USD for 100pcs, depends on size).
- Computer to run Mosquitto; a Raspberry Pi or your workstation will do just
  fine.


**BOM:** 36$USD, _including shipping_, with a lot of left overs.


### Power ⚡

The system runs at 2 or 3 different voltages:

- The ESP8266 needs 3.3V and will burn if exposed at >=4V.
  - The WeMos D1 Mini already includes a 5V -> 3.3V DC-DC converter, which
    simplifies our life. If you use an ESP8266 which doesn't include this, you
    have to plan accordingly.
- The L293D needs 4.5V~7V for its internal logic to operate.
- The motors need 5V~12V. *Higher voltage means a faster race car*.

There's three options for motor voltage:

1. Power the motors at 5V. It will move *slowly* but you can upgrade later.
2. Use a [DC-DC step down power converter
   (>=1A)](https://www.aliexpress.com/wholesale?SearchText=dc-dc) (1$USD; 2$USD
   for 5pcs) and run the motors at high voltage then generate a 5V from it. You
   may want/need to add a capacitor accross the lines to stabilize the voltage.
3. Use two different power sources, one 5V, one >5V. This has the side benefit
   of making voltage for the ESP8266 to be more stable.

There's _many_ options for power sources:

1. Wired [5V~12V
   wallwart](https://www.aliexpress.com/wholesale?SearchText=9v+2a+adapter)
   (3$USD) and [female
   adaptor](https://www.aliexpress.com/wholesale?SearchText=5.5mm+female+dc)
   (0.50$USD).
   - You may have one lying around from an old dead electronic device!
2. AA batteries; 4x 1.5V (Alkaline) gives 6V, 6x 1.2V (Eneloop) gives 7.2V. AAA
   are likely not able to push enough Amps.
3. [5V USB power
   bank](https://www.aliexpress.com/wholesale?SearchText=usb+power+bank&SortType=price_asc)
   (<2$USD). Even the cheapest colored round ones will be fine as the ESP8266
   draws around 100mA. The cheapest ones **won't** be enough for the motor but
   high qualities ones (the ones rated for 2A) will be fine.
4. 2x 3.7V LiPo batteries to give 7.4V:
  - [7.4V LiPo battery
    (>=1000mAh)](https://www.aliexpress.com/wholesale?SearchText=7.4v+battery+mAh&SortType=price_asc)
    (10$USD).
  - [2x Soshine 18650
    >=2600mAh](https://www.aliexpress.com/wholesale?SearchText=soshine+18650+battery)
    (15$USD for 2pcs).
  - [2x Soshine 14500 (AA format;
    >=800mAh)](https://www.aliexpress.com/wholesale?SearchText=Soshine+14500+battery)
    (20$USD for 4pcs) batteries; the main advantage is these fit AA cases.
5. [2x 3.2V LiFePo4](https://www.aliexpress.com/wholesale?SearchText=lifepo4)
   batteries to give 6V:
  - Similar to LiPo batteries, but [not suspectible to
    explosion](https://en.wikipedia.org/wiki/Lithium_iron_phosphate_battery#Safety).
    The 6V level is too high to power the WeMos without burning the
    [RT9013](http://www.richtek.com/assets/product_file/RT9013/DS9013-10.pdf)
    voltage regulator. They are available in both 14500 (AA format) and 18650, I
    recommend branded batteries.
  - I wouldn't be surprised if we see cellphones with LiFePo4 batteries in 2017.
    I said it here first! :)


#### Confused? 😕

Keep it simple and start with a **USB power bank** to power the WeMos and **4x
AA Alkaline batteries** to power the motors.


#### Be aware

- Aliexpress searches are really hit-and-miss, it's possible it just decides
  to show you unrelated items depending on how it feels.
- mAh values are generally **overstated** by the seller.
- Shipping LiPo batteries cost more, so it may be simpler to source it locally.
- *You need a charger*.


## Wiring

Use the breadboard and the breadboard wires to connect everything:

[![Wiring schematics](https://github.com/maruel/bolide-esp8266/raw/master/resources/schematics.png)](https://easyeda.com/editor#id=240df0897d66470eb822c16866b1031e)

_Click on the image to edit the schema in EasyEAD_!


### Permanent soldering

Once you confirmed it to work, you may want to solder the L293D to reduce the
overall size:

- Generic
  [Stripboard](https://www.aliexpress.com/wholesale?SearchText=Stripboard+Veroboard)
  (varies) or the [WeMos
  protoboard](https://www.aliexpress.com/store/product/ProtoBoard-Shield-for-WeMos-D1-mini-double-sided-perf-board/1331105_32627711647.html)
  (1$USD).
  - The WeMos one is interesting because it is small and fits perfectly the D1
    mini.
- [Adjustable soldering
  Iron](https://www.aliexpress.com/wholesale?SearchText=soldering+iron+adjustable)
  (10$USD).
- [Multicore 63/37 26AWG 227g
  spool](https://www.digikey.com/product-detail/en/multicore/397982/82-131-ND/2498929)
  (21$USD). Trust me, don't cheap out and do not buy one from Aliexpress!
  - ⚠ Lead may reduce your life expectancy but it was well worth it.


## Software Installation

### Install mosquitto

1. On Ubuntu 16.04+ or Debian Jessie, run: `sudo apt install mosquitto`. On
   other OSes get it from https://mosquitto.org/download.
2. Run the following to enable Websocket on port 9001 (adapt for other OSes):

```
cat << 'EOF' | sudo tee /etc/mosquitto/conf.d/ws.conf
listener 1883
listener 9001
protocol websockets
EOF
sudo systemctl restart mosquitto
```

**⚠:** anyone on the local network and see and inject commands.


### Install [PlatformIO IDE](http://platformio.org)

#### GUI

1. Install [Atom](https://atom.io).
2. Open Atom. In _Settings_, _Install_, type _platformio_ in the box and choose
   _platformio-ide_.
3. Click 'Rebuild' on the little box on the very bottom right.

##### CLI

Run `pip install --user --upgrade platformio`


## Flashing

1. Connect an ESP8266 via USB.
2. (optional) Copy `config_sample.json` to `data/homie/config.json` and [edit as
   documented](https://homieiot.github.io/homie-esp8266/docs/2.0.0/configuration/json-configuration-file/):
   - device name and id
   - Wifi SSID and password
   - MQTT server host name, port, user and password
   - This is not required, as you can do it manually via the web UI as explained
     below.
3. Run: `./flash_all.sh` at a bash prompt.

The ESP8266 takes less than a second to boot.


## Manual device configuration

If `data/homie/config.json` settings didn't work out, the blue LED will be solid
blue as the ESP8266 will boot in configuration mode. Configure it manually with
the web UI:

1. Connect to the Wifi `bolide-xxx`. The password is the `xxx` portion of the
   SSID.
2. Open a Web browser and navigate to http://192.168.123.1.
3. Follow the instructions.
   - Configure the wifi to use
   - Configure the MQTT server to use
   - Use a friendly name like "Bolide" and a device name like "bolide".
   - Enable OTA.
4. At this point the LED will go slow blink, rapid blink then will turn off.

See [Homie
documentation](https://homie-esp8266.readme.io/docs/getting-started#section-connecting-to-the-ap-and-configuring-the-device)
for more details.


## Contributing

I gladly accept contributions via GitHub pull requests, as long as the author
has signed the Google Contributor License.

Before we can use your code, you must sign the [Google Individual Contributor
License Agreement](https://cla.developers.google.com/about/google-individual)
(CLA), which you can do online. The CLA is necessary mainly because you own the
copyright to your changes, even after your contribution becomes part of our
codebase, so we need your permission to use and distribute your code. We also
need to be sure of various other things—for instance that you'll tell us if you
know that your code infringes on other people's patents. You don't have to sign
the CLA until after you've submitted your code for review and a member has
approved it, but you must do it before we can put your code into our codebase.
Before you start working on a larger contribution, you should get in touch with
us first through the issue tracker with your idea so that we can help out and
possibly guide you. Coordinating up front makes it much easier to avoid
frustration later on.
