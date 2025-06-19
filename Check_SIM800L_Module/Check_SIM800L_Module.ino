#include <HardwareSerial.h>

HardwareSerial SIM800Serial(1); // Use Serial1 on ESP32 (GPIO 16 = RX, GPIO 17 = TX)

void setup() {
  // Start serial communication with SIM800L
  SIM800Serial.begin(9600, SERIAL_8N1, 16, 17); // baud rate, protocol, RX pin, TX pin

  Serial.begin(115200); // Start serial communication with the computer

  Serial.println("Starting SIM800L test...");

  delay(1000);

  // Check if SIM800L module is responding
  SIM800Serial.println("AT");
  delay(1000);
  while (SIM800Serial.available()) {
    Serial.write(SIM800Serial.read());
  }
  Serial.println("AT command sent.");

  // Configure SMS text mode
  SIM800Serial.println("AT+CMGF=1");
  delay(1000);
  while (SIM800Serial.available()) {
    Serial.write(SIM800Serial.read());
  }
  Serial.println("SMS text mode set.");
}

void loop() {

  // Example: Make a call
  makeCall("+94719980732");

  delay(5000); // Wait 5 seconds

// Example: Send an SMS
  sendSMS("+94719980732", "Hello from SIM800L!");



  delay(30000); // Wait 30 seconds before repeating
}

void sendSMS(String number, String message) {
  SIM800Serial.print("AT+CMGS=\"");
  SIM800Serial.print(number);
  SIM800Serial.println("\"");

  delay(1000);

  SIM800Serial.print(message);
  SIM800Serial.println((char)26); // End AT command with Ctrl+Z character

  Serial.println("SMS sent.");
}

void makeCall(String number) {
  SIM800Serial.print("ATD");
  SIM800Serial.print(number);
  SIM800Serial.println(";");

  Serial.println("Calling...");
}
