#ifndef DISPLAY_H
    #define DISPLAY_H

    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>

    extern Adafruit_SSD1306 display;

    void SetupDisplay();
    void DisplayData(float temperature, float humidity, float pressure, float iaq_estimate, uint16_t iaq_accuracy, uint16_t plant_moisture);
#endif
