---
title: Final Code
weight: -10
---

# Hardware Code

```cpp
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
// #define REQUEST "https://script.google.com/macros/s/AKfycbw9ya_ubtclOguJg2MdQg6p049LXC3TP_9N1HSJJ0Gwlf6lorxht9qnf6FF-6H1MFkJTA/exec?"
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
```

# Google Scripts

```js
const FOLDER_DUMP = "<FOLDER_ID>";

function doGet(e) {
  var content = ContentService.createTextOutput(JSON.stringify(e, null, 2));
  console.log(e);
  storeData(e);
  return content;
}

function storeData(values) {
  var date = new Date();
  var current_date = String(date.getDate()).concat(
    "-",
    date.getMonth() + 1,
    "-",
    date.getFullYear()
  );
  console.log(current_date);
  var folder = DriveApp.getFolderById(FOLDER_DUMP);
  var current_file_name = String(date.getFullYear()).concat(
    "-",
    date.getMonth() + 1
  );
  console.log(current_file_name);
  var current_file = DriveApp.getFilesByName(current_file_name);
  console.log(current_file.hasNext());
  if (current_file.hasNext() == false) {
    var new_SS = SpreadsheetApp.create(current_file_name).getId();
    DriveApp.getFileById(new_SS).moveTo(folder);
    SpreadsheetApp.setActiveSpreadsheet(
      SpreadsheetApp.openById(
        folder.getFilesByName(current_file_name).next().getId()
      )
    );
    const googleSheet = SpreadsheetApp.getActiveSpreadsheet().getSheets()[0];
    googleSheet.setName(current_date);
    googleSheet.appendRow([
      "DATE",
      "TIME",
      "TEMPERATURE(C)",
      "HUMIDITY(Relative Humidity)",
      "CO2 CON(PPM)",
      "WEIGHT(N)",
    ]);
    var range = googleSheet.getRange(1, 1, googleSheet.getLastRow(), 6);
    range.setHorizontalAlignment("left");
  } else {
    SpreadsheetApp.setActiveSpreadsheet(
      SpreadsheetApp.openById(
        folder.getFilesByName(current_file_name).next().getId()
      )
    );
  }
  Logger.log(values);
  var TEMPERATURE = values["parameter"]["TEMPERATURE"];
  var HUMIDITY = values["parameter"]["HUMIDITY"];
  var CO2_CON = values["parameter"]["CO2_CON"];
  var WEIGHT = values["parameter"]["WEIGHT"];
  var DATA = [
    current_date,
    date.toTimeString().split(" ")[0],
    TEMPERATURE,
    HUMIDITY,
    CO2_CON,
    WEIGHT,
  ];
  var PARAM = values["parameter"]["PARAM"];
  var ALERT = values["parameter"]["LEVEL"];
  const googleSheet = SpreadsheetApp.getActiveSpreadsheet();
  var sheet_by_date = googleSheet.getSheetByName(current_date);
  if (sheet_by_date == null) {
    var currentDaySheet = googleSheet.insertSheet();
    currentDaySheet.setName(current_date);
    currentDaySheet.appendRow([
      "DATE",
      "TIME",
      "TEMPERATURE",
      "HUMIDITY",
      "CO2 CON",
      "WEIGHT",
    ]);
  }
  if (ALERT == "2") {
    var MESSAGE = "NORMAL";
    var COLOUR = "7cfc00";
  } else if (ALERT == "1") {
    var MESSAGE = "WARNING";
    var COLOUR = "ffa500";
  } else if (ALERT == "0") {
    var MESSAGE = "CRITICAL";
    var COLOUR = "cc0000";
  }
  if (PARAM == "3") {
    var PARAMETER = "Everything is Normal";
  } else if (PARAM == "2") {
    var PARAMETER = "Humidity levels abnormal";
  } else if (PARAM == "1") {
    var PARAMETER = "Temperature levels abnormal";
  } else if (PARAM == "0") {
    var PARAMETER = "C02 levels abnormal";
  }
  vals = [MESSAGE, COLOUR, PARAMETER, TEMPERATURE, HUMIDITY, CO2_CON, WEIGHT];
  sendEmailMessage(vals);
  var active_sheet = googleSheet.getSheetByName(current_date);
  console.log(DATA);
  active_sheet.appendRow(DATA);
  var range = active_sheet.getRange(1, 1, active_sheet.getLastRow(), 6);
  range.setHorizontalAlignment("left");
}

function sendEmailMessage(vals, subject) {
  var t = HtmlService.createTemplateFromFile("Index");
  var date = new Date();
  t.timestamp = String(date.getDate()).concat(
    "-",
    date.getMonth() + 1,
    "-",
    date.getFullYear(),
    "-",
    date.toTimeString().split(" ")[0]
  );
  t.alert_level = vals[0];
  t.color = vals[1];
  t.parameter = vals[2];
  t.temperature = vals[3];
  t.humidity = vals[4];
  t.co2 = vals[5];
  t.weight = vals[6];
  MailApp.sendEmail({
    to: "<EMAIL_ID>",
    subject: "SILO REPORT",
    htmlBody: t.evaluate().getContent(),
  });
}
```

