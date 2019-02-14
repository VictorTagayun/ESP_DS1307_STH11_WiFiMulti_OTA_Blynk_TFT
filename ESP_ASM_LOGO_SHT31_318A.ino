#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "SPI.h"

#include "TFT_eSPI.h"
// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

#include "ThingSpeak.h"
WiFiClient  client;
unsigned long myChannelNumber = 165640;
const char * myWriteAPIKey = "7E4TQ5X8UH66SPDO";

#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();
bool sht31_present = false;

#include <BlynkSimpleEsp8266.h>
BlynkTimer timer;

// check https://lowvoltage.github.io/2017/07/09/Onboard-LEDs-NodeMCU-Got-Two
int LED_module = 2;  // near the antenna
int LED_mainbrd = 16; // main board LED = LED_BUILTIN

void setup(void) {

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);            // Clear screen

  Serial.begin(115200);

  pinMode(LED_module, OUTPUT);
  pinMode(LED_mainbrd, OUTPUT);
  digitalWrite(LED_module, LOW); // HIGH = LED is OFF, LOW  = LED is ON
  digitalWrite(LED_mainbrd, LOW); // HIGH = LED is OFF, LOW  = LED is ON

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("SingaporePolice", "123Qweasd");
  wifiMulti.addAP("SingaporeMilitia", "123Qweasd");
  wifiMulti.addAP("SingaporeMilitia_plus", "123Qweasd");
  wifiMulti.addAP("SingaporeMilitia_5GHz", "123Qweasd");

  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.SSID());
    Serial.println(WiFi.localIP());
    digitalWrite(LED_module, HIGH); // HIGH = LED is OFF, LOW  = LED is ON
  }

  ThingSpeak.begin(client);  // Initialize ThingSpeak

  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    sht31_present = false;
  } else
  {
    sht31_present = true;
    digitalWrite(LED_mainbrd, HIGH); // HIGH = LED is OFF, LOW  = LED is ON
  }

  //starfield_init();
  //pong_setup();
  //  timer.setInterval(2000L, rtc_DS1307_print);
  timer.setInterval(2000L, SHT31_ASM_logo);


  pong_setup();

}

void loop() {
  timer.run();
  pong_loop();
}


float old_temp_c;
float old_temp_f;
float old_humid;

void SHT31_print() {

  float temp_c = sht31.readTemperature();
  float humid = sht31.readHumidity();
  float temp_f;
  bool data_valid = false;

  if (! isnan(temp_c)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.println(temp_c);
    data_valid = true;
    temp_f = (temp_c * 1.8) + 32;
  } else {
    Serial.println("Failed to read temperature");
    data_valid = false;
  }

  if (! isnan(humid)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(humid);
    data_valid = true;
  } else {
    Serial.println("Failed to read humidity");
    data_valid = false;
  }
  Serial.println();

  if (data_valid)
  {
    // old data
    // tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_BLACK);
    tft.setFreeFont(FSB18);       // Select Free Serif 12 point font
    tft.setCursor(0, 0);    // Set cursor near top left corner of screen

    tft.println();                // Move cursor down a line
    tft.println(); //tft.println("Temperature: ");
    tft.print("  ");
    tft.print(old_temp_c);
    tft.print(" C / ");
    tft.print(old_temp_f);
    tft.println(" F");
    tft.println("");
    tft.setCursor(0, 140);    // Set cursor near top left corner of screen
    tft.println(); // tft.println("Humidity: ");
    tft.print("  ");
    tft.print(old_humid);
    tft.println(" %");

    // New data
    // tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.setFreeFont(FSB18);       // Select Free Serif 12 point font
    tft.setCursor(0, 0);    // Set cursor near top left corner of screen

    tft.println();                // Move cursor down a line
    tft.println("  Temperature: ");
    tft.print("  ");
    tft.print(temp_c);
    tft.print(" C / ");
    tft.print(temp_f);
    tft.println(" F");
    tft.println("");
    tft.setCursor(0, 140);    // Set cursor near top left corner of screen
    tft.println("  Humidity: ");
    tft.print("  ");
    tft.print(humid);
    tft.println(" %");

    old_temp_c = temp_c;
    old_temp_f = temp_f;
    old_humid = humid;
  }
}

String temp_string, old_temp_string;
String humid_string, old_humid_string;
int thingspeak_datalog;

