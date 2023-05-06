#include "esphome.h"

class UartReadLineSensor: public Component, public UARTDevice {
  public:
    // "<TX_MSG,0,%lu,%.1f,%.1f,%.2f>", currTime, id(bme280_humidity).state, id(bme280_temperature).state, id(bme280_pressure).state
  Sensor * header_message_sensor = new Sensor();
  Sensor * alarm_status_code_sensor = new Sensor();
  Sensor * current_time_sensor = new Sensor();
  Sensor * humidity_sensor = new Sensor();
  Sensor * temperature_sensor = new Sensor();
  Sensor * pressure_sensor = new Sensor();
  UartReadLineSensor(UARTComponent * parent): UARTDevice(parent) {}

  void setup() override {
    // nothing to do here
  }

  int readline(int readch, char * buffer, int len) {
    static int pos = 0;
    int rpos;

    if (readch > 0) {
      switch (readch) {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0; // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len - 1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
      }
    }
    // No end of line has been found, so return -1.
    return -1;
  }

  void loop() override {
    const int max_line_length = 80;
    static char buffer[max_line_length];
    char * token;

    while (available()) {
      if (readline(read(), buffer, max_line_length) > 0) {
        // "<TX_MSG,0,%lu,%.1f,%.1f,%.2f>", currTime, id(bme280_humidity).state, id(bme280_temperature).state, id(bme280_pressure).state
        Serial.println("UART received string");
        Serial.println(buffer);
        for (char * dst = buffer, * src = buffer + 1;* dst = * src; src++, dst++);
        int length = strlen(buffer);
        buffer[length - 1] = '\0';
        const char * delimiter = ",";
        token = strtok(buffer, delimiter);

        char * values[10];
        int index = 0;
        while (token != NULL) {
          values[index] = token;
          index++;
          token = strtok(NULL, delimiter);
        }

        //header_message_sensor -> publish_state("");
        alarm_status_code_sensor -> publish_state(atof(values[1]));
        current_time_sensor -> publish_state(atof(values[2]));
        humidity_sensor -> publish_state(atof(values[3]));
        temperature_sensor -> publish_state(atof(values[4]));
        pressure_sensor -> publish_state(atof(values[5]));
      }
    }
  }
};