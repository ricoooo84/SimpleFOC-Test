import smbus2
import time
import pygame

PICO_ADDR = 0x08
MSG_INTERVAL = 0.01 # 10 ms

bus = smbus2.SMBus(1)

pygame.init()
pygame.joystick.init()
joystick = pygame.joystick.Joystick(0)
joystick.init()

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
    deadzone = 0.08
    turn = .2 # prevent accidental turning

    pygame.event.pump()
    left_y = joystick.get_axis(1)
    left_x = joystick.get_axis(0)

    if abs(left_y) < deadzone: left_y = 0
    if abs(left_x) < turn: left_x = 0

    if (not left_x): # if only left stick y, just move forward/back
        # y axis flipped
        left = -left_y
        right = left
    elif (not left_y): # if only left stick x, turn in place
        left = left_x
        right = -left_x
    else:                   # else, handle both

        # TODO: might need to make minimum track speed not 0 (have a minimum track speed > 0)

        if (left_x < 0):   # turning left
            right = -left_y
            left = right * (1 - abs(left_x))
        else:               # turning right
            left = -left_y
            right = left * (1 - abs(left_x))
    
    return int(left*127),int(right*127)


print("Ctrl+C to quit")
last_msg = time.time()
try:
    while (True):
        # ping
        if time.time() - last_msg > MSG_INTERVAL:
            left,right=getInput()
            send_msg(left,right)
            last_msg = time.time()
except KeyboardInterrupt:
    send_msg(0,0)
    pygame.quit()