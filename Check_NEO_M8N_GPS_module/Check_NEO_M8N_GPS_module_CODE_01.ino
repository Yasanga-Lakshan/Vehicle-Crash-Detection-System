#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Define the serial port for GPS module
#define SerialGPS Serial2

// Create a TinyGPS++ object
TinyGPSPlus gps;

void setup() {
  // Start serial communication with GPS module
  Serial.begin(115200);   // Serial monitor
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17); // GPS module (baud rate, config, RX pin, TX pin)

  Serial.println("GPS module started!");
}

void loop() {
  // Read data from GPS module
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // Display GPS data if valid
  if (gps.location.isValid()) {
    Serial.print("Latitude: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", Longitude: ");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("Waiting for GPS data...");
  }

  // Recommended delay to wait for the next data update
  delay(1000);
}
