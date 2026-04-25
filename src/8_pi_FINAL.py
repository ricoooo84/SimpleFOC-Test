import smbus2
import time
import pygame
import cv2
from flask import Flask, Response
import threading

# THINGS TO CHANGE
# make this script auto run when pi turns on
# change tof variable names because "left/mid/right" were randomly given

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

app = Flask(__name__)
cap = cv2.VideoCapture(0)
latest_data = {"tof": [0,0,0], "left": 0, "right": 0}

def send_msg_motor(left, right):
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

def generate():
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        _, buffer = cv2.imencode('.jpg', frame)
        frame_bytes = buffer.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')

@app.route('/video')
def video():
    return Response(generate(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/data')
def data():
    return latest_data

@app.route('/')
def index():
    return '''
    <html><body>
    <h2>Swiss Army Bot</h2>
    <p>TOF: <span id="tof"></span></p>
    <p>Motors: <span id="motors"></span></p>
    <img src="/video">
    <script>
        setInterval(() => {
            fetch('/data').then(r=>r.json()).then(d => {
                document.getElementById('tof').innerText = d.tof;
                document.getElementById('motors').innerText = `L: ${d.left} R: ${d.right}`;
            });
        }, 100);
    </script>
    </body></html>
    '''

thread = threading.Thread(target=lambda: app.run(host='0.0.0.0', port=5000))
thread.daemon = True
thread.start()

tof = None
left, right = 0, 0

print("Ctrl+C to quit")
last_msg = time.time()
try:
    while True:
        if time.time() - last_msg > MSG_INTERVAL:
            tof = read_tof()
            left, right = getInput()
            send_msg_motor(left, right)
            latest_data["tof"] = tof if tof else [0,0,0]
            latest_data["left"] = left
            latest_data["right"] = right
            last_msg = time.time()
except KeyboardInterrupt:
    send_msg_motor(0, 0)
    pygame.quit()