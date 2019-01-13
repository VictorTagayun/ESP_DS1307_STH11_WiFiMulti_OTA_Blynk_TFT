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
#include "TFT_eSPI.h"
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
#define clockPin 16
SHT1x sht1x(dataPin, clockPin);

TFT_eSPI myGLCD = TFT_eSPI();       // Invoke custom library
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

  myGLCD.init();
  myGLCD.setRotation(3); // SD card upside
  myGLCD.fillScreen(TFT_BLACK);
  
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

  timer.setInterval(2000L, rtc_DS1307_print);
  //timer.setInterval(2000L, SHT11_print);

}

void loop() {
  ArduinoOTA.handle();
  timer.run();
  //Blynk.run();
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
  Serial.print(temp_c, DEC);
  Serial.print("C / ");
  Serial.print(temp_f, DEC);
  Serial.print("F. Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
}
