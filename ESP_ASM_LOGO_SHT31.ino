/*
  Example for TFT_eSPI library

  Created by Bodmer 31/12/16

  This example draws all fonts (as used by the Adafruit_GFX library) onto the
  screen. These fonts are called the GFX Free Fonts (GFXFF) in this library.

  The fonts are referenced by a short name, see the Free_Fonts.h file
  attached to this sketch.

  Other True Type fonts could be converted using the utility within the
  "fontconvert" folder inside the library. This converted has also been
  copied from the Adafruit_GFX library.

  Since these fonts are a recent addition Adafruit do not have a tutorial
  available yet on how to use the fontconvert utility.   Linux users will
  no doubt figure it out!  In the meantime there are 48 font files to use
  in sizes from 9 point to 24 point, and in normal, bold, and italic or
  oblique styles.

  This example sketch uses both the print class and drawString() functions
  to plot text to the screen.

  Make sure LOAD_GFXFF is defined in the User_Setup.h file within the
  library folder.

  --------------------------- NOTE ----------------------------------------
  The free font encoding format does not lend itself easily to plotting
  the background without flicker. For values that changes on screen it is
  better to use Fonts 1- 8 which are encoded specifically for rapid
  drawing with background.
  -------------------------------------------------------------------------

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  ######       TO SELECT YOUR DISPLAY TYPE AND ENABLE FONTS          ######
  #########################################################################
*/

#include <BlynkSimpleEsp8266.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "SPI.h"
#include "TFT_eSPI.h"

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();
bool sht31_present = false;

BlynkTimer timer;

void setup(void) {

  tft.begin();

  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);            // Clear screen

  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    sht31_present = false;
  } else
  {
    sht31_present = true;
  }

  //starfield_init();
  pong_setup();
  //  timer.setInterval(2000L, rtc_DS1307_print);
  timer.setInterval(4000L, SHT31_ASM_logo);

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

    tft.setTextSize(4);
    //    tft.drawChar(127, 150, 7, 2);

    tft.setTextSize(2);
    tft.drawString(old_temp_string, 30, 10, 1);
    tft.drawString(old_humid_string, 30, 55, 1);

    // new data
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    tft.setTextSize(4);
    tft.drawChar(127, 145, 3, 2);

    tft.setTextSize(2);
    temp_string =  String(temp_c, 2);
    humid_string =  String(humid, 2);
    tft.drawString(temp_string, 30, 10, 1);
    tft.drawString("C", 170, 10, 1);
    tft.drawString(humid_string, 30, 55, 1);
    tft.drawString("%", 150, 55, 1);

    old_temp_string = temp_string;
    old_humid_string = humid_string;

  }


  // ASM LOGO
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FF20);                 // Select the font
  tft.drawString("ASM", 30, 120, GFXFF);// Print the string name of the font
  tft.setFreeFont(FF18);
  tft.drawString("PACIFIC TECHNOLOGY", 30, 180, GFXFF);
  tft.drawLine(247, 48, 292, 74, TFT_RED);
  tft.drawLine(292, 74, 293, 127, TFT_RED);
  tft.drawLine(293, 127, 248, 152, TFT_RED);
  tft.drawLine(248, 152, 202, 127, TFT_RED);
  tft.drawLine(202, 127, 203, 75, TFT_RED);
  tft.drawLine(203, 75, 247, 48, TFT_RED);
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
