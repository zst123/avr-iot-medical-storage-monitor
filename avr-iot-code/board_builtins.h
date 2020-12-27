#pragma once

#include <SPI.h>
#include <Wire.h>
#include <WiFi101.h>
#include "Adafruit_MCP9808.h"

// Button pin definitions
#define BUTTON_SW1         (PIN_PF5)
#define BUTTON_RST         (PIN_PF6)

// LED pin definitions
#define LED_RED            (PIN_PD0)
#define LED_YELLOW         (PIN_PD1)
#define LED_GREEN          (PIN_PD2)
#define LED_BLUE           (PIN_PD3)

// Serial port for debugging
#define Board_Serial       (Serial1)
#define SERIAL_TXD1        (PIN_PC0)
#define SERIAL_RXD1        (PIN_PC1)

// Light sensor
#define LIGHTSENSORPIN     (PIN_PD5)

// MCP9808 temperature sensor
#define MCP9808_ADDR       (0x18)
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();


void Board_LED_Init(void) {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
}

void Board_Serial_Init(void) {
  Board_Serial.pins(SERIAL_TXD1, SERIAL_RXD1);
  Board_Serial.begin(9600);
}

void Board_Temp_Init(void) {
  //  A2 A1 A0 address
  //  0  0  0   0x18  this is the default address
  //  0  0  1   0x19
  //  0  1  0   0x1A
  //  0  1  1   0x1B
  //  1  0  0   0x1C
  //  1  0  1   0x1D
  //  1  1  0   0x1E
  //  1  1  1   0x1F
  if (!tempsensor.begin(MCP9808_ADDR)) {
    Board_Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1);
  } else {
    Board_Serial.println("Found MCP9808!");
  }

  tempsensor.wake();   // wake up, ready to read!

  tempsensor.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms
}

void Board_Light_Init() {
  pinMode(LIGHTSENSORPIN, INPUT);
}

static void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Board_Serial.print("SSID: ");
  Board_Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Board_Serial.print("IP Address: ");
  Board_Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Board_Serial.print("signal strength (RSSI):");
  Board_Serial.print(rssi);
  Board_Serial.println(" dBm");
}

void Board_Wifi_Init(char * wifi_ssid, char * wifi_pass) {
  int status = WL_IDLE_STATUS;

  // Remap to AVR-IoT pinout
  WiFi.setPins(
    PIN_PA7, // CS
    PIN_PF2, // IRQ
    PIN_PA1, // RST
    PIN_PF3 // EN
  );

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Board_Serial.println("WiFi shield not present");
    // don't continue:
    while (true) {
      digitalWrite(LED_RED, !digitalRead(LED_RED));
      delay(100);
    }
  }

  // attempt to connect to WiFi network:
  Board_Serial.print("Attempting to connect to SSID: ");
  Board_Serial.println(wifi_ssid);
  
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  status = WiFi.begin(wifi_ssid, wifi_pass);
  while (status != WL_CONNECTED) {
    digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
    delay(250);
  }
  
  Serial.println("Connected to wifi");
  printWiFiStatus();
}


//#define GWIOT_7941E_BAUDRATE        9600
#define GWIOT_7941E_PACKET_SIZE     10
#define GWIOT_7941E_PACKET_BEGIN    0x02
#define GWIOT_7941E_PACKET_END      0x03
#define GWIOT_7941E_READ_TIMEOUT    20

#define TAG_ID_LENGTH (5)
#define TAG_ID_OFFSET (3)

// https://github.com/gutierrezps/gwiot7941e/blob/master/src/Gwiot7941e.cpp
uint64_t Gwiot7941e_update(Stream *stream_) {
    uint64_t lastTagId_ = 0;
    char buff[GWIOT_7941E_PACKET_SIZE];

    if (!stream_) return false;

    if (!stream_->available()) return false;

    // if a packet doesn't begin with the right byte, remove that byte
    if (stream_->peek() != GWIOT_7941E_PACKET_BEGIN && stream_->read()) {
        return false;
    }

    // if read a packet with the wrong size, drop it
    if (GWIOT_7941E_PACKET_SIZE != stream_->readBytes(buff, GWIOT_7941E_PACKET_SIZE)) {
        return false;
    }

    // if a packet doesn't end with the right byte, drop it
    if (buff[9] != GWIOT_7941E_PACKET_END) return false;

    // calculate checksum (excluding start and end bytes)
    uint8_t checksum = 0;
    for (uint8_t i = 1; i <= 8; ++i) {
        checksum ^= buff[i];
    }
    if (checksum) return false;

    // extract tag id from message
    lastTagId_ = 0;
    for (uint8_t i = 0; i < TAG_ID_LENGTH; ++i) {
        uint8_t val = (uint8_t) buff[i+TAG_ID_OFFSET];
        lastTagId_ = (lastTagId_ << 8) | val;
    }

    return lastTagId_;
}
