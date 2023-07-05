#include <String.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <time.h>
#include <Timezone.h>
#include <TimeLib.h>

#include <TFT_eSPI.h>
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

#include "settings.h"

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and no offset
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
NTPClient timeClient(ntpUDP, NTP_SERVER, GTM0, NTP_UPDATE_INTERVAL);

// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = { "CEST", Last, Sun, Mar, 2, 120 };  // Central European Summer Time
TimeChangeRule CET = { "CET ", Last, Sun, Oct, 3, 60 };    // Central European Standard Time
Timezone CE(CEST, CET);

// ==========================
/**
 * Input time in epoch format and return tm time format
 * by Renzo Mischianti <www.mischianti.org> 
 */
static tm getDateTimeByParams(long time) {
  struct tm* newtime;
  const time_t tim = time;
  newtime = localtime(&tim);
  return *newtime;
}

/**
 * Input tm time format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org>
 */
static String getDateTimeStringByParams(tm* newtime, char* pattern = (char*)"%d/%m/%Y %H:%M:%S") {
  char buffer[30];
  strftime(buffer, 30, pattern, newtime);
  return buffer;
}

/**
 * Input time in epoch format format and return String with format pattern
 * by Renzo Mischianti <www.mischianti.org> 
 */
static String getEpochStringByParams(long time, char* pattern = (char*)"%d/%m/%Y %H:%M:%S") {
  //    struct tm *newtime;
  tm newtime;
  newtime = getDateTimeByParams(time);
  return getDateTimeStringByParams(&newtime, pattern);
}

Number dig;

TFT_eSPI tft = TFT_eSPI();               // Create object "tft"
TFT_eSprite orario = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
                                         // the pointer is used by pushSprite() to push it onto the TFT
TFT_eSprite data = TFT_eSprite(&tft);

TFT_eSprite umidita_sprite = TFT_eSprite(&tft);
TFT_eSprite temperatura_sprite = TFT_eSprite(&tft);
TFT_eSprite pressione_sprite = TFT_eSprite(&tft);

TFT_eSprite message_banner = TFT_eSprite(&tft);

#define BITS_PER_PIXEL 8  // How many bits per pixel in Sprite
#define TFT_BETTER_ORANGE 0xFB80
#define TFT_DARK_YELLOW 0xFC00
#define IWIDTH 240
#define IHEIGHT 30

String inputString;              // Variabile per memorizzare la stringa letta
boolean stringComplete = false;  // Flag per indicare se la stringa è stata completata

String message;      // Variabile per memorizzare il messaggio
int alarm;           // Variabile per memorizzare il valore dell'allarme
String humidity;     // Variabile per memorizzare l'umidità
String temperature;  // Variabile per memorizzare la temperatura
String pressure;     // Variabile per memorizzare la pressione

int hours = 0;
int minutes = 0;
int seconds = 0;

static int backgroundColor = TFT_BLACK;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}

void digitalClockDisplay();

byte loadNum = 6;
void loading(TFT_eSprite* sprite, String up_message, String bottom_message, int x, int y, byte delayTime) {
  sprite->setColorDepth(8);

  sprite->createSprite(200, 100);
  sprite->fillSprite(0x0000);

  sprite->drawRoundRect(0, 0, 200, 16, 8, 0xFFFF);
  sprite->fillRoundRect(3, 3, loadNum, 10, 5, 0xFFFF);
  sprite->setTextDatum(CC_DATUM);
  sprite->setTextColor(TFT_GREEN, 0x0000);
  sprite->drawString(up_message, 100, 30, 2);
  sprite->setTextColor(TFT_GREEN, 0x0000);
  sprite->drawString(bottom_message, 100, 50, 1);
  sprite->setTextColor(TFT_WHITE, 0x0000);
  sprite->drawRightString("Created by Bigmoby v1.0", 180, 60, 2);
  sprite->pushSprite(20, 110);

  sprite->deleteSprite();
  loadNum += 1;
  delay(delayTime);
}

