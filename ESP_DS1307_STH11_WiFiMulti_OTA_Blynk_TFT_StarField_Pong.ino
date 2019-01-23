#define BLYNK_PRINT Serial /* Comment this out to disable prints and save space */
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include "RTClib.h"
#include <WidgetRTC.h>
#include "SPI.h"
#include "TFT_eSPI.h" // https://github.com/Bodmer/TFT_eSPI
#include "Free_Fonts.h" // Include the header file attached to this sketch
// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();
#define TFT_GREY 0x7BEF
#include <SHT1x.h> // use https://github.com/practicalarduino/SHT1x
// shaped like a U, numbering starts from top left of the IC https://www.sparkfun.com/datasheets/Sensors/SHT1x_datasheet.pdf
// pin 1 NC
// pin 2 GND
// pin 3 Data
// pin 4 CLK
// pin 5 VCC

// Specify data and clock connections and instantiate SHT1x object, cannot mix with DS1307, DS1307 hang-up
#define dataPin  10
#define clockPin 0
SHT1x sht1x(dataPin, clockPin);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
RTC_DS1307 rtc_DS1307;
WidgetRTC rtc_Blynk;
ESP8266WiFiMulti wifiMulti;
bool RTC_DS1307_found, STH11_found, RTC_DS1307_running, first_run = true;
DateTime RTC_DS1307_now = rtc_DS1307.now();
BlynkTimer timer;
//char auth[] = "bc08f366b13147d6ab50d36f25a73438";

// check https://lowvoltage.github.io/2017/07/09/Onboard-LEDs-NodeMCU-Got-Two
int LED1 = 2;  // near the antenna
int LED2 = 16; // main board LED = LED_BUILTIN

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc_Blynk.begin();
}

void setup() {

  Serial.begin(115200);

  tft.init();
  tft.setRotation(3); // SD card upside
  tft.fillScreen(TFT_BLACK);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW); // LOW  = LED is ON
  digitalWrite(LED2, LOW); // HIGH = LED is OFF

  if (rtc_DS1307.begin()) {
    Serial.println("RTC Found .... ");
    RTC_DS1307_found = true;
    if (! rtc_DS1307.isrunning()) {
      Serial.println("RTC is NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      //rtc.adjust(DateTime(2018, 1, 13, 7, 0, 0));
      RTC_DS1307_running = false;
    }
    //rtc_DS1307.adjust(DateTime(2019, 1, 14, 0, 46, 0)); // adjust time
    digitalWrite(LED1, HIGH); // HIGH = LED is OFF
  } else
  {
    Serial.println("Couldn't find RTC");
    RTC_DS1307_found = false;
  }

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("SingaporePolice", "123Qweasd");
  wifiMulti.addAP("SingaporeMilitia", "123Qweasd");
  wifiMulti.addAP("SingaporeMilitia_plus", "123Qweasd");
  wifiMulti.addAP("SingaporeMilitia_5GHz", "123Qweasd");

  Serial.print("Connecting Wifi ");

  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.SSID());
    Serial.println(WiFi.localIP());
    digitalWrite(LED2, HIGH); // HIGH = LED is OFF
  }

  //Blynk.config(auth);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  //starfield_init();
  pong_setup();
  //  timer.setInterval(2000L, rtc_DS1307_print);
  timer.setInterval(10000L, SHT11_print);

}

void loop() {
  ArduinoOTA.handle();
  timer.run();
  //Blynk.run();
  //starfield_loop();
  pong_loop();
}

