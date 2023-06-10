#include <TimeLib.h>
#include <TFT_eSPI.h>  // Include the graphics library (this includes the sprite functions)
#include "NotoSansBold36.h"

TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

TFT_eSprite orario = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
                                         // the pointer is used by pushSprite() to push it onto the TFT

#define BITS_PER_PIXEL 16  // How many bits per pixel in Sprite

boolean newData = true;

// Definizione delle variabili per il parsing
char message[50];
int alarm;
long timestamp;
float humidity;
float temperature;
float pressure;

// Dimensione del buffer di input seriale
const int bufferSize = 100;

// Buffer per la lettura seriale
char buffer[bufferSize];
int bufferIndex = 0;

// Funzione per il parsing della stringa
void parseString(const char* input) {
  // Ignora il primo carattere "<"
  input++;

  // Copia la stringa di input nel buffer
  strcpy(buffer, input);
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
    humidity = atof(token);
    token = strtok(NULL, ",");
  }
  if (token != NULL) {
    temperature = atof(token);
    token = strtok(NULL, ",");
  }
  if (token != NULL) {
    pressure = atof(token);
    token = strtok(NULL, ",");
  }
}

// =========================================================================
void setup(void) {
  Serial.begin(9600);

  Serial.println();
  Serial.println("Created by Bigmoby");
  Serial.println("Enter data in this format <TX_MSG,0,1671289668,66.9,15.4,991.45>");
  Serial.println();

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_NAVY);

  // Create the clock sprite
  orario.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  orario.createSprite(90, 40);           // then create the sprite

  // Only 1 font used in the sprite, so can remain loaded
  orario.loadFont(NotoSansBold36);
}
// =========================================================================
void loop() {

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
      Serial.print("alarm: ");
      Serial.println(alarm);

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

      drawBanner(&orario, simplifiedTime, 40, 50, TFT_ORANGE);

      delay(2000);
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

  // Create an 1 bit (2 colour) sprite 70x80 pixels (uses 70*80/8 = 700 bytes of RAM)
  // Colour depths of 8 bits per pixel and 16 bits are also supported.

  sprite->fillSprite(TFT_NAVY);
  //sprite->fillSprite(TFT_TRANSPARENT);
  sprite->setTextColor(TFT_ORANGE);
  sprite->setTextDatum(TL_DATUM);
  // Only 1 font used in the sprite, so can remain loaded
  sprite->loadFont(NotoSansBold36);

  sprite->drawString(text, 0, 0, 4);
  // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
  // Specify what colour is to be treated as transparent (black in this example)
  sprite->pushSprite(x, y, TFT_TRANSPARENT);
  //  sprite->deleteSprite();
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