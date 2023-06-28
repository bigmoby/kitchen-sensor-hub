#include <TimeLib.h>
#include <TFT_eSPI.h>  // Include the graphics library (this includes the sprite functions)
#include "Free_Fonts.h"
#include <TJpg_Decoder.h>
#include "number.h"
#include "img/temperature.h"
#include "img/humidity.h"
#include "img/pangzi/i0.h"
#include "img/pangzi/i1.h"
#include "img/pangzi/i2.h"
#include "img/pangzi/i3.h"
#include "img/pangzi/i4.h"
#include "img/pangzi/i5.h"
#include "img/pangzi/i6.h"
#include "img/pangzi/i7.h"
#include "img/pangzi/i8.h"
#include "img/pangzi/i9.h"
#include "font/ZdyLwFont_20.h"

Number dig;

TFT_eSPI tft = TFT_eSPI();               // Create object "tft"
TFT_eSprite orario = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
                                         // the pointer is used by pushSprite() to push it onto the TFT
TFT_eSprite data = TFT_eSprite(&tft);

TFT_eSprite umidita = TFT_eSprite(&tft);
TFT_eSprite temperatura = TFT_eSprite(&tft);
TFT_eSprite pressione = TFT_eSprite(&tft);

TFT_eSprite message_banner = TFT_eSprite(&tft);

#define BITS_PER_PIXEL 8  // How many bits per pixel in Sprite
#define TFT_BETTER_ORANGE 0xFB80
#define TFT_DARK_YELLOW 0xFC00
#define IWIDTH 240
#define IHEIGHT 30

boolean newData = true;

// Definizione delle variabili per il parsing
char message[50];
int alarm;
long timestamp;
int humidity;
float temperature;
float pressure;

int hours = 0;
int minutes = 0;
int seconds = 0;

// Dimensione del buffer di input seriale
const int bufferSize = 100;

// Buffer per la lettura seriale
char buffer[bufferSize];
int bufferIndex = 0;

static int backgroundColor = TFT_BLACK;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}

void digitalClockDisplay();

// =========================================================================
void setup(void) {
  Serial.begin(9600);

  Serial.println();
  Serial.println("Created by Bigmoby");
  Serial.println("Enter data in this format <TX_MSG,0,1671289668,66.9,15.4,991.45>");
  Serial.println();

  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(true);
  tft.fillScreen(backgroundColor);

  // Create the date sprite
  data.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  data.createSprite(145, 15);          // then create the sprite

  // Create the clock sprite
  orario.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  orario.createSprite(122, 40);          // then create the sprite

  // Create the message_banner sprite
  message_banner.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  message_banner.createSprite(IWIDTH, IHEIGHT);  // then create the sprite

  // Create the umidità sprite
  umidita.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  umidita.createSprite(60, 20);           // then create the sprite

  // Create the temperatura sprite
  temperatura.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  temperatura.createSprite(80, 20);           // then create the sprite

  // Create the pressione sprite
  pressione.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  pressione.createSprite(120, 20);          // then create the sprite

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  TJpgDec.drawJpg(10, 155, humidity_img, sizeof(humidity_img));
  TJpgDec.drawJpg(35, 177, temperature_img, sizeof(temperature_img));

  digitalClockDisplay();
}

// ====================== Animation ==================================
int Anim = 0;
int AprevTime = 0;
void imgAnim() {
  int x = 160, y = 160;
  if (millis() - AprevTime > 37) {
    Anim++;
    AprevTime = millis();
  }
  if (Anim == 10)
    Anim = 0;

  switch (Anim) {
    case 0:
      TJpgDec.drawJpg(x, y, i0, sizeof(i0));
      break;
    case 1:
      TJpgDec.drawJpg(x, y, i1, sizeof(i1));
      break;
    case 2:
      TJpgDec.drawJpg(x, y, i2, sizeof(i2));
      break;
    case 3:
      TJpgDec.drawJpg(x, y, i3, sizeof(i3));
      break;
    case 4:
      TJpgDec.drawJpg(x, y, i4, sizeof(i4));
      break;
    case 5:
      TJpgDec.drawJpg(x, y, i5, sizeof(i5));
      break;
    case 6:
      TJpgDec.drawJpg(x, y, i6, sizeof(i6));
      break;
    case 7:
      TJpgDec.drawJpg(x, y, i7, sizeof(i7));
      break;
    case 8:
      TJpgDec.drawJpg(x, y, i8, sizeof(i8));
      break;
    case 9:
      TJpgDec.drawJpg(x, y, i9, sizeof(i9));
      break;
    default:
      Serial.println("Error anim");
      break;
  }
}

