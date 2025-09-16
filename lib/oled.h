#ifndef OLED_H
#define OLED_H


#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET      -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_SDA        21 
#define OLED_SCL        22 

namespace Oled{
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    bool init()
    {
        Wire.setPins(21, 22);
        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
            Serial.println(F("SSD1306 allocation failed"));
            return false;
        }
        
        display.clearDisplay();
        display.display();

        return true;
    }

    void sleepDisplay(Adafruit_SSD1306* display) {
        display->ssd1306_command(SSD1306_DISPLAYOFF);
    }

    void wakeDisplay(Adafruit_SSD1306* display) {
        display->ssd1306_command(SSD1306_DISPLAYON);
    }

    void drawText(int x, int y, const __FlashStringHelper *text)
    {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(x,y);
        display.println(text);
        display.display();
    }
    
    void drawText(int x, int y, String text)
    {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(x,y);
        display.println(text);
        display.display();
    }

    void drawBigText(int x, int y, const __FlashStringHelper *text)
    {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(x,y);
        display.println(text);
        display.display();
    }

    void drawBigText(int x, int y, String text)
    {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(x,y);
        display.println(text);
        display.display();
    }

    void clear()
    {
        display.clearDisplay();
        display.display();
    }


}



#endif