# Email Template

```html
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html
  xmlns="http://www.w3.org/1999/xhtml"
  xmlns:o="urn:schemas-microsoft-com:office:office"
  style="font-family:arial, 'helvetica neue', helvetica, sans-serif"
>
  <head>
    <meta charset="UTF-8" />
    <meta content="width=device-width, initial-scale=1" name="viewport" />
    <meta name="x-apple-disable-message-reformatting" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <meta content="telephone=no" name="format-detection" />
    <title>New message</title>
    <!--[if (mso 16)]>
      <style type="text/css">
        a {
          text-decoration: none;
        }
      </style>
    <![endif]-->
    <!--[if gte mso 9
      ]><style>
        sup {
          font-size: 100% !important;
        }
      </style><!
    [endif]-->
    <!--[if gte mso 9]>
      <xml>
        <o:OfficeDocumentSettings>
          <o:AllowPNG></o:AllowPNG>
          <o:PixelsPerInch>96</o:PixelsPerInch>
        </o:OfficeDocumentSettings>
      </xml>
    <![endif]-->
    <style type="text/css">
      #outlook a {
        padding: 0;
      }
      .es-button {
        mso-style-priority: 100 !important;
        text-decoration: none !important;
      }
      a[x-apple-data-detectors] {
        color: inherit !important;
        text-decoration: none !important;
        font-size: inherit !important;
        font-family: inherit !important;
        font-weight: inherit !important;
        line-height: inherit !important;
      }
      .es-desk-hidden {
        display: none;
        float: left;
        overflow: hidden;
        width: 0;
        max-height: 0;
        line-height: 0;
        mso-hide: all;
      }
      [data-ogsb] .es-button {
        border-width: 0 !important;
        padding: 10px 30px 10px 30px !important;
      }
      @media only screen and (max-width: 600px) {
        p,
        ul li,
        ol li,
        a {
          line-height: 150% !important;
        }
        h1,
        h2,
        h3,
        h1 a,
        h2 a,
        h3 a {
          line-height: 120% !important;
        }
        h1 {
          font-size: 36px !important;
          text-align: left;
        }
        h2 {
          font-size: 26px !important;
          text-align: left;
        }
        h3 {
          font-size: 20px !important;
          text-align: left;
        }
        .es-header-body h1 a,
        .es-content-body h1 a,
        .es-footer-body h1 a {
          font-size: 36px !important;
          text-align: left;
        }
        .es-header-body h2 a,
        .es-content-body h2 a,
        .es-footer-body h2 a {
          font-size: 26px !important;
          text-align: left;
        }
        .es-header-body h3 a,
        .es-content-body h3 a,
        .es-footer-body h3 a {
          font-size: 20px !important;
          text-align: left;
        }
        .es-menu td a {
          font-size: 12px !important;
        }
        .es-header-body p,
        .es-header-body ul li,
        .es-header-body ol li,
        .es-header-body a {
          font-size: 14px !important;
        }
        .es-content-body p,
        .es-content-body ul li,
        .es-content-body ol li,
        .es-content-body a {
          font-size: 14px !important;
        }
        .es-footer-body p,
        .es-footer-body ul li,
        .es-footer-body ol li,
        .es-footer-body a {
          font-size: 14px !important;
        }
        .es-infoblock p,
        .es-infoblock ul li,
        .es-infoblock ol li,
        .es-infoblock a {
          font-size: 12px !important;
        }
        *[class="gmail-fix"] {
          display: none !important;
        }
        .es-m-txt-c,
        .es-m-txt-c h1,
        .es-m-txt-c h2,
        .es-m-txt-c h3 {
          text-align: center !important;
        }
        .es-m-txt-r,
        .es-m-txt-r h1,
        .es-m-txt-r h2,
        .es-m-txt-r h3 {
          text-align: right !important;
        }
        .es-m-txt-l,
        .es-m-txt-l h1,
        .es-m-txt-l h2,
        .es-m-txt-l h3 {
          text-align: left !important;
        }
        .es-m-txt-r img,
        .es-m-txt-c img,
        .es-m-txt-l img {
          display: inline !important;
        }
        .es-button-border {
          display: inline-block !important;
        }
        a.es-button,
        button.es-button {
          font-size: 20px !important;
          display: inline-block !important;
        }
        .es-adaptive table,
        .es-left,
        .es-right {
          width: 100% !important;
        }
        .es-content table,
        .es-header table,
        .es-footer table,
        .es-content,
        .es-footer,
        .es-header {
          width: 100% !important;
          max-width: 600px !important;
        }
        .es-adapt-td {
          display: block !important;
          width: 100% !important;
        }
        .adapt-img {
          width: 100% !important;
          height: auto !important;
        }
        .es-m-p0 {
          padding: 0 !important;
        }
        .es-m-p0r {
          padding-right: 0 !important;
        }
        .es-m-p0l {
          padding-left: 0 !important;
        }
        .es-m-p0t {
          padding-top: 0 !important;
        }
        .es-m-p0b {
          padding-bottom: 0 !important;
        }
        .es-m-p20b {
          padding-bottom: 20px !important;
        }
        .es-mobile-hidden,
        .es-hidden {
          display: none !important;
        }
        tr.es-desk-hidden,
        td.es-desk-hidden,
        table.es-desk-hidden {
          width: auto !important;
          overflow: visible !important;
          float: none !important;
          max-height: inherit !important;
          line-height: inherit !important;
        }
        tr.es-desk-hidden {
          display: table-row !important;
        }
        table.es-desk-hidden {
          display: table !important;
        }
        td.es-desk-menu-hidden {
          display: table-cell !important;
        }
        .es-menu td {
          width: 1% !important;
        }
        table.es-table-not-adapt,
        .esd-block-html table {
          width: auto !important;
        }
        table.es-social {
          display: inline-block !important;
        }
        table.es-social td {
          display: inline-block !important;
        }
        .es-m-p5 {
          padding: 5px !important;
        }
        .es-m-p5t {
          padding-top: 5px !important;
        }
        .es-m-p5b {
          padding-bottom: 5px !important;
        }
        .es-m-p5r {
          padding-right: 5px !important;
        }
        .es-m-p5l {
          padding-left: 5px !important;
        }
        .es-m-p10 {
          padding: 10px !important;
        }
        .es-m-p10t {
          padding-top: 10px !important;
        }
        .es-m-p10b {
          padding-bottom: 10px !important;
        }
        .es-m-p10r {
          padding-right: 10px !important;
        }
        .es-m-p10l {
          padding-left: 10px !important;
        }
        .es-m-p15 {
          padding: 15px !important;
        }
        .es-m-p15t {
          padding-top: 15px !important;
        }
        .es-m-p15b {
          padding-bottom: 15px !important;
        }
        .es-m-p15r {
          padding-right: 15px !important;
        }
        .es-m-p15l {
          padding-left: 15px !important;
        }
        .es-m-p20 {
          padding: 20px !important;
        }
        .es-m-p20t {
          padding-top: 20px !important;
        }
        .es-m-p20r {
          padding-right: 20px !important;
        }
        .es-m-p20l {
          padding-left: 20px !important;
        }
        .es-m-p25 {
          padding: 25px !important;
        }
        .es-m-p25t {
          padding-top: 25px !important;
        }
        .es-m-p25b {
          padding-bottom: 25px !important;
        }
        .es-m-p25r {
          padding-right: 25px !important;
        }
        .es-m-p25l {
          padding-left: 25px !important;
        }
        .es-m-p30 {
          padding: 30px !important;
        }
        .es-m-p30t {
          padding-top: 30px !important;
        }
        .es-m-p30b {
          padding-bottom: 30px !important;
        }
        .es-m-p30r {
          padding-right: 30px !important;
        }
        .es-m-p30l {
          padding-left: 30px !important;
        }
        .es-m-p35 {
          padding: 35px !important;
        }
        .es-m-p35t {
          padding-top: 35px !important;
        }
        .es-m-p35b {
          padding-bottom: 35px !important;
        }
        .es-m-p35r {
          padding-right: 35px !important;
        }
        .es-m-p35l {
          padding-left: 35px !important;
        }
        .es-m-p40 {
          padding: 40px !important;
        }
        .es-m-p40t {
          padding-top: 40px !important;
        }
        .es-m-p40b {
          padding-bottom: 40px !important;
        }
        .es-m-p40r {
          padding-right: 40px !important;
        }
        .es-m-p40l {
          padding-left: 40px !important;
        }
        .es-desk-hidden {
          display: table-row !important;
          width: auto !important;
          overflow: visible !important;
          max-height: inherit !important;
        }
      }
    </style>
  </head>
  <body
    data-new-gr-c-s-loaded="14.1070.0"
    style="width:100%;font-family:arial, 'helvetica neue', helvetica, sans-serif;-webkit-text-size-adjust:100%;-ms-text-size-adjust:100%;padding:0;Margin:0"
  >
    <div class="es-wrapper-color" style="background-color:#FAFAFA">
      <!--[if gte mso 9]>
        <v:background xmlns:v="urn:schemas-microsoft-com:vml" fill="t">
          <v:fill type="tile" color="#fafafa"></v:fill>
        </v:background>
      <![endif]-->
      <table
        class="es-wrapper"
        width="100%"
        cellspacing="0"
        cellpadding="0"
        style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;padding:0;Margin:0;width:100%;height:100%;background-repeat:repeat;background-position:center top;background-color:#FAFAFA"
      >
        <tr>
          <td valign="top" style="padding:0;Margin:0">
            <table
              cellpadding="0"
              cellspacing="0"
              class="es-content"
              align="center"
              style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;table-layout:fixed !important;width:100%"
            >
              <tr>
                <td
                  class="es-info-area"
                  align="center"
                  style="padding:0;Margin:0"
                >
                  <table
                    class="es-content-body"
                    align="center"
                    cellpadding="0"
                    cellspacing="0"
                    style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;background-color:transparent;width:600px"
                    bgcolor="#FFFFFF"
                  >
                    <tr>
                      <td align="left" style="padding:20px;Margin:0">
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          width="100%"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                        >
                          <tr>
                            <td
                              align="center"
                              valign="top"
                              style="padding:0;Margin:0;width:560px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td
                                    align="center"
                                    style="padding:0;Margin:0;display:none"
                                  ></td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                      </td>
                    </tr>
                  </table>
                </td>
              </tr>
            </table>
            <table
              cellpadding="0"
              cellspacing="0"
              class="es-header"
              align="center"
              style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;table-layout:fixed !important;width:100%;background-color:transparent;background-repeat:repeat;background-position:center top"
            >
              <tr>
                <td align="center" style="padding:0;Margin:0">
                  <table
                    bgcolor="#ffffff"
                    class="es-header-body"
                    align="center"
                    cellpadding="0"
                    cellspacing="0"
                    style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;background-color:transparent;width:600px"
                  >
                    <tr>
                      <td
                        align="left"
                        style="Margin:0;padding-top:10px;padding-bottom:10px;padding-left:20px;padding-right:20px"
                      >
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          width="100%"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                        >
                          <tr>
                            <td
                              class="es-m-p0r"
                              valign="top"
                              align="center"
                              style="padding:0;Margin:0;width:560px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td
                                    align="center"
                                    style="padding:0;Margin:0;display:none"
                                  ></td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                      </td>
                    </tr>
                  </table>
                </td>
              </tr>
            </table>
            <table
              cellpadding="0"
              cellspacing="0"
              class="es-content"
              align="center"
              style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;table-layout:fixed !important;width:100%"
            >
              <tr>
                <td align="center" style="padding:0;Margin:0">
                  <table
                    bgcolor="#ffffff"
                    class="es-content-body"
                    align="center"
                    cellpadding="0"
                    cellspacing="0"
                    style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;background-color:#FFFFFF;width:600px"
                  >
                    <tr>
                      <td
                        align="left"
                        style="padding:0;Margin:0;padding-top:15px;padding-left:20px;padding-right:20px"
                      >
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          width="100%"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                        >
                          <tr>
                            <td
                              align="center"
                              valign="top"
                              style="padding:0;Margin:0;width:560px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td
                                    align="center"
                                    style="padding:0;Margin:0;padding-top:10px;padding-bottom:10px;font-size:0px"
                                  >
                                    <img
                                      src="https://jtwivw.stripocdn.email/content/guids/CABINET_d0083983c1a429c1d9272e38a5b350a3/images/1401618430238266.png"
                                      alt
                                      style="display:block;border:0;outline:none;text-decoration:none;-ms-interpolation-mode:bicubic"
                                      width="100"
                                    />
                                  </td>
                                </tr>
                                <tr>
                                  <td
                                    align="center"
                                    class="es-m-txt-c"
                                    style="padding:0;Margin:0;padding-bottom:20px"
                                  >
                                    <h1
                                      style="Margin:0;line-height:46px;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;font-size:46px;font-style:normal;font-weight:bold;color:#333333"
                                    >
                                      <?= parameter ?>
                                    </h1>
                                  </td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                      </td>
                    </tr>
                  </table>
                </td>
              </tr>
            </table>
            <table
              cellpadding="0"
              cellspacing="0"
              class="es-content"
              align="center"
              style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;table-layout:fixed !important;width:100%"
            >
              <tr>
                <td align="center" style="padding:0;Margin:0">
                  <table
                    bgcolor="#ffffff"
                    class="es-content-body"
                    align="center"
                    cellpadding="0"
                    cellspacing="0"
                    style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;background-color:#FFFFFF;width:600px"
                  >
                    <tr>
                      <td
                        align="left"
                        style="padding:0;Margin:0;padding-bottom:10px;padding-left:20px;padding-right:20px"
                      >
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          width="100%"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                        >
                          <tr>
                            <td
                              align="left"
                              style="padding:0;Margin:0;width:560px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td
                                    align="center"
                                    style="padding:0;Margin:0;padding-top:10px;padding-bottom:20px"
                                  >
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:21px;color:#333333;font-size:14px"
                                    >
                                      <?= parameter ?>, this event has occurred
                                      at
                                      <?= timestamp ?>, please make sure to
                                      neccessary actions before the value
                                      reaches the threshold value, else
                                      emergency measures will be taken&nbsp;to
                                      mitigate critical issues.
                                    </p>
                                  </td>
                                </tr>
                                <tr>
                                  <td align="center" style="padding:0;Margin:0">
                                    <span
                                      class="es-button-border"
                                      style="border-style:solid;border-color:#2CB543;background:#cc0000;border-width:0px;display:inline-block;border-radius:5px;width:auto"
                                      ><a
                                        href=""
                                        class="es-button"
                                        target="_blank"
                                        style="mso-style-priority:100 !important;text-decoration:none;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;color:#FFFFFF;font-size:20px;border-style:solid;border-color:#<?= color ?>;border-width:10px 30px 10px 30px;display:inline-block;background:#<?= color ?>;border-radius:5px;font-family:arial, 'helvetica neue', helvetica, sans-serif;font-weight:normal;font-style:normal;line-height:24px;width:auto;text-align:center"
                                        ><?= alert_level?></a
                                      ></span
                                    >
                                  </td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                      </td>
                    </tr>
                    <tr>
                      <td
                        align="left"
                        style="padding:0;Margin:0;padding-top:20px;padding-left:20px;padding-right:20px"
                      >
                        <!--[if mso]><table style="width:560px" cellpadding="0" cellspacing="0"><tr><td style="width:145px" valign="top"><![endif]-->
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          class="es-left"
                          align="left"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;float:left"
                        >
                          <tr>
                            <td
                              class="es-m-p0r es-m-p20b"
                              align="center"
                              style="padding:0;Margin:0;width:125px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td align="center" style="padding:0;Margin:0">
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:21px;color:#333333;font-size:14px"
                                    >
                                      TEMPERATURE<br /><?= temperature ?>
                                    </p>
                                  </td>
                                </tr>
                              </table>
                            </td>
                            <td
                              class="es-hidden"
                              style="padding:0;Margin:0;width:20px"
                            ></td>
                          </tr>
                        </table>
                        <!--[if mso]></td><td style="width:145px" valign="top"><![endif]-->
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          class="es-left"
                          align="left"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;float:left"
                        >
                          <tr>
                            <td
                              class="es-m-p20b"
                              align="center"
                              style="padding:0;Margin:0;width:125px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td align="center" style="padding:0;Margin:0">
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:21px;color:#333333;font-size:14px"
                                    >
                                      HUMIDITY
                                    </p>
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:21px;color:#333333;font-size:14px"
                                    >
                                      <?= humidity ?>
                                    </p>
                                  </td>
                                </tr>
                              </table>
                            </td>
                            <td
                              class="es-hidden"
                              style="padding:0;Margin:0;width:20px"
                            ></td>
                          </tr>
                        </table>
                        <!--[if mso]></td><td style="width:125px" valign="top"><![endif]-->
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          class="es-left"
                          align="left"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;float:left"
                        >
                          <tr>
                            <td
                              class="es-m-p20b"
                              align="center"
                              style="padding:0;Margin:0;width:125px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td align="center" style="padding:0;Margin:0">
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:21px;color:#333333;font-size:14px"
                                    >
                                      CO2 Level<br /><?= co2 ?>
                                    </p>
                                  </td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                        <!--[if mso]></td><td style="width:20px"></td><td style="width:125px" valign="top"><![endif]-->
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          class="es-right"
                          align="right"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;float:right"
                        >
                          <tr>
                            <td
                              align="center"
                              style="padding:0;Margin:0;width:125px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td align="center" style="padding:0;Margin:0">
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:21px;color:#333333;font-size:14px"
                                    >
                                      WEIGHT<br /><?= weight ?>
                                    </p>
                                  </td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                        <!--[if mso]></td></tr></table><![endif]-->
                      </td>
                    </tr>
                    <tr>
                      <td
                        align="left"
                        style="Margin:0;padding-bottom:10px;padding-top:15px;padding-left:20px;padding-right:20px"
                      >
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          width="100%"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                        >
                          <tr>
                            <td
                              align="left"
                              style="padding:0;Margin:0;width:560px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td
                                    align="center"
                                    style="padding:0;Margin:0;padding-top:10px;padding-bottom:10px"
                                  >
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:21px;color:#333333;font-size:14px"
                                    >
                                      Got a question?&nbsp;Email me&nbsp;at
                                      <a
                                        target="_blank"
                                        href="mailto:theinhumaneme@gmail.com"
                                        style="-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;text-decoration:underline;color:#5C68E2;font-size:14px"
                                        >theinhumaneme@gmail.com</a
                                      >
                                    </p>
                                  </td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                      </td>
                    </tr>
                  </table>
                </td>
              </tr>
            </table>
            <table
              cellpadding="0"
              cellspacing="0"
              class="es-footer"
              align="center"
              style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;table-layout:fixed !important;width:100%;background-color:transparent;background-repeat:repeat;background-position:center top"
            >
              <tr>
                <td align="center" style="padding:0;Margin:0">
                  <table
                    class="es-footer-body"
                    align="center"
                    cellpadding="0"
                    cellspacing="0"
                    style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;background-color:transparent;width:640px"
                  >
                    <tr>
                      <td
                        align="left"
                        style="Margin:0;padding-top:20px;padding-bottom:20px;padding-left:20px;padding-right:20px"
                      >
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          width="100%"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                        >
                          <tr>
                            <td
                              align="left"
                              style="padding:0;Margin:0;width:600px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td
                                    align="center"
                                    style="padding:0;Margin:0;padding-top:15px;padding-bottom:15px;font-size:0"
                                  >
                                    <table
                                      cellpadding="0"
                                      cellspacing="0"
                                      class="es-table-not-adapt es-social"
                                      style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                                    >
                                      <tr>
                                        <td
                                          align="center"
                                          valign="top"
                                          style="padding:0;Margin:0"
                                        >
                                          <img
                                            title="Youtube"
                                            src="https://jtwivw.stripocdn.email/content/assets/img/social-icons/logo-black/youtube-logo-black.png"
                                            alt="Yt"
                                            width="32"
                                            style="display:block;border:0;outline:none;text-decoration:none;-ms-interpolation-mode:bicubic"
                                          />
                                        </td>
                                        <td
                                          align="center"
                                          valign="top"
                                          style="padding:0;Margin:0"
                                        >
                                          <a
                                            target="_blank"
                                            href="https://www.instagram.com/theinhumaneme/"
                                            style="-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;text-decoration:underline;color:#333333;font-size:12px"
                                            ><img
                                              title="Instagram"
                                              src="https://jtwivw.stripocdn.email/content/assets/img/social-icons/logo-black/instagram-logo-black.png"
                                              alt="Ig"
                                              width="32"
                                              style="display:block;border:0;outline:none;text-decoration:none;-ms-interpolation-mode:bicubic"
                                          /></a>
                                        </td>
                                        <td
                                          align="center"
                                          valign="top"
                                          style="padding:0;Margin:0"
                                        >
                                          <a
                                            target="_blank"
                                            href="https://www.linkedin.com/in/kalyan-mudumby-736a40166/"
                                            style="-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;text-decoration:underline;color:#333333;font-size:12px"
                                            ><img
                                              title="Linkedin"
                                              src="https://jtwivw.stripocdn.email/content/assets/img/social-icons/logo-black/linkedin-logo-black.png"
                                              alt="In"
                                              width="32"
                                              style="display:block;border:0;outline:none;text-decoration:none;-ms-interpolation-mode:bicubic"
                                          /></a>
                                        </td>
                                      </tr>
                                    </table>
                                  </td>
                                </tr>
                                <tr>
                                  <td
                                    align="center"
                                    style="padding:0;Margin:0;padding-bottom:35px"
                                  >
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:18px;color:#333333;font-size:12px"
                                    >
                                      TEAM E-13, Geethanjali College of
                                      Engineering and Technology
                                    </p>
                                  </td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                      </td>
                    </tr>
                  </table>
                </td>
              </tr>
            </table>
            <table
              cellpadding="0"
              cellspacing="0"
              class="es-content"
              align="center"
              style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;table-layout:fixed !important;width:100%"
            >
              <tr>
                <td
                  class="es-info-area"
                  align="center"
                  style="padding:0;Margin:0"
                >
                  <table
                    class="es-content-body"
                    align="center"
                    cellpadding="0"
                    cellspacing="0"
                    style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px;background-color:transparent;width:600px"
                    bgcolor="#FFFFFF"
                  >
                    <tr>
                      <td align="left" style="padding:20px;Margin:0">
                        <table
                          cellpadding="0"
                          cellspacing="0"
                          width="100%"
                          style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                        >
                          <tr>
                            <td
                              align="center"
                              valign="top"
                              style="padding:0;Margin:0;width:560px"
                            >
                              <table
                                cellpadding="0"
                                cellspacing="0"
                                width="100%"
                                style="mso-table-lspace:0pt;mso-table-rspace:0pt;border-collapse:collapse;border-spacing:0px"
                              >
                                <tr>
                                  <td
                                    align="center"
                                    class="es-infoblock"
                                    style="padding:0;Margin:0;line-height:14px;font-size:12px;color:#CCCCCC"
                                  >
                                    <p
                                      style="Margin:0;-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;font-family:arial, 'helvetica neue', helvetica, sans-serif;line-height:14px;color:#CCCCCC;font-size:12px"
                                    >
                                      <a
                                        target="_blank"
                                        href=""
                                        style="-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;text-decoration:underline;color:#CCCCCC;font-size:12px"
                                      ></a
                                      >No longer want to receive these
                                      emails?&nbsp;<a
                                        href=""
                                        target="_blank"
                                        style="-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;text-decoration:underline;color:#CCCCCC;font-size:12px"
                                        >Unsubscribe</a
                                      >.<a
                                        target="_blank"
                                        href=""
                                        style="-webkit-text-size-adjust:none;-ms-text-size-adjust:none;mso-line-height-rule:exactly;text-decoration:underline;color:#CCCCCC;font-size:12px"
                                      ></a>
                                    </p>
                                  </td>
                                </tr>
                              </table>
                            </td>
                          </tr>
                        </table>
                      </td>
                    </tr>
                  </table>
                </td>
              </tr>
            </table>
          </td>
        </tr>
      </table>
    </div>
  </body>
</html>
```

# PINOUT

| Sensor / Component | Pin                  | Board Pin |
| :----------------- | :------------------- | :-------- |
| DHT11              | OUT                  | 18        |
| MQ135              | AO adjusted_voltage  | 35        |
| Force Sensor       | adjusted_voltage pin | 32        |
| Servo              | Control Pin          | 17        |
| OLED               | SCL/SCK              | 22        |
| OLED               | SDA                  | 21        |
| Buzzer             | + pin                | 16        |
