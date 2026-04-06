import pygame
import RPi.GPIO as GPIO

LEFT_FWD, LEFT_BWD = 17, 18
RIGHT_FWD, RIGHT_BWD = 22, 23

GPIO.setmode(GPIO.BCM)
for pin in [LEFT_FWD, LEFT_BWD, RIGHT_FWD, RIGHT_BWD]:
    GPIO.setup(GPIO.OUT)

pygame.init()
pygame.joystick.init()
print(f"Number of joysticks detected: {pygame.joystick.get_count()}")

joystick = pygame.joystick.Joystick(0)
joystick.init()

def set_output(fwd_pin, bwd_pin, value):
    if value > 0.1:
        GPIO.output(fwd_pin, True)
        GPIO.output(bwd_pin, False)
    elif value < -0.1:
        GPIO.output(fwd_pin, False)
        GPIO.output(bwd_pin, True)
    else:
        GPIO.output(fwd_pin, False)
        GPIO.output(bwd_pin, False)

try:
    while True:
        pygame.event.pump()
        # double check axes
        left_y = joystick.get_axis(1)
        right_y = joystick.get_axis(4)
        # y-axis is inverted
        set_output(LEFT_FWD, LEFT_BWD, -left_y)
        set_output(RIGHT_FWD, RIGHT_BWD, -right_y)

finally:
    GPIO.cleanup()