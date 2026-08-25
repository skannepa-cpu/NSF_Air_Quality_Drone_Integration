import serial
import time

# IMPORTANT: Change 'COM10' to whatever port your Arduino is actually using
# You can check this in the Arduino IDE under Tools > Port
arduino_port = 'COM10' 

try:
    # Open the serial connection
    ser = serial.Serial(arduino_port, 9600, timeout=1)
    
    # The Arduino restarts when a serial connection opens, so we wait 2 seconds for it to boot
    time.sleep(2) 

    # Grab the exact current Unix timestamp
    current_time = int(time.time())

    # Fire the timestamp to the Arduino
    ser.write(f"{current_time}\n".encode())
    
    # Read the confirmation message back from the Arduino
    response = ser.readline().decode().strip()

    print(f"Time successfully sent: {current_time}")
    print(f"Arduino says: {response}")
    
    ser.close()

except PermissionError:
    print("Error: Access denied. Close the Arduino IDE Serial Monitor and try again.")
except serial.SerialException as e:
    print(f"Error connecting to {arduino_port}: {e}")

print("Hello! The script successfully reached the end.")