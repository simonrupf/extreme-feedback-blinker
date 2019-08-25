#include <Arduino.h>
#include <ESP8266WiFi.h>
 
#include <DNSServer.h>        // Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> // Local WebServer to serve the configuration portal
#include <WiFiManager.h>      // Local AP with configuration portal for credentials

#include <Adafruit_NeoPixel.h>

#define LED_NUM 3
#define LED_PIN 14
#define ANIMATION_DELAY 750
#define ANIMATION_MINIMUM 28

Adafruit_NeoPixel led(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

ESP8266WebServer server(80);

// blue until initialized
String color = "0000ff";
uint8_t red   = 0;
uint8_t green = 0;
uint8_t blue  = 255;
uint8_t display_red   = 0;
uint8_t display_green = 0;
uint8_t display_blue  = 255;

bool animationDirection[LED_NUM];
uint8_t animationBrightness[LED_NUM];
uint16_t animationCounter = 0;

const char type[] PROGMEM = {"text/html"};
const char page[] PROGMEM = {
  "<!DOCTYPE HTML>\n"
  "<html>\n"
  "\t<head>\n"
  "\t\t<meta charset=\"utf-8\"/>\n"
  "\t\t<title>Extreme Feedback Blinker</title>\n"
  "\t\t<style>\n"
  "\t\t\tbody {\n"
  "\t\t\t\tcolor: #fff;\n"
  "\t\t\t\tbackground-color: #333;\n"
  "\t\t\t\tmargin: auto;\n"
  "\t\t\t\twidth: max-content;\n"
  "\t\t\t}\n"
  "\t\t\th1 {\n"
  "\t\t\t\ttext-shadow: 1px 1px 3px #999;\n"
  "\t\t\t}\n"
  "\t\t\t.led:before {\n"
  "\t\t\t\tcontent: \"\\25CF\\00A0\";\n"
  "\t\t\t\tfont-size: 1.2em;\n"
  "\t\t\t}\n"
  "\t\t\tbutton .led:before {\n"
  "\t\t\t\ttext-shadow: 0 0 2px #666;\n"
  "\t\t\t}\n"
  "\t\t\t#state {\n"
  "\t\t\t\tcolor: #XXXXXX;\n"
  "\t\t\t\ttext-shadow: -1px -2px #fff, 3px 5px 7px #000;\n"
  "\t\t\t\tanimation-name: breathe;\n"
  "\t\t\t\tanimation-duration: 3s;\n"
  "\t\t\t\tanimation-iteration-count: infinite;\n"
  "\t\t\t}\n"
  "\t\t\t@keyframes breathe {\n"
  "\t\t\t\t0%   {opacity: 1;}\n"
  "\t\t\t\t50%  {opacity: 0.2;}\n"
  "\t\t\t\t100% {opacity: 1;}\n"
  "\t\t\t}\n"
  "\t\t</style>\n"
  "\t</head>\n"
  "\t<body>\n"
  "\t\t<h1>Extreme Feedback Blinker</h1>\n"
  "\t\t<dl>\n"
  "\t\t\t<dt>Current state:</dt>\n"
  "\t\t\t<dd><span id=\"state\" class=\"led\"></span> <a href=\"./\"><button style=\"float:right\">refresh</button></a></dd>\n"
  "\t\t</dl>\n"
  "\t\t<p>Change state:</p>\n"
  "\t\t<p>\n"
  "\t\t\t<a href=\"?color=ff0000\"><button><span class=\"led\" style=\"color:#f00;\"></span> red alert</button></a>\n"
  "\t\t\t<a href=\"?color=ffff00\"><button><span class=\"led\" style=\"color:#ff0;\"></span> yellow alert</button></a>\n"
  "\t\t\t<a href=\"?color=0000ff\"><button><span class=\"led\" style=\"color:#00f;\"></span> blue alert</button></a>\n"
  "\t\t\t<a href=\"?color=00ff00\"><button><span class=\"led\" style=\"color:#0f0;\"></span> all clear</button></a>\n"
  "\t\t</p>\n"
  "\t</body>\n"
  "</html>"
};

String getParam = "color";

void handleRoot() {
  Serial.println("request received");
  if (server.hasArg(getParam)) {
    color = server.arg(getParam);
    long colorNumber = (long) strtol(&color[0], NULL, 16);
    red   = colorNumber >> 16;
    green = colorNumber >> 8 & 0xFF;
    blue  = colorNumber & 0xFF;
    Serial.print("color code received: ");
    Serial.println(color);
    Serial.println("color decoded as:");
    Serial.print("- red   ");
    Serial.println(red);
    Serial.print("- green ");
    Serial.println(green);
    Serial.print("- blue  ");
    Serial.println(blue);
  }
  server.sendHeader("Cache-Control", "no-cache, max-age=0, must-revalidate, no-store");
  String content = FPSTR(page);
  content.replace("XXXXXX", color);
  server.send(200, FPSTR(type), content);
}

void handleAnimation() {
  updateColor(red, display_red);
  updateColor(green, display_green);
  updateColor(blue, display_blue);
  for (uint8_t i = 0; i < LED_NUM; ++i) {
    led.setPixelColor(
      i,
      led.Color(
        display_red   / 255.0 * animationBrightness[i],
        display_green / 255.0 * animationBrightness[i],
        display_blue  / 255.0 * animationBrightness[i]
      )
    );
    // handle animation loop
    if (animationDirection[i] && animationBrightness[i] < 255) {
      ++animationBrightness[i];
    } else if (animationBrightness[i] > ANIMATION_MINIMUM) {
      --animationBrightness[i];
    }
    if (animationBrightness[i] == ANIMATION_MINIMUM || animationBrightness[i] == 255) {
      animationDirection[i] = !animationDirection[i];
    }
  }
  led.show();
}

void updateColor(uint8_t & color, uint8_t & display_color) {
  if (color > display_color) {
    ++display_color;
  } else if (color < display_color) {
    --display_color;
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("");
  Serial.println("");

  // Initialize the NeoPixel
  led.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  led.show();             // Turn OFF all pixels ASAP
  led.setBrightness(255); // Set BRIGHTNESS to maximum
  // test pattern red, green, blue
  led.setPixelColor(0, led.Color(255, 0, 0));
  led.setPixelColor(1, led.Color(0, 255, 0));
  led.setPixelColor(2, led.Color(0, 0, 255));
  led.show();

  // Initialize animation arrays, so all pixels start at a different part of the cycle
  for (uint8_t i = 0; i < LED_NUM; ++i) {
    animationDirection[i]  = (i == LED_NUM - 1);
    animationBrightness[i] = ceil(255 - (255 / (LED_NUM - 1) * i));
    if (animationBrightness[i] < ANIMATION_MINIMUM) {
      animationBrightness[i] = ANIMATION_MINIMUM;
    }
  }

  // Connect to WiFi network
  WiFiManager wifiManager;
  wifiManager.autoConnect("Blinker");
  Serial.print("WiFi connected to ");
  Serial.println(WiFi.SSID());

  // Start the server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}
 
void loop() {
  // Check if a client has connected
  server.handleClient();

  // set colors only after a certain delay to avoid flickering
  if (animationCounter >= ANIMATION_DELAY) {
    handleAnimation();
    animationCounter = 0;
  } else {
    ++animationCounter;
  }
}