// ==========================
void loop() {
  static bool alarmActive = false;
  //imgAnim();

  if (Serial.available()) {
    // Leggi la stringa dalla seriale
    Serial.println("Reading from serial");
    char c = Serial.read();

    // Se il carattere è '>', effettua il parsing della stringa
    if (c == '>') {
      Serial.println("Read from serial");
      buffer[bufferIndex] = '\0';  // Termina il buffer con il carattere nullo
      parseString(buffer);

      // Stampa i valori ottenuti dal parsing
      Serial.print("message: ");
      Serial.println(message);
      drawBanner(&message_banner, message, 50, 130, TFT_RED);

      Serial.print("alarm: ");
      Serial.println(alarm);

      if (alarm == 1) {
        alarmActive = true;
        backgroundColor = (backgroundColor == TFT_BLACK) ? TFT_RED : TFT_BLACK;
      } else if (alarm == 0) {
        alarmActive = false;
        backgroundColor = TFT_BLACK;
      }

      // Converte il timestamp in una data leggibile
      tmElements_t time;
      breakTime(timestamp, time);

      char timestampString[20];
      sprintf(timestampString, "%04d-%02d-%02d %02d:%02d:%02d",
              time.Year + 1970, time.Month, time.Day,
              time.Hour, time.Minute, time.Second);

      // Suddivide la stringa timestampString in data e orario
      char* dateToken = strtok(timestampString, " ");
      char* timeToken = strtok(NULL, " ");

      // Suddivide la stringa timeToken in ore, minuti e secondi
      char* hourToken = strtok(timeToken, ":");
      char* minuteToken = strtok(NULL, ":");
      char* secondToken = strtok(NULL, ":");

      hours = atoi(hourToken);
      minutes = atoi(minuteToken);
      seconds = atoi(secondToken);

      // Array contenente i nomi dei mesi
      const char* monthNames[] = { "Gen", "Feb", "Mar", "Apr", "Mag", "Giu", "Lug", "Ago", "Set", "Ott", "Nov", "Dic" };

      // Converte il dateToken in un numero di giorno della settimana (0 = Domenica, 1 = Lunedì, ecc.)
      int year, month, day;
      sscanf(dateToken, "%04d-%02d-%02d", &year, &month, &day);
      int dayOfWeek = calculateDayOfWeek(year, month, day);

      // Tronca il nome del mese ai primi tre caratteri
      char simplifiedMonth[4];
      strncpy(simplifiedMonth, monthNames[month - 1], 3);
      simplifiedMonth[3] = '\0';

      // Mappa il numero del giorno della settimana al nome del giorno
      const char* dayNames[] = { "Domenica", "Lunedì", "Martedì", "Mercoledì", "Giovedì", "Venerdì", "Sabato" };
      const char* dayOfWeekName = dayNames[dayOfWeek];

      // Tronca il giorno della settimana ai primi tre caratteri
      char simplifiedDayOfWeek[4];
      strncpy(simplifiedDayOfWeek, dayOfWeekName, 3);
      simplifiedDayOfWeek[3] = '\0';

      // Crea una nuova variabile simplifiedTime con ore e minuti
      char simplifiedTime[10];
      sprintf(simplifiedTime, "%s:%s", hourToken, minuteToken);

      Serial.print("date: ");
      Serial.println(dateToken);
      Serial.print("month: ");
      Serial.println(simplifiedMonth);
      Serial.print("day of week: ");
      Serial.println(simplifiedDayOfWeek);
      Serial.print("simplifiedTime: ");
      Serial.println(simplifiedTime);
      Serial.print("humidity: ");
      Serial.println(humidity);
      Serial.print("temperature: ");
      Serial.println(temperature);
      Serial.print("pressure: ");
      Serial.println(pressure);

      drawBanner(&data, simplifiedDayOfWeek + String(" ") + String(day) + String(" ") + simplifiedMonth, 60, 30, TFT_LIGHTGREY);
      //drawBanner(&orario, simplifiedTime, 60, 52, TFT_BETTER_ORANGE);
      digitalClockDisplay();

      drawBanner(&umidita, String(humidity) + String("%"), 35, 158, TFT_LIGHTGREY);
      drawBanner(&temperatura, String(temperature) + "℃", 59, 180, TFT_LIGHTGREY);
      drawBanner(&pressione, String(pressure) + String(" hPa"), 65, 202, TFT_LIGHTGREY);

      delay(3000);
    } else {
      // Aggiungi il carattere al buffer
      buffer[bufferIndex] = c;
      bufferIndex = (bufferIndex + 1) % bufferSize;
    }
  }
}

