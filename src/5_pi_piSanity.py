import smbus2
import time
import RPi.GPIO as GPIO

bus = smbus2.SMBus(1)
PICO_ADDR = 0x08

GREEN_LED = 17 # CHANGE
RED_LED = 16 # CHANGE

GPIO.setmode(GPIO.BCM)
GPIO.setup(GREEN_LED, GPIO.OUT)
GPIO.setup(RED_LED, GPIO.OUT)

def i2ctest():
    msg = smbus2.i2c_msg.write(PICO_ADDR, [0x01, 0XFF])
    try:
        bus.i2c_rdwr(msg)
        GPIO.output(GREEN_LED, GPIO.HIGH)
        GPIO.output(RED_LED, GPIO.LOW)
    except:
        GPIO.output(RED_LED, GPIO.HIGH)
        GPIO.output(GREEN_LED, GPIO.LOW)

while True:
    try:
        i2ctest()
        time.sleep(0.5)
    except KeyboardInterrupt:
        GPIO.cleanup() # IMPORTANT: forgetting can leave pins in weird state
        break        