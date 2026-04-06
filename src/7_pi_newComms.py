import smbus2
import time

bus = smbus2.SMBus(1)
PICO_ADDR = 0x08

MSG_INTERVAL = 0.01 # 10 ms

def send_msg(left, right):
    # force left and right into 8 bit
    l = 0xFF & left
    r = 0xFF & right
    checksum = l ^ r
    try:
        msg = smbus2.i2c_msg.write(PICO_ADDR, [l, r, checksum])
        bus.i2c_rdwr(msg)
    except:
        print("msg send fail")

def getInput():
    1 # do later

print("Ctrl+C to quit")
last_msg = time.time()
try:
    while (True):
        # ping
        if time.time() - last_msg > MSG_INTERVAL:
            getInput()
            last_msg = time.time()
except KeyboardInterrupt:
    send_msg(0,0)