void createPlainBoard() {
  tft.fillScreen(backgroundColor);

  // Create the date sprite
  data.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  data.createSprite(145, 20);          // then create the sprite

  // Create the clock sprite
  orario.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  orario.createSprite(122, 40);          // then create the sprite

  // Create the message_banner sprite
  message_banner.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  message_banner.createSprite(IWIDTH, IHEIGHT);  // then create the sprite

  // Create the umidità sprite
  umidita_sprite.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  umidita_sprite.createSprite(60, 20);           // then create the sprite

  // Create the temperatura sprite
  temperatura_sprite.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  temperatura_sprite.createSprite(80, 20);           // then create the sprite

  // Create the pressione sprite
  pressione_sprite.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  pressione_sprite.createSprite(120, 30);          // then create the sprite

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  TJpgDec.drawJpg(10, 155, humidity_img, sizeof(humidity_img));
  TJpgDec.drawJpg(35, 177, temperature_img, sizeof(temperature_img));
}

// =========================================================================
void setup(void) {
  Serial.begin(9600);

  while (!Serial) {}

  Serial.println();
  Serial.println("Created by Bigmoby");
  Serial.println("Enter data in this format <NO MSG,0,59.0,26.3,987.97>");
  Serial.println();

  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(true);
  tft.fillScreen(backgroundColor);

  // =============== CONFIGURE WiFi portal ===================
  Serial.print("Connecting to WiFi......");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    loading(&message_banner, "Connecting to WiFi......", "", 50, 130, 70);
  }

  delay(20);
  loading(&message_banner, "Successfully connected!", "", 50, 130, 1);

  Serial.println("Successfully connected!");
  Serial.println(WiFi.SSID());

  timeClient.begin();

  delay(5000);
  while (!timeClient.update()) {
    delay(1000);
  }

  Serial.println("Adjust local clock");
  unsigned long epoch = timeClient.getEpochTime();
  Serial.println(getEpochStringByParams(epoch));
  setTime(epoch);
  Serial.println(getEpochStringByParams(CE.toLocal(now())));

  // ==================================================

  MDNS.begin(OTA_HOST);

  httpUpdater.setup(&httpServer, "/update", OTA_USER, OTA_PASSWORD);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  char host_url[60];
  sprintf(host_url, "http://%s.local/update", OTA_HOST);
  Serial.println("OTA service available: " + String(host_url));
  loading(&message_banner, "OTA service available:", String(host_url), 50, 130, 1);

  delay(10000);

  // ===================================================

  createPlainBoard();
  digitalClockDisplay();
}

void loop() {
  httpServer.handleClient();
  MDNS.update();

  printClock();

  while (Serial.available()) {
    char incomingChar = Serial.read();  // Legge un carattere dalla porta seriale

    if (incomingChar == '<') {  // Controllo se è stato ricevuto il carattere di inizio stringa
      inputString = "";         // Resetta la stringa iniziale
      stringComplete = false;
    } else if (incomingChar == '>') {  // Controllo se è stato ricevuto il carattere di fine stringa
      stringComplete = true;           // Imposta il flag per indicare che la stringa è stata completata
    } else {
      inputString += incomingChar;  // Aggiunge il carattere alla stringa in costruzione
    }
  }

  if (stringComplete) {
    // La stringa è stata completata, puoi analizzare e assegnare i valori alle variabili
    parseString();

    if (alarm == 1) {
      Serial.print("alarm ON with message : ");
      Serial.println(message);
      // Stampa i valori ottenuti dal parsing
      drawBanner(&message_banner, message, 50, 130, TFT_RED);
    } else if (alarm == 0) {
      cleanBanner(&message_banner, 50, 130, backgroundColor);
    }

    // Ora puoi utilizzare le variabili message, alarm, humidity, temperature, pressure come necessario
    Serial.println("Messaggio: " + message);
    Serial.println("Allarme: " + String(alarm));
    Serial.println("Umidità: " + humidity);
    Serial.println("Temperatura: " + temperature);
    Serial.println("Pressione: " + pressure);

    drawBanner(&umidita_sprite, humidity + String("%"), 35, 158, TFT_LIGHTGREY);
    drawBanner(&temperatura_sprite, temperature + "℃", 59, 180, TFT_LIGHTGREY);
    drawBanner(&pressione_sprite, pressure + String(" hPa"), 65, 202, TFT_LIGHTGREY);

    // Resetta la stringa per leggere una nuova stringa
    inputString = "";
    stringComplete = false;
  }

  delay(1000);
}

