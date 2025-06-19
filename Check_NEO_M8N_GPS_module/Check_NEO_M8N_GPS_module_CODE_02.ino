#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Define the serial port for GPS module
#define SerialGPS Serial2

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Global variables to store the current GPS location
double current_lat = 0.0;
double current_long = 0.0;

// Timing variables
unsigned long time1 = 0;
unsigned long time2 = 0;

void setup() {
  // Start serial communication with GPS module
  Serial.begin(115200);   // Serial monitor
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17); // GPS module (baud rate, config, RX pin, TX pin)

  Serial.println("GPS module started!");
}

void get_GPS() {
  // Read data from GPS module
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // Update global variables if GPS data is valid
  if (gps.location.isValid()) {
    current_lat = gps.location.lat();
    current_long = gps.location.lng();
  }

  // Print the current location to the serial monitor
  if (gps.location.isValid()) {
    Serial.print("Latitude: ");
    Serial.print(current_lat, 6);
    Serial.print(", Longitude: ");
    Serial.println(current_long, 6);
  } else {
    Serial.println("Waiting for GPS data...");
  }
}

void Impact() {
  // Impact function code
  // Place your existing impact detection logic here
  Serial.println("Impact function called.");
}

void loop() {
  unsigned long current_time = micros();

  // Call get_GPS function every 2ms
  if (current_time - time1 > 1999) {
    time1 = current_time; // Update the time for the next interval
    get_GPS();
  }

  // Call Impact function every 2ms
  if (current_time - time2 > 1999) {
    time2 = current_time; // Update the time for the next interval
    Impact();
  }

  // Other code that needs to run continuously can go here
}