void SHT31_ASM_logo () {

  float temp_c = sht31.readTemperature();
  float humid = sht31.readHumidity();
  bool data_valid = false;

  if (! isnan(temp_c)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.println(temp_c);
    data_valid = true;
  } else {
    Serial.println("Failed to read temperature");
    data_valid = false;
  }

  if (! isnan(humid)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(humid);
    data_valid = true;
  } else {
    Serial.println("Failed to read humidity");
    data_valid = false;
  }
  Serial.println();

  if (data_valid)
  {
    // old data
    tft.setTextColor(TFT_BLACK, TFT_BLACK);

    tft.setTextSize(2);
    tft.drawString(old_temp_string, 25, 10, 1);
    tft.drawString(old_humid_string, 25, 55, 1);

    // new data
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    tft.setTextSize(4);
    tft.drawChar(127, 140, 3, 2);

    tft.setTextSize(2);
    temp_string =  String(temp_c, 2);
    humid_string =  String(humid, 2);
    tft.drawString(temp_string, 25, 10, 1);
    tft.drawString("C", 163, 10, 1);
    tft.drawString(humid_string, 25, 55, 1);
    tft.drawString("%", 145, 55, 1);

    old_temp_string = temp_string;
    old_humid_string = humid_string;

    // datalog to thingspeak
    thingspeak_datalog++;
    if (thingspeak_datalog == 60)
    {
      // set the fields with the values
      ThingSpeak.setField(1, temp_c);
      ThingSpeak.setField(2, humid);

      // write to the ThingSpeak channel
      int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      if (x == 200) {
        Serial.println("Channel update successful.");
      }
      else {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }

      thingspeak_datalog = 0;
    }
  }

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FF20);                 // Select the font
  tft.drawString("ASM", 25, 125, GFXFF);// Print the string name of the font
  tft.setFreeFont(FF18);
  tft.drawString("PACIFIC TECHNOLOGY", 25, 180, GFXFF);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("by Victor T.", 100, 210, 1);

  // ASM LOGO

  // outside top right "\"
  //tft.drawLine(246, 50, 291, 76, TFT_RED);
  tft.drawLine(247, 59, 292, 85, TFT_RED);
  tft.drawLine(248, 58, 293, 84, TFT_RED); // center,
  tft.drawLine(249, 57, 294, 83, TFT_RED);
  //tft.drawLine(250, 46, 295, 72, TFT_RED);

  // outside right "|"
  //tft.drawLine(291, 74, 291, 126, TFT_RED);
  tft.drawLine(292, 84, 292, 136, TFT_RED);
  tft.drawLine(293, 84, 293, 136, TFT_RED); // center
  tft.drawLine(294, 84, 294, 136, TFT_RED);
  //tft.drawLine(295, 74, 295, 126, TFT_RED);

  // outside bottom right "/"
  //tft.drawLine(291, 124, 246, 150, TFT_RED);
  tft.drawLine(292, 135, 247, 161, TFT_RED);
  tft.drawLine(293, 136, 248, 162, TFT_RED); // center
  tft.drawLine(294, 137, 249, 163, TFT_RED);
  //tft.drawLine(295, 128, 250, 154, TFT_RED);

  // outside bottom left "\"
  //tft.drawLine(246, 154, 201, 128, TFT_RED);
  tft.drawLine(247, 163, 202, 137, TFT_RED);
  tft.drawLine(248, 162, 203, 136, TFT_RED); // center
  tft.drawLine(249, 161, 204, 135, TFT_RED);
  //tft.drawLine(250, 150, 205, 124, TFT_RED);

  // outside left "|"
  //tft.drawLine(201, 126, 201, 74, TFT_RED);
  tft.drawLine(202, 136, 202, 84, TFT_RED);
  tft.drawLine(203, 136, 203, 84, TFT_RED); // center
  tft.drawLine(204, 136, 204, 84, TFT_RED);
  //tft.drawLine(205, 126, 205, 74, TFT_RED);

  // outside top left "/"
  //tft.drawLine(201, 72, 246, 46, TFT_RED);
  tft.drawLine(202, 83, 247, 57, TFT_RED);
  tft.drawLine(203, 84, 248, 58, TFT_RED); // center
  tft.drawLine(204, 85, 249, 59, TFT_RED);
  //tft.drawLine(205, 76, 250, 50, TFT_RED);

  tft.drawLine(202, 85, 247, 111, TFT_RED);
  tft.drawLine(203, 84, 248, 110, TFT_RED); // center
  tft.drawLine(204, 83, 249, 109, TFT_RED);

  tft.drawLine(292, 83, 247, 109, TFT_RED);
  tft.drawLine(293, 84, 248, 110, TFT_RED); // center
  tft.drawLine(294, 85, 249, 111, TFT_RED);

  tft.drawLine(247, 162, 247, 110, TFT_RED);
  tft.drawLine(248, 162, 248, 110, TFT_RED); // center
  tft.drawLine(249, 162, 249, 110, TFT_RED);

  // erase the corners
  tft.fillCircle(248, 58, 8, TFT_BLACK);
  tft.fillCircle(293, 84, 8, TFT_BLACK);
  tft.fillCircle(293, 136, 8, TFT_BLACK);
  tft.fillCircle(248, 162, 8, TFT_BLACK);
  tft.fillCircle(203, 136, 8, TFT_BLACK);
  tft.fillCircle(203, 84, 8, TFT_BLACK);
  tft.fillCircle(248, 110, 8, TFT_BLACK);

  // thin triangles
  tft.drawLine(248, 58, 293, 136, TFT_RED);
  tft.drawLine(293, 136, 203, 136, TFT_RED);
  tft.drawLine(203, 136, 248, 58, TFT_RED);
  tft.drawLine(248, 58, 248, 110, TFT_RED);
  tft.drawLine(203, 136, 248, 110, TFT_RED);
  tft.drawLine(293, 136, 248, 110, TFT_RED);

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

  //midline();

  ball();
}

void pong_initgame() {
  lpaddle_y = random(0, h - paddle_h);
  rpaddle_y = random(0, h - paddle_h);

  // ball is placed on the center of the left paddle
  ball_y = lpaddle_y + (paddle_h / 2);

  calc_target_y();

  //midline();

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