// =========================================================================
// Create sprite, plot graphics in it, plot to screen, then delete sprite
// =========================================================================
void drawBanner(TFT_eSprite* sprite, String text, int x, int y, int color) {
  if (text) {
    sprite->setColorDepth(8);
    sprite->loadFont(ZdyLwFont_20);
    sprite->fillSprite(backgroundColor);
    sprite->setTextWrap(false);
    sprite->setTextColor(color, backgroundColor);
    sprite->setTextDatum(TL_DATUM);

    sprite->drawString(text, 0, 0, GFXFF);
    // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
    // Specify what colour is to be treated as transparent (black in this example)
    sprite->pushSprite(x, y, TFT_TRANSPARENT);
    //  sprite->deleteSprite();

    sprite->unloadFont();
  }
}
// =========================================================================

// Funzione per calcolare il giorno della settimana
int calculateDayOfWeek(int year, int month, int day) {
  if (month < 3) {
    month += 12;
    year--;
  }

  int century = year / 100;
  year %= 100;

  int dayOfWeek = (day + ((13 * (month + 1)) / 5) + year + (year / 4) + (century / 4) - (2 * century)) % 7;

  if (dayOfWeek < 0) {
    dayOfWeek += 7;
  }

  return dayOfWeek;
}

// Funzione per il parsing della stringa
void parseString(const char* input) {

  // Copia la stringa di input nel buffer droppando il primo carattere
  strcpy(buffer, &input[2]);
  bufferIndex = 0;

  // Effettua il parsing dei valori
  char* token = strtok(buffer, ",");
  if (token != NULL) {
    strcpy(message, token);
    token = strtok(NULL, ",");
  }
  if (token != NULL) {
    alarm = atoi(token);
    token = strtok(NULL, ",");
  }
  if (token != NULL) {
    timestamp = atol(token);
    token = strtok(NULL, ",");
  }
  if (token != NULL) {
    humidity = atoi(token);
    token = strtok(NULL, ",");
  }
  if (token != NULL) {
    temperature = round1(atof(token));
    token = strtok(NULL, ",");
  }
  if (token != NULL) {
    pressure = atof(token);
    token = strtok(NULL, ",");
  }
}

float round1(float value) {
  long val = (long)(value * 10L);
  value = (float)val / 10.0;
  return value;
}

// ================= PRINT CLOCK ===========================
unsigned char Hour_sign = 60;
unsigned char Minute_sign = 60;
unsigned char Second_sign = 60;
uint16_t bgColor = 0x0000;
void digitalClockDisplay() {
  int timey = 52;
  if (hours != Hour_sign) {
    dig.printfW3660(20, timey, hours / 10);
    dig.printfW3660(60, timey, hours % 10);
    Hour_sign = hours;
  }
  if (minutes != Minute_sign) {
    dig.printfO3660(101, timey, minutes / 10);
    dig.printfO3660(141, timey, minutes % 10);
    Minute_sign = minutes;
  }
  if (seconds != Second_sign) {
    dig.printfW1830(182, timey + 30, seconds / 10);
    dig.printfW1830(202, timey + 30, seconds % 10);
    Second_sign = seconds;
  }
}
