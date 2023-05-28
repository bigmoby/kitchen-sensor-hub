// Receive with start- and end-markers combined with parsing
// It assumes you send something like "<TX_MSG,0,1671289668,66.9,15.4,991.45>\n"
// ordered as TX_MSG = msg type
// 0 = alarm status,
// 1671289668 current timestamp,
// 66.9 = humidity,
// 15.4 = temperature
// 991.45 = pressure
#include <TimeLib.h>    // https://github.com/PaulStoffregen/Time
#include <Timezone.h>   // https://github.com/JChristensen/Timezone
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

#define TFT_DC 7
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_CLK 9
#define TFT_RESET 8

#define LILLA 0xFC0E
#define SEAGREEN 0x2C4A
#define SEASHELL 0xFFBD
#define PURPLE 0x4810

// Hardware SPI on Feather or other boards
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RESET);
GFXcanvas1 canvas_time(150, 36);
GFXcanvas1 canvas_date(130, 18);

const byte numChars = 40;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = {0};
unsigned long time_stamp = 1;
char formatted_date[10] = {0};
char formatted_time[8] = {0};
unsigned short allarme = 0;
float humidity = -1.0;
float temperature = -273.0;
float pressure = -1.0;

boolean newData = true;

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};   // Central European Standard Time
Timezone myTZ(CEST, CET);

//============

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(GC9A01A_BLACK);
  canvas_time.setTextWrap(false);
  canvas_date.setTextWrap(false);
  Serial.print(char(169));
  Serial.println("Created by Bigmoby");
  Serial.println("Enter data in this format <TX_MSG,0,1671289668,66.9,15.4,991.45>");
  Serial.println();

  delay(2);
  Serial.print(F("Screen fill"));
  delay(500);
}

void testFillScreen() {
  tft.fillScreen(GC9A01A_BLACK);
  yield();
  tft.fillScreen(GC9A01A_RED);
  yield();
}

//============

void loop() {
  recvWithStartEndMarkers();
  if (newData == true) {
    Serial.print(F("New data arrived"));
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0

    Serial.println(F("Received data from serial: "));
    Serial.println(tempChars);

    parseData();
    showParsedData();
    printDisplay();
    newData = false;
  }
}

//============

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {

    Serial.println(F("Receiving from serial"));

    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;

        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//============

void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  allarme = atoi(strtokIndx);

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  time_stamp = atol(strtokIndx);
  time_t localTime;
  localTime = myTZ.toLocal((time_t) time_stamp);
  sprintf(formatted_date, "%02d.%02d.%02d", day(localTime), month(localTime), year(localTime));
  sprintf(formatted_time, "%02d:%02d", hour(localTime), minute(localTime), second(localTime));

  Serial.println(F("formatted_date"));
  Serial.println(formatted_date);

  Serial.println(F("formatted_time"));
  Serial.println(formatted_time);

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  humidity = atof(strtokIndx);

  Serial.println(F("humidity"));
  Serial.println(humidity);

  strtokIndx = strtok(NULL, ",");
  temperature = atof(strtokIndx);

  Serial.println(F("temperature"));
  Serial.println(temperature);

  strtokIndx = strtok(NULL, ",");
  pressure = atof(strtokIndx);

  Serial.println(F("pressure"));
  Serial.println(pressure);
}

unsigned long printDisplay() {
  unsigned long start = micros();

  //tft.fillScreen(GC9A01A_BLACK);

  if (allarme == 1)
  {
    // tft.setTextSize(1);
    //tft.setFont(&FreeSans12pt7b);
    //tft.fillScreen(PURPLE);
    tft.setCursor(65, 190);
    tft.setTextColor(SEASHELL);
    tft.print("ALLARME");
    tft.setCursor(95, 220);
    tft.setTextColor(SEASHELL);
    tft.print("GAS");
  }
  else
  {

  }

  if (true)
  {
    //tft.setFont(&FreeSans12pt7b);
    tft.setCursor(60, 30);
    tft.setTextColor(LILLA);
    tft.println(formatted_date);

    Serial.println(F("Clear canvas (not display) prima"));

    canvas_time.fillScreen(GC9A01A_BLACK);    // Clear canvas (not display)

    Serial.println(F("Clear canvas (not display) dopo"));

    canvas_time.setCursor(0, 0);
    //canvas_time.setTextSize(5);
    canvas_time.print(formatted_time);

    Serial.println(F("(canvas_time.height())"));
    Serial.println(canvas_time.height());

    //drawBitmap(short, short, unsigned char*, short, short, unsigned short, unsigned short)
   // tft.drawBitmap(45, 60, canvas_time.getBuffer(), canvas_time.width(), canvas_time.height(), LILLA, GC9A01A_BLACK);

    Serial.println(F("(tft.drawBitmap) dopo"));
  }

  //tft.setTextSize(1);
  //tft.setFont(&FreeSans12pt7b);
  tft.setCursor(80, 120);
  tft.setTextColor(LILLA);
  tft.print(temperature); tft.print(" °C");

  tft.setCursor(80, 140);
  tft.setTextColor(LILLA);
  tft.print(humidity); tft.print(" %");

  tft.setCursor(60, 160);
  tft.setTextColor(LILLA);
  tft.print(pressure); tft.print(" hPa");

  return micros() - start;
}

//============

void showParsedData() {
  Serial.print("Message ");
  Serial.println(messageFromPC);
  Serial.print("Alarm status ");
  Serial.println(allarme);
  Serial.print("Formatted date ");
  Serial.println(formatted_date);
  Serial.print("humidity ");
  Serial.println(humidity);
  Serial.print("temperature ");
  Serial.println(temperature);
  Serial.print("pressure ");
  Serial.println(pressure);
}
