import smbus2
import time
import keyboard

bus = smbus2.SMBus(1)
PICO_ADDR = 0x08
throttle1 = 0
throttle2 = 0

# steering/throttle tuning
accel = 20
decel = .85
steer = .6

CMD_DRIVE = 0x01
CMD_STOP  = 0x02
CMD_PING  = 0x04
PING_INTERVAL = 0.2
MSG_INTERVAL = 0.05

def bound(val, upper=255,lower=0):
    return max(min(val,upper),lower)

def send_msg(cmd, left=0, right=0):
    checksum = cmd^left^right
    try:
        msg = smbus2.i2c_msg.write(PICO_ADDR, [cmd, left, right, checksum])
        bus.i2c_rdwr(msg)
    except:
        print("msg send fail")

def send_ping():
    try:
        checksum = CMD_PING^0^0
        msg = smbus2.i2c_msg.write(PICO_ADDR, [CMD_PING, 0, 0, checksum])
        bus.i2c_rdwr(msg)
    except:
        print("msg send fail 2")
    
# hold w to accelerate, a/d to turn (wont accelerate), w+s to brake
# will slow down if no input
def get_wasd():
    # throttle
    # if keyboard.is_pressed('w'):
    #     if keyboard.is_pressed('s'):
    #         send_msg(CMD_STOP)
    #         throttle1 = 0
    #         throttle2 = 0
    #     elif keyboard.is_pressed('a'):
    #         throttle1 = steer*throttle1
    #     elif keyboard.is_pressed('d'):
    #         throttle2 = steer*throttle2
    #     else:
    #         throttle1 += accel
    #         throttle2 += accel
        
        
    # elif keyboard.is_pressed('s'):
    #     if keyboard.is_pressed('w'):
    #         send_msg(CMD_STOP)
    #         throttle1 = 0
    #         throttle2 = 0
    #     elif keyboard.is_pressed('a'):
    #         throttle1 = steer*throttle1
    #     elif keyboard.is_pressed('d'):
    #         throttle2 = steer*throttle2
    #     else:
    #         throttle1 -= accel
    #         throttle2 -= accel
    # else:
    #     throttle1 = decel*throttle1
    #     throttle2 = decel*throttle2

    # throttle1 = bound(throttle1)
    # throttle2 = bound(throttle2)
    # send_msg(CMD_DRIVE, throttle1, throttle2)
    drive = 0
    if keyboard.is_pressed('w'):
        send_msg(CMD_DRIVE, 255, 255)
    elif keyboard.is_pressed('a'):
        send_msg(CMD_DRIVE, 0, 255)
    elif keyboard.is_pressed('d'):
        send_msg(CMD_DRIVE, 255, 0)
    elif keyboard.is_pressed('s'):
        send_msg(CMD_STOP)

    
    
    



print("WASD to drive, Ctrl+C to quit")
last_ping = time.time()
last_msg = time.time()
try:
    while (True):
        # ping
        if time.time() - last_ping > PING_INTERVAL:
            send_ping()
            last_ping = time.time()
        if time.time() - last_msg > MSG_INTERVAL:
            get_wasd()
            last_msg = time.time()
except KeyboardInterrupt:
    send_msg(CMD_STOP)