---
title: Configuration
weight: -10
---

{{< toc >}}

## Platform IO Configuration

It is prefered to use Platform IO via VS Code as it becomes easy to manage dependencies as they are added to the project
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
	miguel5612/MQUnifiedsensor@^3.0.0
	adafruit/Adafruit GFX Library@^1.11.3
	adafruit/Adafruit SSD1306@^2.5.7
	adafruit/Adafruit BusIO@^1.13.2
	adafruit/Adafruit Unified Sensor@^1.1.6
	adafruit/DHT sensor library@^1.4.4
	madhephaestus/ESP32Servo@^0.11.0

```