// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

/*
 * Added libaries:
 * * UUID
 * 
 */
#include <SPI.h>
#include <RH_RF95.h>
#include "UUID.h"

// First 3 here are boards w/radio BUILT-IN. Boards using FeatherWing follow.
//#if defined (__AVR_ATmega32U4__)  // Feather 32u4 w/Radio
//  #define RFM95_CS    8
//  #define RFM95_INT   7
//  #define RFM95_RST   4
//
//#elif defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)  // Feather M0 w/Radio
//  #define RFM95_CS    8
//  #define RFM95_INT   3
//  #define RFM95_RST   4
//
//#elif defined(ARDUINO_ADAFRUIT_FEATHER_RP2040_RFM)  // Feather RP2040 w/Radio
//  #define RFM95_CS   16
//  #define RFM95_INT  21
//  #define RFM95_RST  17
//
//#elif defined (__AVR_ATmega328P__)  // Feather 328P w/wing
//  #define RFM95_CS    4  //
//  #define RFM95_INT   3  //
//  #define RFM95_RST   2  // "A"
//
//#elif defined(ESP8266)  // ESP8266 feather w/wing
//  #define RFM95_CS    2  // "E"
//  #define RFM95_INT  15  // "B"
//  #define RFM95_RST  16  // "D"
//
//#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_NRF52840_FEATHER) || defined(ARDUINO_NRF52840_FEATHER_SENSE)
//  #define RFM95_CS   10  // "B"
//  #define RFM95_INT   9  // "A"
//  #define RFM95_RST  11  // "C"
//
//#elif defined(ESP32)  // ESP32 feather w/wing
//  #define RFM95_CS   33  // "B"
//  #define RFM95_INT  27  // "A"
//  #define RFM95_RST  13
//
//#elif defined(ARDUINO_NRF52832_FEATHER)  // nRF52832 feather w/wing
//  #define RFM95_CS   11  // "B"
//  #define RFM95_INT  31  // "C"
//  #define RFM95_RST   7  // "A"
//
//#endif

///* Some other possible setups include:
//
//// Feather 32u4:
//#define RFM95_CS   8
//#define RFM95_RST  4
//#define RFM95_INT  7

// Feather M0:
#define RFM95_CS   8
#define RFM95_RST  4
#define RFM95_INT  3

//// Arduino shield:
//#define RFM95_CS  10
//#define RFM95_RST  9
//#define RFM95_INT  7

//// Feather 32u4 w/wing:
//#define RFM95_RST 11  // "A"
//#define RFM95_CS  10  // "B"
//#define RFM95_INT  2  // "SDA" (only SDA/SCL/RX/TX have IRQ!)
//
//// Feather m0 w/wing:
//#define RFM95_RST 11  // "A"
//#define RFM95_CS  10  // "B"
//#define RFM95_INT  6  // "D"
//*/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0
int id = 0;
int timeTilNext = 0;
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while (!Serial) delay(1);
  delay(100);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Set up connection
  bool idNotRecieved = true;
  while (idNotRecieved) {

    rf95.send((uint8_t *) "!CO", 36);
    delay(10);
    rf95.waitPacketSent();
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
  
    if (rf95.waitAvailableTimeout(1000)) {
      if (rf95.recv(buf, &len)) {
        int i = 3;
        id = 0;
        while (buf[i] != '|') {
          id = id * 10 + (buf[i] - '0');
          i++;
        }
        i++;
        while (i < sizeof(buf)) {
          timeTilNext = (timeTilNext * 10) + (buf[i] - '0'); 
          i++;
        }
        idNotRecieved=false;
      } else {
        Serial.println("Receive failed");
      }
    } else {
      Serial.println("No reply, is there a listener around?");
    }
  }
  delay(timeTilNext);
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

String collect_data() {
  return "data";
}

void loop() {
  Serial.println("Transmitting..."); // Send a message to rf95_server
  String data_message = "!TR" + String(id) + "|" + collect_data();
  // itoa(packetnum++, data_message+13, 10);
  Serial.print("Sending "); Serial.println(data_message);
  data_message.setCharAt(data_message.length() - 1, 0);
  Serial.println("Sending...");
  
  delay(10);
  char data_buffer[64] = {};
  data_message.toCharArray(data_buffer, 64);
  rf95.send((uint8_t *) data_buffer, 64);
  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  // Maybe change?
  if (rf95.waitAvailableTimeout(1000)) {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      int timeTilNext = 0;
      int i = 3;
      while (buf[i] != '|') {
        i++;
      }
      i++;
      while (i < sizeof(buf) && (buf[i] >= '0' && buf[i] <= '9')) {
        timeTilNext = (timeTilNext * 10) + (buf[i] - '0'); 
        i++;
      }
      Serial.println((char*)buf);
      Serial.print("Recieves next at: ");
      Serial.println(timeTilNext);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      delay(timeTilNext);
    } else {
      Serial.println("Receive failed");
    }
  } else {
    Serial.println("No reply, is there a listener around?");
  }

}
