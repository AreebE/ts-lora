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
 * * Regexp
 */
#include <SPI.h>
#include <RH_RF95.h>
#include "UUID.h"
#include "transmitter.h"

// Feather M0:
#define RFM95_CS   8
#define RFM95_RST  4
#define RFM95_INT  3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 434.0
int id = 0;
int timeTilNext = 0;
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
char message[MAX_PACKET_SIZE] = "";

void setup() {
  
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  Serial.begin(115200);
  while (!Serial) delay(1);
  delay(100);

  Serial.println("Feather LoRa TX Test!");
  randomSeed(analogRead(0));

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

  // Create message
  // note: 16777216 in hexadecimal is '10000000'
  long featherboard_identifier = random(16777216, 2147483647);
  message[0] = '!';
  message[1] = (char) SET_CONNECTION_FLAG;
  message[2] = '/'; // no id yet
  message[3] = BORDER_CHAR;
  char[8] identifier_str = ['0','0','0','0','0','0','0','0'];
  itoa(featherboard_identifier, message, 0);
  for (int i = 0; i < 8; i++) {
    message[4 + i] = identifier_str[i];
  }
  message[12] = '|';
  
  // Set up connection
  bool idNotRecieved = true;
  while (idNotRecieved) {
    
    rf95.send((uint8_t *) message, 36);
    delay(10);
    rf95.waitPacketSent();
    uint8_t buf[MAX_PACKET_SIZE];
    uint8_t len = sizeof(buf);
  
    if (rf95.waitAvailableTimeout(1000)) {
      // A package was recieved, now waiting for id.
      while (idNotRecieved) {
        if (rf95.recv(buf, &len)) {
          Serial.println((char *) buf);
          char flag = (char) (buf[1]);
          id = (char) buf[2];
          int i = 4;
          timeTilNext = 0;
          while (buf[i] != BORDER_CHAR) {
            timeTilNext = (timeTilNext * 10) + ((char) buf[i] - '0'); 
            i++;
          }
          i += 2;
          
          long identifier = 0; // To determine if this transmitter has the right acknowledgement package
          while (buf[i] != BORDER_CHAR) {
            int val = 0;
            if (buf[i] > 'a') {
              val = buf[i] - 'a' + 10;
            }
            else {
              val = buf[i] - '0';
            }
            identifier = identifier * 16 + val;
            i++;
          }
          
          if (identifier == featherboard_identifier) {
            idNotRecieved=false;
            break;
          }
          else {
            rf95.waitPacketSent();
          }
        } else {
          Serial.println("Receive failed");
        }
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

  // Resetting the message
  for (int i = 0; i < MAX_PACKET_SIZE; i++) {
    message[i] = 0;
  }
  message[0] = '!';
  message[1] = (char) TRANSMISSION_FLAG;
  message[2] = (char) id;
  message[3] = BORDER_CHAR;
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

String collect_data() {
  return "data";
}

void loop() {
  Serial.println("Transmitting..."); // Send a message to rf95_server
  String data_message = collect_data();
  int i = 0;
  for (; i < data_message.length(); i++) {
    message[4 + i] = data_message[i];
  }
  message[i] = '|';
  
  delay(10);
  rf95.send((uint8_t *) message, 64);
  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[MAX_PACKET_SIZE];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  // Maybe change?
  if (rf95.waitAvailableTimeout(1000)) {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      int timeTilNext = 0;
      int pkgID = (char) buf[3];
      if (pkgID != id) {
        return;
      }
      int i = 4;
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
