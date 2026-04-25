import smbus2
import time
import pygame
import cv2
from flask import Flask, Response
import threading

# THINGS TO CHANGE
# make this script auto run when pi turns on

# THINGS TO ADD RN

# read tofs, handle i2c
# add a send_msg_motor if statement to cap track movement to 0 if front tof is too close
# add data to web server

MOTOR_ADDR = 0x08
TOF_ADDR = 0x09
MSG_INTERVAL = 0.01 # 10 ms
NUM_OF_TOF = 3
TOF_THRESHOLD = 200

bus = smbus2.SMBus(1)

pygame.init()
pygame.joystick.init()
joystick = pygame.joystick.Joystick(0)
joystick.init()

def send_msg_motor(left, right):
    # force left and right into 8 bit
    l = 0xFF & left
    r = 0xFF & right
    try:
        if (read_tof()[1] < TOF_THRESHOLD): ## NEED TO CHECK WHICH TOF IS WHICH
            l = 0 if (l <= 127) else l
            r = 0 if (r <= 127) else r
        checksum = l ^ r
        msg = smbus2.i2c_msg.write(MOTOR_ADDR, [l, r, checksum])
        bus.i2c_rdwr(msg)
    except:
        print("msg send fail")

def read_tof():
    try:
        msg = smbus2.i2c_msg.read(TOF_ADDR, NUM_OF_TOF + 1)
        bus.i2c_rdwr(msg)
        data = list(msg)
        readings = data[:NUM_OF_TOF]
        checksum = data[-1]

        if readings[0] ^ readings[1] ^ readings[2] != checksum:
            print("tof checksum fail")
            return None
        # convert to mm (r is in mm/16)
        return [r * 16 for r in readings]
    except:
        print("tof read fail")
        return None

def getInput():
    deadzone = 0.08

    pygame.event.pump()
    left_y = joystick.get_axis(1)
    right_x = joystick.get_axis(3)

    if abs(left_y) < deadzone: left_y = 0
    if abs(right_x) < deadzone: right_x = 0

    if (not right_x): # if only left stick, just move forward/back
        # y axis flipped
        left = -left_y
        right = left
    elif (not left_y): # if only right stick, turn in place
        left = right_x
        right = -right_x
    else:                   # else, handle both

        # TODO: might need to make minimum track speed not 0 (have a minimum track speed > 0)

        if (right_x < 0):   # turning left
            right = -left_y
            left = right * (1 - abs(right_x))
        else:               # turning right
            left = -left_y
            right = left * (1 - abs(right_x))
    
    return int(left*127),int(right*127)

print("Ctrl+C to quit")
last_msg = time.time()
try:
    while (True):
        # ping
        if time.time() - last_msg > MSG_INTERVAL:
            left,right=getInput()
            send_msg_motor(left,right)
            last_msg = time.time()
except KeyboardInterrupt:
    send_msg_motor(0,0)
    pygame.quit()