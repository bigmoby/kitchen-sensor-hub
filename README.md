# kitchen-sensor-hub
A simple ESP32 (Esphome) and ESP8266 (Arduino) project hub for Miflora messages reply, 1.28 TFT SPI 240*240 display, BME280 sensor, MQ2 sensor and RCWL-0516 microwave motion sensor.

For the 3D project please have a look at `3D` directory.

# Schematics

Esp8266 display driver: 
 
| ESP8266 (Wemos D1 Mini) | 1.28 TFT SPI 240*240 |
|-------------------------|----------------------|
| 3V3                     | LED                  |
| D5                      | SCK                  |
| D7                      | SDA                  |
| D3                      | A0/DC                |
| D4                      | RESET                |
| D2                      | CS                   |
| GND                     | GND                  |
| 3V3                     | VCC                  |
 

| ESP8266 (Wemos D1 Mini) | ESP32 (Wemos D1 Mini) |
|-------------------------|-----------------------|
| RX                      | 17                    |
| TX                      | 16                    |


| ESP32 (Wemos D1 Mini) | BME280 |
|-----------------------|--------|
| 3V3                   | VIN    |
| GND                   | GND    |
| 19                    | SCL    |
| 18                    | SDA    |


| ESP32 (Wemos D1 Mini) | RCWL-0516 |
|-----------------------|-----------|
| GND                   | GND       |
| IO33                  | DOUT      |
| 5V                    | VCC       |


| ESP32 (Wemos D1 Mini) | MQ2 |
|-----------------------|-----|
| IO32                  | A0  |
| GND                   | GND |
| 5V                    | VCC |