void rtc_DS1307_print () {

  delay(100);

  RTC_DS1307_now = rtc_DS1307.now();

  Serial.print(RTC_DS1307_now.year(), DEC);
  Serial.print('/');
  Serial.print(RTC_DS1307_now.month(), DEC);
  Serial.print('/');
  Serial.print(RTC_DS1307_now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[RTC_DS1307_now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(RTC_DS1307_now.hour(), DEC);
  Serial.print(':');
  Serial.print(RTC_DS1307_now.minute(), DEC);
  Serial.print(':');
  Serial.print(RTC_DS1307_now.second(), DEC);
  Serial.println();

  delay(100);

  SHT11_print();
}

float old_temp_c;
float old_temp_f;
float old_humidity;

void SHT11_print()
{
  float temp_c;
  float temp_f;
  float humidity;

  // Read values from the sensor
  temp_c = sht1x.readTemperatureC();
  temp_f = sht1x.readTemperatureF();
  humidity = sht1x.readHumidity();

  // Print the values to the serial port
  Serial.print("Temperature: ");
  Serial.print(temp_c);
  Serial.print("C / ");
  Serial.print(temp_f);
  Serial.println("F");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // old data
  // tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FSB18);       // Select Free Serif 12 point font
  tft.setCursor(0, 0);    // Set cursor near top left corner of screen

  tft.println();                // Move cursor down a line
  tft.println(); //tft.println("Temperature: ");
  tft.print(old_temp_c);
  tft.print(" C / ");
  tft.print(old_temp_f);
  tft.println(" F");
  tft.println("");
  tft.setCursor(0, 140);    // Set cursor near top left corner of screen
  tft.println(); // tft.println("Humidity: ");
  tft.print(old_humidity);
  tft.println(" %");

  // New data
  // tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setFreeFont(FSB18);       // Select Free Serif 12 point font
  tft.setCursor(0, 0);    // Set cursor near top left corner of screen

  tft.println();                // Move cursor down a line
  tft.println("Temperature: ");
  tft.print(temp_c);
  tft.print(" C / ");
  tft.print(temp_f);
  tft.println(" F");
  tft.println("");
  tft.setCursor(0, 140);    // Set cursor near top left corner of screen
  tft.println("Humidity: ");
  tft.print(humidity);
  tft.println(" %");

  old_temp_c = temp_c;
  old_temp_f = temp_f;
  old_humidity = humidity;
}


// Star Field, With 1024 stars the update rate is ~65 frames per second
#define NSTARS 1024
uint8_t sx[NSTARS] = {};
uint8_t sy[NSTARS] = {};
uint8_t sz[NSTARS] = {};

uint8_t za, zb, zc, zx;

// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint8_t __attribute__((always_inline)) rng()
{
  zx++;
  za = (za ^ zc ^ zx);
  zb = (zb + za);
  zc = (zc + (zb >> 1)^za);
  return zc;
}

void starfield_init() {
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);

  Serial.begin(115200);
  tft.init();
  //tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // fastSetup() must be used immediately before fastPixel() to prepare screen
  // It must be called after any other graphics drawing function call if fastPixel()
  // is to be called again
  //tft.fastSetup(); // Prepare plot window range for fast pixel plotting
}

void starfield_loop()
{
  //unsigned long t0 = micros();
  uint8_t spawnDepthVariation = 255;

  for (int i = 0; i < NSTARS; ++i)
  {
    if (sz[i] <= 1)
    {
      sx[i] = 160 - 120 + rng();
      sy[i] = rng();
      sz[i] = spawnDepthVariation--;
    }
    else
    {
      int old_screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
      int old_screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;

      // This is a faster pixel drawing function for occassions where many single pixels must be drawn
      tft.drawPixel(old_screen_x, old_screen_y, TFT_BLACK);

      sz[i] -= 2;
      if (sz[i] > 1)
      {
        int screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
        int screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;

        if (screen_x >= 0 && screen_y >= 0 && screen_x < 320 && screen_y < 240)
        {
          uint8_t r, g, b;
          r = g = b = 255 - sz[i];
          tft.drawPixel(screen_x, screen_y, tft.color565(r, g, b));
        }
        else
          sz[i] = 0; // Out of screen, die.
      }
    }
  }
  //unsigned long t1 = micros();
  //static char timeMicros[8] = {};

  // Calcualte frames per second
  //Serial.println(1.0 / ((t1 - t0) / 1000000.0));
}

// PONG
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREY  0x5AEB

int16_t h = 240;
int16_t w = 320;

int dly = 5;

int16_t paddle_h = 30;
int16_t paddle_w = 4;

int16_t lpaddle_x = 0;
int16_t rpaddle_x = w - paddle_w;

int16_t lpaddle_y = 0;
int16_t rpaddle_y = h - paddle_h;

int16_t lpaddle_d = 1;
int16_t rpaddle_d = -1;

int16_t lpaddle_ball_t = w - w / 4;
int16_t rpaddle_ball_t = w / 4;

int16_t target_y = 0;

int16_t ball_x = 2;
int16_t ball_y = 2;
int16_t oldball_x = 2;
int16_t oldball_y = 2;

int16_t ball_dx = 1;
int16_t ball_dy = 1;

int16_t ball_w = 6;
int16_t ball_h = 6;

int16_t dashline_h = 4;
int16_t dashline_w = 2;
int16_t dashline_n = h / dashline_h;
int16_t dashline_x = w / 2 - 1;
int16_t dashline_y = dashline_h / 2;

int16_t lscore = 12;
int16_t rscore = 4;

void pong_setup(void) {

  //randomSeed(analogRead(0)*analogRead(1));

  tft.init();

  tft.fillScreen(BLACK);
  //tft.fillScreen(GREY);

  pong_initgame();

  tft.setTextColor(WHITE, BLACK);

}

void pong_loop() {
  delay(dly);

  lpaddle();
  rpaddle();

  midline();

  ball();
}

void pong_initgame() {
  lpaddle_y = random(0, h - paddle_h);
  rpaddle_y = random(0, h - paddle_h);

  // ball is placed on the center of the left paddle
  ball_y = lpaddle_y + (paddle_h / 2);

  calc_target_y();

  midline();

  //  tft.fillRect(0, h - 26, w, 239, GREY);
  //
  //  tft.setTextDatum(TC_DATUM);
  //  tft.setTextColor(WHITE, GREY);
  //  tft.drawString("TFT_eSPI example", w / 2, h - 26 , 4);
}

void midline() {

  // If the ball is not on the line then don't redraw the line
  if ((ball_x < dashline_x - ball_w) && (ball_x > dashline_x + dashline_w)) return;

  // Quick way to draw a dashed line
  tft.setWindow(dashline_x, 0, dashline_x + dashline_w - 1, h);

  for (int16_t i = 0; i < dashline_n; i += 2) {
    tft.pushColor(WHITE, dashline_w * dashline_h); // push dash pixels
    tft.pushColor(BLACK, dashline_w * dashline_h); // push gap pixels
  }
}

void lpaddle() {

  if (lpaddle_d == 1) {
    tft.fillRect(lpaddle_x, lpaddle_y, paddle_w, 1, BLACK);
  }
  else if (lpaddle_d == -1) {
    tft.fillRect(lpaddle_x, lpaddle_y + paddle_h - 1, paddle_w, 1, BLACK);
  }

  lpaddle_y = lpaddle_y + lpaddle_d;

  if (ball_dx == 1) lpaddle_d = 0;
  else {
    if (lpaddle_y + paddle_h / 2 == target_y) lpaddle_d = 0;
    else if (lpaddle_y + paddle_h / 2 > target_y) lpaddle_d = -1;
    else lpaddle_d = 1;
  }

  if (lpaddle_y + paddle_h >= h && lpaddle_d == 1) lpaddle_d = 0;
  else if (lpaddle_y <= 0 && lpaddle_d == -1) lpaddle_d = 0;

  tft.fillRect(lpaddle_x, lpaddle_y, paddle_w, paddle_h, WHITE);
}

void rpaddle() {

  if (rpaddle_d == 1) {
    tft.fillRect(rpaddle_x, rpaddle_y, paddle_w, 1, BLACK);
  }
  else if (rpaddle_d == -1) {
    tft.fillRect(rpaddle_x, rpaddle_y + paddle_h - 1, paddle_w, 1, BLACK);
  }

  rpaddle_y = rpaddle_y + rpaddle_d;

  if (ball_dx == -1) rpaddle_d = 0;
  else {
    if (rpaddle_y + paddle_h / 2 == target_y) rpaddle_d = 0;
    else if (rpaddle_y + paddle_h / 2 > target_y) rpaddle_d = -1;
    else rpaddle_d = 1;
  }

  if (rpaddle_y + paddle_h >= h && rpaddle_d == 1) rpaddle_d = 0;
  else if (rpaddle_y <= 0 && rpaddle_d == -1) rpaddle_d = 0;

  tft.fillRect(rpaddle_x, rpaddle_y, paddle_w, paddle_h, WHITE);
}

void calc_target_y() {
  int16_t target_x;
  int16_t reflections;
  int16_t y;

  if (ball_dx == 1) {
    target_x = w - ball_w;
  }
  else {
    target_x = -1 * (w - ball_w);
  }

  y = abs(target_x * (ball_dy / ball_dx) + ball_y);

  reflections = floor(y / h);

  if (reflections % 2 == 0) {
    target_y = y % h;
  }
  else {
    target_y = h - (y % h);
  }
}

void ball() {
  ball_x = ball_x + ball_dx;
  ball_y = ball_y + ball_dy;

  if (ball_dx == -1 && ball_x == paddle_w && ball_y + ball_h >= lpaddle_y && ball_y <= lpaddle_y + paddle_h) {
    ball_dx = ball_dx * -1;
    dly = random(5); // change speed of ball after paddle contact
    calc_target_y();
  } else if (ball_dx == 1 && ball_x + ball_w == w - paddle_w && ball_y + ball_h >= rpaddle_y && ball_y <= rpaddle_y + paddle_h) {
    ball_dx = ball_dx * -1;
    dly = random(5); // change speed of ball after paddle contact
    calc_target_y();
  } else if ((ball_dx == 1 && ball_x >= w) || (ball_dx == -1 && ball_x + ball_w < 0)) {
    dly = 5;
  }

  if (ball_y > h - ball_w || ball_y < 0) {
    ball_dy = ball_dy * -1;
    ball_y += ball_dy; // Keep in bounds
  }

  //tft.fillRect(oldball_x, oldball_y, ball_w, ball_h, BLACK);
  tft.drawRect(oldball_x, oldball_y, ball_w, ball_h, BLACK); // Less TFT refresh aliasing than line above for large balls
  tft.fillRect(   ball_x,    ball_y, ball_w, ball_h, WHITE);
  oldball_x = ball_x;
  oldball_y = ball_y;
}
