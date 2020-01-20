#include "config.h"

#ifdef CAPABILITIES_DISPLAY
    #include "display.h"
    #include "MAD_ESP32.h"

    Adafruit_SSD1306 display(128, 64);

    void SetupDisplay()
    {
        display.begin(SSD1306_SWITCHCAPVCC, config.ssd1306_i2c_addr);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        LOGLN("LCD init done.");
    }

    void DisplayData(float temperature, float humidity, float pressure, float iaq_estimate, uint16_t iaq_accuracy, uint16_t plant_moisture)
    {
        display.clearDisplay();
        display.setCursor(0, 6);
        display.print("Temperature: " + String(temperature));
        display.setCursor(0, 22);
        display.print("Humidity: " + String(humidity));
        display.setCursor(0, 38);
        display.print("Pressure: " + String(pressure / 100)); // hPa
        display.setCursor(0, 54);
        display.print("IAQ: " + String(iaq_estimate) + " / " + String(iaq_accuracy));

        display.display();
        delay(1);
    }
#endif
