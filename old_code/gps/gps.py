import serial, pynmea2

# ls /dev/tty.*
# Replace with your actual device, e.g. '/dev/tty.usbserial-1420'
gps = serial.Serial('/dev/tty.usbserial-10', 9600, timeout=1)

while True:
    line = gps.readline().decode(errors="ignore").strip()
    if line.startswith("$GNRMC") or line.startswith("$GNGGA"):
        try:
            msg = pynmea2.parse(line)
            print(f"Lat: {msg.latitude}, Lon: {msg.longitude}")
        except pynmea2.ParseError:
            continue
