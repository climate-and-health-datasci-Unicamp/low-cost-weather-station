# Low-Cost-Weather-Station

## Description

This is the development of a low-cost weather station. Low cost is related to power consumption and project cost. The components used in this project are relatively cheap for regular customers costing around 5-6 dollars the total price. In terms of power consumption, the code has been developed to consume the minimum energy as possible using deepsleep mode provided by ESP8266. For now, this weather station is capable to measure just temperature and humidity with high accuracy and save data in an SD CARD but new features will be appended to the project in the future.

**A new feature to update data online using the wifi module installed in ESP8266 is desired to the next update of the project.**

## List of Components

- ESP8266 NODEMCU
- RTC DS3231
- SD CARD MODULE
- HTU21D SENSOR
- 3 10kΩ RESISTORS
- 1 5uF CAPACITOR
- 2 10uF CAPACITOR
- 1 VOLTAGE REGULATOR HTL333

## Block Diagram

![Block Diagram](figures/Block_Diagram.png)

## Schematic

![Schematic](figures/Schematic.svg)
