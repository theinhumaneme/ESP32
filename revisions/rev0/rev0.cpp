// INCLUDE ALL LIBRARIES
#include <DHT.h>
#include <ESP32Servo.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

// INIT ALL PINS AND PARAMS FOR BETTER READABILITY

// DEFINE DHT PARAMETERS AND OBJECT
#define DHT_PIN 18
#define DHT_TYPE DHT11

// calor -> Heat in Spanish
DHT calor(DHT_PIN, DHT_TYPE);

//DEFINE SERVO AND OBJECT AND STATES
#define SERVO_PIN 17
Servo topLid;
#define OPEN_LID 0
#define CLOSE_LID 180

// DEFINE BUZZER PIN
#define BUZZER 16
// DEFINE HTTP and WiFi
WiFiMulti WIFI;
#define REQUEST "https://script.google.com/macros/s/AKfycbw9ya_ubtclOguJg2MdQg6p049LXC3TP_9N1HSJJ0Gwlf6lorxht9qnf6FF-6H1MFkJTA/exec?"

//DEFINE WIFI SSID AND PASSWORD
#define WIFI_SSID "ESP32"
#define WIFI_PASS ""

//OTHER SHORTCUTS
#define SYSTEM_OUT Serial

// DEFINE DELAYS
#define smallDelay 200
#define regDelay 3000
#define longDelay 5000
void setup() {
  // put your setup code here, to run once:
  calor.begin();
  topLid.attach(SERVO_PIN);
  SYSTEM_OUT.begin(9600);
  SYSTEM_OUT.println("PROGRAM INITIATED");
  // Add Access Point
  WIFI.addAP(WIFI_SSID, WIFI_PASS);
  topLid.write(CLOSE_LID);
  pinMode(BUZZER, OUTPUT);


}

void loop() {
  // put your main code here, to run repeatedly:
  if (WIFI.run() == WL_CONNECTED) {
    HTTPClient http;
    //    SYSTEM_OUT.println("CONNECTED TO WIFI NETWORK");
    float sensTemp = calor.readTemperature();
    float sensMoist = calor.readHumidity();
    SYSTEM_OUT.println(sensTemp);
    SYSTEM_OUT.println(sensMoist);
    topLid.write(OPEN_LID);
    digitalWrite(BUZZER, HIGH);
    delay(smallDelay);
    topLid.write(CLOSE_LID);
    digitalWrite(BUZZER, LOW);
    delay(smallDelay);
    String url = String(REQUEST) + "TEMPERATURE=" + String(sensTemp) + "&HUMIDITY=" + String(sensMoist) + "&CO2_CON=" + String("DUMMY") + "&WEIGHT=" + String("DUMMY_VAL");
    http.begin(url);
    int HTTP_CODE = http.GET();
    if (HTTP_CODE > 0) {
      String PAYLOAD = http.getString();
//      SYSTEM_OUT.println(PAYLOAD);
    }
  }
  else {
    SYSTEM_OUT.println("WIFI CONNECTION NOT ESTABLISHED, TRYING AGAIN");

  }
}
