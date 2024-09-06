import math

def airtime_calc(spreading_frequency,coding_rate,payload_size,bandwidth):
    H = 0        # implicit header disabled (H=0) or not (H=1)
    DE = 0       # low data rate optimization enabled (=1) or not (=0)
    Npream = 8
    if bandwidth == 125 and spreading_frequency in [11, 12]:
        # low data rate optimization mandated for BW125 with SF11 and SF12
        DE = 1
    if spreading_frequency == 6:
        # can only have implicit header with SF6
        H = 1
    Tsym = (2.0**spreading_frequency)/bandwidth
    Tpream = (Npream + 4.25)*Tsym
    payloadSymbNB = 8 + max(math.ceil(
        (8.0*payload_size-4.0*spreading_frequency+28+15*coding_rate-20*H)
        / (4.0*(spreading_frequency-2*DE)))*(coding_rate+4),0)
    Tpayload = payloadSymbNB * Tsym
    return Tpream + Tpayload