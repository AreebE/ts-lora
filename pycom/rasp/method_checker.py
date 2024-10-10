import lora_methods as lora
import lora_consts as lc
print(lora.airtime_calc(lc.SPREADING_FREQUENCY, lc.CODING_RATE, lc.MAX_PACKET_SIZE, lc.BANDWIDTH))
print(138.5 - 143.616) # 64
print(112.9 - 112.896) # 48
print(92.4 - 88.32) # 32
# Note: Coding rate must be in integers, from 1 to 4. Represents num of 'real' bytes compared to 'filler'.
# Closer to expected results from this calculator (https://www.thethingsnetwork.org/airtime-calculator) when coding rate is 2.
# https://iftnt.github.io/lora-air-time/index.html. Noteably different from given results.
# However, neither mentions different device.
# Updated formula to properly include coding rate. Makes results closer to expected value, though some noteable differences.
# Issue may be in how payload is calculated. Changing DE or H does not appear to meaningfully change results.
# Formula also matches my notes after some updates.

# +- 10%