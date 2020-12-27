import serial

ser = "/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0"
ser = serial.Serial(ser, 9600)
ser.flushInput()
ser.flushOutput()

while True:
	print(ser.read().hex(), end="", flush=True)
