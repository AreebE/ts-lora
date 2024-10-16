#!/usr/bin/python3
# SPDX-FileCopyrightText: 2018 Brent Rubell for Adafruit Industries
#
# SPDX-License-Identifier: MIT

"""
Example for using the RFM69HCW Radio with Raspberry Pi.

Learn Guide: https://learn.adafruit.com/lora-and-lorawan-for-raspberry-pi
Author: Brent Rubell for Adafruit Industries
"""
# Import Python System Libraries
import time
# Import Blinka Libraries
import busio
from digitalio import DigitalInOut, Direction, Pull
import board
# Import the SSD1306 module.
import adafruit_ssd1306
# Import RFM9x
import adafruit_rfm9x

import lora_consts as consts
import lora_methods as lora

airtime = round(lora.airtime_calc(consts.SPREADING_FREQUENCY, consts.CODING_RATE, 
                                consts.MAX_PACKET_SIZE, consts.BANDWIDTH) + consts.BASE_DELAY)
total_airtime_of_cycle = 0
next_slot_time = []
is_in_emergency = []
start = round(time.time() * 1000)
for i in range(consts.NUM_OF_NODES):
  total_airtime_of_cycle += airtime
  next_slot_time.append(round(start + airtime * (i + 1)))
  is_in_emergency.append(False)
nodes_recieved = 0
print(next_slot_time)

def get_delay(transmitter_id):
    current_time = round(time.time() * 1000)
    while (next_slot_time[transmitter_id] < current_time):
        next_slot_time[transmitter_id] += total_airtime_of_cycle
    delay = next_slot_time[transmitter_id] - current_time
    next_slot_time[transmitter_id] += total_airtime_of_cycle
    return delay

def analyze_data(id, data_str):
    print(data_str)

def process_package(package):
    text = str(package, "utf-8")
    transmitter_id = -1
    ack_pkg = None
    flag = 0
    if (len(text) >= 2):
        flag = int(text[1:3], 16)
        print(text[1:3])
        print("Flag = " + str(flag))
        
        if flag == consts.SET_CONNECTION_FLAG:
            global nodes_recieved
            transmitter_id = nodes_recieved
            nodes_recieved += 1
            flag = consts.SEND_ID_FLAG
            connecting_id = text[6:]
            print(connecting_id)
            ack_pkg = consts.CONNECTION_PACKAGE_FORMAT.format(flag=flag,
                                    id=transmitter_id,
                                    delay=get_delay(transmitter_id),
                                    connecting_id=connecting_id)

        if flag == consts.TRANSMISSION_FLAG:
            transmitter_id = int(text[3:5], 16)
            print("transmitter ID = " + str(transmitter_id))
            data_start = 6
            data_end = data_start + 1
            while data_end < len(text) and text[data_end] != '|':
                data_end += 1
            data_end += 1
            analyze_data(transmitter_id, text[data_start:data_end])
            flag = consts.RECEIVED_FLAG
    
    if (ack_pkg == None):
        ack_pkg = consts.ACKNOWLEDGEMENT_PACKAGE_FORMAT.format(flag=flag,
                                        id=transmitter_id,
                                        delay=get_delay(transmitter_id))

    return ack_pkg

# Create the I2C interface.
i2c = busio.I2C(board.SCL, board.SDA)

# 128x32 OLED Display
reset_pin = DigitalInOut(board.D4)
display = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c, reset=reset_pin)
# Clear the display.
display.fill(0)
display.show()
width = display.width
height = display.height

# Configure LoRa Radio
CS = DigitalInOut(board.CE1)
RESET = DigitalInOut(board.D25)
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, 434.0)
rfm9x.tx_power = 23
prev_packet = None

while True:
    packet = None
    # draw a box to clear the image
    display.fill(0)
    display.text('RasPi LoRa', 35, 0, 1)

    # check for packet rx
    packet = rfm9x.receive()
    print(packet)
    if packet is None:
        display.show()
        if (prev_packet != None):
            display.text("Current Data: {prev}".format(prev=str(prev_packet, "utf-8")), 
                        15, 20, 1)
        else:
            display.text("Current Data: None", 15, 20, 1)
    else:
        display.fill(0)
        prev_packet = packet
        packet_text = str(prev_packet, "utf-8")
        print("New packet: ")
        print(packet_text)
        ack_pkg = bytes(process_package(package=packet), "utf-8")
        rfm9x.send(ack_pkg)        
        time.sleep(1)


    display.show()
    time.sleep(0.1)