// INCLUDE ALL LIBRARIES
#include <DHT.h>
#include <ESP32Servo.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQUnifiedsensor.h>

// INIT ALL PINS AND PARAMS FOR BETTER READABILITY

// DEFINE DHT PARAMETERS AND OBJECT
#define DHT_PIN 18
#define DHT_TYPE DHT11

// calor -> Heat in Spanish
DHT calor(DHT_PIN, DHT_TYPE);

// DEFINE SERVO AND OBJECT AND STATES
#define SERVO_PIN 17
Servo topLid;
#define OPEN_LID 0
#define CLOSE_LID 180

// DEFINE BUZZER PIN
#define BUZZER 16
// DEFINE HTTP and WiFi
WiFiMulti WIFI;
#define REQUEST "https://script.google.com/macros/s/AKfycbxqsNNCTDZDsVbmvNtU8_yGrKtg_2W0dsMwheFnP9rFZ0UFduCthMpk-mRJf_H7FijcLQ/exec?"

// DEFINE WIFI SSID AND PASSWORD
#define WIFI_SSID "ESP32"
#define WIFI_PASS ""

// DEFINE DELAYS
#define smallDelay 1000
#define regDelay 3000
#define longDelay 5000

// DEFINE FORCE SENSOR
#define FS_PIN 32

// DEFINE OLED
#define SSD1306_WHITE 1
Adafruit_SSD1306 display(-1);

// DEFINE user

#define recepient "19r11a04n1@gcet.edu.in"
// DEFINE MQ135 to sens CO2
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 35
#define type "MQ-135"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6
double CO2 = (0);
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);


// ALERT LEVEL
int ALERT_LEVEL=2;
int ALERT_PARAM=3;

void display_OLED(float temp, float humidity, float co2, float weight)
{
    // Display Text
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("TEMP     -   ");
    display.setCursor(0, 8);
    display.println("HUMIDITY -   ");
    display.setCursor(0, 16);
    display.println("CO2      -   ");
    display.setCursor(0, 24);
    display.println("WEIGHT   -   ");
    display.setCursor(76, 0);
    display.println(temp);
    display.setCursor(76, 8);
    display.println(humidity);
    display.setCursor(76, 16);
    display.println(co2);
    display.setCursor(76, 24);
    display.println(weight);
    display.display();
}

void setup()
{
    // put your setup code here, to run once:
    calor.begin();
    topLid.attach(SERVO_PIN);
    Serial.begin(9600);
    Serial.println("PROGRAM INITIATED");
    // Add Access Point
    WIFI.addAP(WIFI_SSID, WIFI_PASS);
    topLid.write(CLOSE_LID);
    pinMode(BUZZER, OUTPUT);
    // initialize with the I2C addr 0x3C
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    // Clear the buffer.
    display.clearDisplay();

    //   MQ135 sensor
    // Set math model to calculate the PPM concentration and the value of constants
    Serial.begin(9600);
    MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
    MQ135.setA(110.47);
    MQ135.setB(-2.862);
    // Configurate the ecuation values to get NH4 concentration
    MQ135.init();
    Serial.print("Calibrating please wait.");
    float calcR0 = 0;
    for (int i = 1; i <= 10; i++)
    {
        MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
        calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
        Serial.print(".");
    }
    MQ135.setR0(calcR0 / 10);
    Serial.println("  done!.");
    if (isinf(calcR0))
    {
        Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply");
        while (1)
            ;
    }
    if (calcR0 == 0)
    {
        Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply");
        while (1)
            ;
    }
    /*****************************  MQ CAlibration **************************/
    MQ135.serialDebug(false);
}

void loop()
{
    // put your main code here, to run repeatedly:
    if (WIFI.run() == WL_CONNECTED)
    {
        HTTPClient http;
        Serial.println("CONNECTED TO WIFI NETWORK");
        float sensTemp = calor.readTemperature();
        float sensMoist = calor.readHumidity();
        float force_sensor = analogRead(FS_PIN);
        MQ135.update();           // Update data, the arduino will be read the voltage on the analog pin
        CO2 = MQ135.readSensor(); // Sensor will read CO2 concentration using the model and a and b values setted before or in the setup
        
        // PRINT DATA TO CONSOLE AND DISPLAY ON OLED DISPLAY
        Serial.print("Force: ");
        Serial.println(force_sensor);
        Serial.print("Temperature: ");
        Serial.println(sensTemp);
        Serial.print("Moisture: ");
        Serial.println(sensMoist);
        Serial.print("CO2: ");
        Serial.println(CO2);
        display_OLED(sensTemp, sensMoist, CO2, force_sensor);

        if (CO2 > 8 || sensTemp > 55 || sensMoist > 90){
            // DANGER
            if (CO2>8){
                ALERT_PARAM =0;
            }
            else if (sensTemp>55){
                ALERT_PARAM =1;
            }
            else if (sensTemp>90){
                ALERT_PARAM =2;
            }
            ALERT_LEVEL=0;
            topLid.write(OPEN_LID);
            digitalWrite(BUZZER, HIGH);
            delay(longDelay);
            topLid.write(CLOSE_LID);
            digitalWrite(BUZZER, LOW);
        }
        else if (CO2> 4 || sensTemp > 40 || sensMoist > 80){
            if (CO2>4){
                ALERT_PARAM =0;
            }
            else if (sensTemp>40){
                ALERT_PARAM =1;
            }
            else if (sensMoist>80){
                ALERT_PARAM =2;
            }
            ALERT_LEVEL=1;
            digitalWrite(BUZZER, HIGH);
            delay(regDelay);
            digitalWrite(BUZZER, LOW);
        }
        String url = String(REQUEST) + "TEMPERATURE=" + String(sensTemp) + "&HUMIDITY=" + String(sensMoist) + "&CO2_CON=" + String(CO2) + "&WEIGHT=" + String(force_sensor)+ "&LEVEL=" + String(ALERT_LEVEL)+ "&PARAM=" + String(ALERT_PARAM);
        http.begin(url);
        Serial.println(url);
        int HTTP_CODE = http.GET();
        if (HTTP_CODE > 0)
        {
            String PAYLOAD = http.getString();
                 Serial.println(PAYLOAD);
        }
    }
    else
    {
        Serial.println("WIFI CONNECTION NOT ESTABLISHED, TRYING AGAIN");
    }
}
