import lora_methods as lora
import lora_consts as consts
import time

DATA_PACKAGE_FORMAT = "!{flag:02X}{id:02X}|{content}|"
airtime = lora.airtime_calc(consts.SPREADING_FREQUENCY, consts.CODING_RATE, 
                                consts.MAX_PACKET_SIZE, consts.BANDWIDTH) + consts.BASE_DELAY
total_airtime_of_cycle = 0
next_slot_time = []
is_in_emergency = []
start = round(time.time() * 1000)
for i in range(consts.NUM_OF_NODES):
  total_airtime_of_cycle += airtime
  next_slot_time.append(start + airtime * (i + 1))
  is_in_emergency.append(False)
nodes_recieved = 0
print(total_airtime_of_cycle)
print(next_slot_time)

def get_delay(transmitter_id):
    delay = round(next_slot_time[transmitter_id] - round(time.time() * 1000))
    if (delay) < 0:
        delay = 0
    return delay

input("1")
print(round(time.time() * 1000))
print(get_delay(0))
print(get_delay(1))