void printClock() {
  // Converte il timestamp in una data leggibile
  tmElements_t time;
  long timestamp = CE.toLocal(now());
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

  int year, month, day;
  sscanf(dateToken, "%04d-%02d-%02d", &year, &month, &day);

  // Tronca il nome del mese ai primi tre caratteri
  char simplifiedMonth[4];
  strncpy(simplifiedMonth, monthNames[month - 1], 3);
  simplifiedMonth[3] = '\0';

  // Converte il dateToken in un numero di giorno della settimana (0 = Domenica, 1 = Lunedì, ecc.)
  int dayOfWeek = time.Wday;
  // Mappa il numero del giorno della settimana al nome del giorno
  const char* dayNames[] = { "Domenica", "Lunedì", "Martedì", "Mercoledì", "Giovedì", "Venerdì", "Sabato" };
  const char* dayOfWeekName = dayNames[dayOfWeek - 1];

  // Tronca il giorno della settimana ai primi tre caratteri
  char simplifiedDayOfWeek[4];
  strncpy(simplifiedDayOfWeek, dayOfWeekName, 3);
  simplifiedDayOfWeek[3] = '\0';

  // Crea una nuova variabile simplifiedTime con ore e minuti
  char simplifiedTime[10];
  sprintf(simplifiedTime, "%s:%s", hourToken, minuteToken);

  Serial.print("date: ");
  Serial.println(dateToken);
  Serial.print("day of week: ");
  Serial.println(simplifiedDayOfWeek);
  Serial.print("simplifiedTime: ");
  Serial.println(simplifiedTime);

  drawBanner(&data, simplifiedDayOfWeek + String(" ") + String(day) + String(" ") + simplifiedMonth, 70, 15, TFT_LIGHTGREY);

  digitalClockDisplay();
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

    sprite->drawString(text, 0, 0);
    // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
    sprite->pushSprite(x, y, TFT_TRANSPARENT);
    sprite->deleteSprite();

    sprite->unloadFont();
  }
}

void cleanBanner(TFT_eSprite* sprite, int x, int y, int backgroundColor) {
  TFT_eSprite clear_message_banner = TFT_eSprite(&tft);
  clear_message_banner.setColorDepth(BITS_PER_PIXEL);                   // Set colour depth first
  clear_message_banner.createSprite(sprite->width(), sprite->width());  // then create the sprite

  clear_message_banner.setColorDepth(8);
  clear_message_banner.fillRect(x, y, sprite->height(), sprite->width(), backgroundColor);

  // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
  clear_message_banner.pushSprite(x, y, TFT_TRANSPARENT);
  clear_message_banner.deleteSprite();
}
// =========================================================================

// Funzione per il parsing della stringa <NO MSG,0,59.0,26.3,987.97>
void parseString() {
  Serial.print("Trying to parse: ");
  Serial.println(inputString);
  inputString.trim();
  inputString.replace("\n", "");

  int commaIndices[4];   // Array per memorizzare gli indici delle virgole
  int currentIndex = 0;  // Indice corrente nell'array

  for (int i = 0; i < inputString.length(); i++) {
    if (inputString.charAt(i) == ',') {
      commaIndices[currentIndex] = i;  // Memorizza l'indice della virgola
      currentIndex++;                  // Passa all'indice successivo
    }
  }

  // Estrai i valori dalle sottostringhe
  message = inputString.substring(0, commaIndices[0]);
  alarm = inputString.substring(commaIndices[0] + 1, commaIndices[1]).toInt();
  humidity = inputString.substring(commaIndices[1] + 1, commaIndices[2]);
  humidity = inputString.substring(commaIndices[1] + 1, commaIndices[2]);
  temperature = inputString.substring(commaIndices[2] + 1, commaIndices[3]);
  pressure = inputString.substring(commaIndices[3] + 1);
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
