import serial

ser = serial.Serial('/dev/cu.usbmodem145101')
ser.flushInput()
output_file = None
START = True
write_counter = 0
close_counter = 0

while True:
    try:
        ser_bytes = ser.readline()
        data = float(ser_bytes)
        if START:
            output_file = open("output.txt", "a+")
        if write_counter % 100 == 0:
            output_file.write(str(data) + "\n")
        write_counter += 1
        START = False
        if close_counter % 1000 == 0:
            output_file.close()
            START = True
        close_counter += 1

    except:
        print("Keyboard Interrupt")
        break
