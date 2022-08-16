#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SSD1306_WHITE 1


Adafruit_SSD1306 display(-1);
void setup(){
// initialize with the I2C addr 0x3C
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

// Clear the buffer.
display.clearDisplay();
}
void loop(){
// Display Text
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(SSD1306_WHITE);
display.setCursor(0,0);
display.println("TEMP     -   ");
display.setCursor(0,8);
display.println("HUMIDITY -   ");
display.setCursor(0,16);
display.println("CO2      -   ");
display.setCursor(0,24);
display.println("WEIGHT   -   ");
display.setCursor(76,0);
display.println(100);
display.setCursor(76,8);
display.println(100);
display.setCursor(76,16);
display.println(100);
display.setCursor(76,24);
display.println(100);
display.display();
delay(2000);
}