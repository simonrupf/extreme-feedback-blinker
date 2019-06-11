# Extreme Feedback Blinker

A simple extreme feedback device (XFD) based on an ESP8266 microcontroller and
one or multiple WS2812 Neopixels, providing an HTTP GET interface over WiFi to
set the colors of the LEDs. The device is intended to be integrated into a CI/CD
or monitoring system providing a simple visual representation of the current
status.

## Usage

When first powered up, the WifiManager library will throw up a WLAN with SSID
"Blinker" and no password protection. When connecting to it using a WiFi enabled
device (notebook, smart phone, etc.), you should be redirected to a
configuration website that lets you select your main WLANs SSID and store the
WPA2 password on the microcontroller. Whenever the device is reset or powered up
*and* doesn't find the configured WiFi or fails to connect to it, it will return
to this configuration mode. The LED(s) should display red, blue, green as a test
pattern during this time.

Once connected to configured WLAN, the LEDs should start to shimmer in the
default color. At this point in time you can connect to the device via a
webbrowser to see a page with multiple buttons to change the color of the LEDs.
You can find the IP of the device via your DHCP servers leases.

The color is controlled via a GET parameter called "color" and expects the 24
bit RGB color in hexadecimal notation (like in CSS). Click the example buttons
to see some URLs with the codes for the labeled colors. It should be easy to
integrate such a URL into your CI/CD or monitoring system or you can call it in
your shell scripts using curl or wget.

In a production setup you would want to persist the IP address of the device and
give it a resolvable hostname in your DNS, so that you can replace the device
without changing your CI/CDs or monitoring systems configuration. If possible,
keep the device on a segregated WiFi SSID apart from other users. Set up packet
filter rules to only allow the necessary host to access the devices port 80.

## Prerequisites

- Any ESP8266 module, i.e. NodeMCU or Wemos D1 mini, etc.
- Arduino IDE setup to support your particular module
- one or multiple WS281**2**, WS281**2b** or WS281**3** Neopixels that are 3.3 V
  tolerant (not WS281**1**)
- jumper wires and bread board *or* light soldering skills and equipment

## Wiring

Example wiring of a NodeMCU with 470 Ohm resistor on the WS2812's data line:

![NodeMCU with WS2812 Neopixel](https://raw.githubusercontent.com/simonrupf/extreme-feedback-blinker/master/NodeMCU%20WS2812.png)

For a bare bones ESP-12 two pull-up/down resistors (10 kΩ) and a few capacitors
are needed when using 5 V USB power via a linear voltage regulator:

![ESP-12 with WS2812 Neopixel](https://raw.githubusercontent.com/simonrupf/extreme-feedback-blinker/master/ESP8266%20WS2812.png)

Capacitor ratings are:
- 100 nF (5 V to GND at voltage regulator)
- 2.2 µF (3.3 V to GND at voltage regulator)
- 470 µF (3.3 V to GND at ESP-12)
- 100 nF (3.3 V to GND at ESP-12)

Resistor ratings are:
- 10 kΩ (pull-down from ESP-12 GND to GPIO 15)
- 10 kΩ (pull-up from ESP-12 3.3 V to enable)
- 470 Ω (data line from ESP-12 GND to WS2812 data-in)

## Limitations

- All content is served over HTTP only.
- There is no authentication implemented, anyone with access to that network
  can change the color.
- The microcontroller runs single-threaded and can only handle a single request
  at a time.
- The configuration page used to set the password of your WPA2 secured WLAN is
  served over an unsecured WiFi. The password is transmitted unencrypted.
