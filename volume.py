import serial
import subprocess

ser = serial.Serial('/dev/cu.usbmodem145101')
ser.flushInput()
vol_bytes = 0

wall_dist = 850
min_dist = 150
max_dist = 800
min_vol = 0
max_vol = 100
stored_vol = 0

while True:
    try:
        ser_bytes = ser.readline()
        cm = float(ser_bytes) - 75
        if cm <=0:
            cm = 0
        print(cm)
        if cm > wall_dist:
            vol_bytes = stored_vol
        elif cm <= min_dist:
            vol_bytes = 100 - max_vol
        elif cm >= max_dist:
            vol_bytes = 100 - min_vol
        else:
            vol_bytes = 100 - (((min_vol-max_vol)/(max_dist-min_dist))*(cm-min_dist)+max_vol)
        subprocess.call(["osascript", "-e", "set volume output volume " + str(vol_bytes)])
        stored_vol = vol_bytes
    except:
        print("Keyboard Interrupt")
        subprocess.call(["osascript", "-e", "set volume output volume 0"])
        break
