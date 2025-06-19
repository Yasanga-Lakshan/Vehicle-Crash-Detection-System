#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <MPU6050.h> // Include the MPU6050 library
#include <WiFi.h>
#include <WebServer.h>

// SSID and Password for the ESP32 Access Point
const char* ssid = "ESP32-AP";
const char* password = "12345678";

// Web server object on port 80
WebServer server(80);

// HTML content to be served by the ESP32
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>FENIX Vehicle Crash Detection System</title>
  <style>
    body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 0; }
    .container { max-width: 500px; margin: 50px auto; padding: 20px; background-color: #fff; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }
    h1 { color: #333; }
    input[type="text"], select { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 4px; }
    input[type="submit"] { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }
    input[type="submit"]:hover { background-color: #45a049; }
  </style>
</head>
<body>
  <div class="container">
    <h1>FENIX Vehicle Crash Detection System</h1>
    <form action="/submit" method="POST" id="dataForm">
      <label for="emergencyPhone">Emergency Contact Person's Number:</label>
      <input type="text" id="emergencyPhone" name="emergencyPhone" value="+94" pattern="\+94[0-9]{9}" required>
      <label for="vehicleType">Vehicle Type:</label>
      <select id="vehicleType" name="vehicleType">
        <option value="10">Toyota Aqua</option>
        <option value="12">Nissan Leaf</option>
        <option value="13">Honda Fit</option>
        <option value="9">Mitsubishi</option>
        <option value="8">Suzuki Swift</option>
        <option value="13">Suzuki Wagon R</option>
        <option value="18">Honda Vezel</option>
        <option value="8">Suzuki Alto</option>
        <option value="14">Honda CR-V</option>
        <option value="16">Nissan X-Trail</option>
        <option value="15">Toyota Prius</option>
        <option value="18">Toyota Land Cruiser Prado</option>
      </select>
      <input type="submit" value="Submit">
    </form>
    <div id="response"></div>
  </div>
  <script>
    document.getElementById('dataForm').addEventListener('submit', function(event) {
      event.preventDefault();
      var emergencyPhone = document.getElementById('emergencyPhone').value;
      var vehicleType = document.getElementById('vehicleType').value;
      var xhr = new XMLHttpRequest();
      xhr.open('POST', '/submit', true);
      xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
      xhr.onreadystatechange = function() {
        if (xhr.readyState === 4 && xhr.status === 200) {
          document.getElementById('response').innerHTML = '<p>' + xhr.responseText + '</p>';
        }
      };
      xhr.send('emergencyPhone=' + encodeURIComponent(emergencyPhone) + '&vehicleType=' + encodeURIComponent(vehicleType));
    });
  </script>
</body>
</html>
)rawliteral";

// Initialize the MPU6050
MPU6050 mpu;

// Initialize I2C LCD with address 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Emergency contact number
String EMERGENCY_PHONE = "ENTER_EMERGENCY_PHONE_NUMBER"; 

// Define serial interfaces
HardwareSerial sim800(1);  // UART1
HardwareSerial neogps(2);  // UART2

TinyGPSPlus gps;

String sms_status, sender_number, received_date, msg;
String latitude, longitude;

#define BUZZER 12
#define BUTTON 13
#define MANUAL_BUTTON 14 // Define the pin for the manual emergency button

byte updateflag;

int xaxis = 0, yaxis = 0, zaxis = 0;
int deltx = 0, delty = 0, deltz = 0;
int vibration = 2, devibrate = 75;
int magnitude = 0;
int sensitivity = 10;
boolean impact_detected = false;
boolean manual_triggered = false; // Flag to track manual emergency trigger
// Used to run impact routine every 2mS.
unsigned long time1;
unsigned long impact_time;
unsigned long alert_delay = 30000; // 30 seconds

/*****************************************************************************************
 * setup() function
 *****************************************************************************************/
void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);

  // Set up the ESP32 as an access point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Created");

  // Print the IP address of the ESP32 in AP mode
  IPAddress IP = WiFi.softAPIP();
  Serial.print("IP Address: ");
  Serial.println(IP);

  // Define the root URL ("/") to serve the HTML page
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", htmlPage);
  });

  // Define the "/submit" URL to handle form submissions
  server.on("/submit", HTTP_POST, []() {
    // Retrieve the data from the form
    EMERGENCY_PHONE = server.arg("emergencyPhone");
    sensitivity = server.arg("vehicleType").toInt();
    // Print the current emergency phone number and sensitivity to the Serial Monitor
    Serial.println("Emergency Phone: " + EMERGENCY_PHONE);
    Serial.println("Sensitivity: " + String(sensitivity));
    // Send a response back to the client
    server.send(200, "text/html", "Successfully Submitted<br><a href='/'>Go back</a>");
  });

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");

  //Initialise the SIm800 Module and Gps module
  sim800.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  neogps.begin(9600, SERIAL_8N1, 18, 19); // RX=18, TX=19

  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(MANUAL_BUTTON, INPUT_PULLUP); // Initialize the manual emergency button

  // Initialize LCD screen
  //lcd.init(); // Use init() instead of begin()
  // Turn on the backlight
  lcd.backlight();
  lcd.clear();

  sms_status = "";
  sender_number = "";
  received_date = "";
  msg = "";

  sim800.println("AT"); // Check GSM Module
  delay(1000);
  sim800.println("ATE1"); // Echo ON
  delay(1000);
  sim800.println("AT+CPIN?"); // Check SIM ready
  delay(1000);
  sim800.println("AT+CMGF=1"); // SMS text mode
  delay(1000);
  sim800.println("AT+CNMI=1,1,0,0,0"); // Set SMS received format
  delay(1000);

  // Initialize the MPU6050
  Wire.begin();
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }

  time1 = micros(); 
}

/*****************************************************************************************
 * loop() function
 *****************************************************************************************/
void loop() {
  // Handle incoming client requests
  server.handleClient();

  // Print the current emergency phone number and sensitivity to the Serial Monitor
  Serial.println("Emergency Phone: " + EMERGENCY_PHONE);
  Serial.println("Sensitivity: " + String(sensitivity));

  // Call impact routine every 2mS
  if (micros() - time1 > 1999) Impact();

  // Update LCD display based on impact detection
  if (updateflag > 0) {
    updateflag = 0;
    Serial.println("Updateflag is set. Impact detected!!");
    Serial.print("Magnitude: "); Serial.println(magnitude);

    getGps();
    digitalWrite(BUZZER, HIGH);
    impact_detected = true;
    impact_time = millis();

    lcd.clear();
    lcd.setCursor(0, 0); // col=0 row=0
    lcd.print("Crash Detected");
    lcd.setCursor(0, 1); // col=0 row=1
    lcd.print("Magnitude:" + String(magnitude));
  } else if (!manual_triggered && !impact_detected) {
    // No impact detected, display "Vehicle is Good" message
    Serial.println("No impact detected. Vehicle is Good.");
    lcd.clear();
    lcd.setCursor(0, 0); // col=0 row=0
    lcd.print("Vehicle is Good");
  }

  // Handle impact alert and reset
  if (impact_detected || manual_triggered) {
    if (millis() - impact_time >= alert_delay) {
      digitalWrite(BUZZER, LOW);
      makeCall();
      delay(1000);
      sendAlert();
      impact_detected = false;
      manual_triggered = false; // Reset manual trigger flag
      impact_time = 0;
    }
  }

  // Reset impact detection or manual trigger if button is pressed
  if (digitalRead(BUTTON) == LOW) {
    delay(200); // debounce delay
    digitalWrite(BUZZER, LOW);
    impact_detected = false;
    manual_triggered = false; // Reset manual trigger flag
    impact_time = 0;
    Serial.println("Operation canceled.");
    lcd.clear();
    lcd.setCursor(0, 0); // col=0 row=0
    lcd.print("Operation Canceled");
    delay(2000); // Display the message for a short time
  }

  // Manual emergency button
  if (digitalRead(MANUAL_BUTTON) == LOW && !manual_triggered) {
    Serial.println("Manual emergency button pressed!");
    getGps();
    digitalWrite(BUZZER, HIGH);
    manual_triggered = true;
    impact_time = millis();

    lcd.clear();
    lcd.setCursor(0, 0); // col=0 row=0
    lcd.print("Manual Alert");
    lcd.setCursor(0, 1); // col=0 row=1
    lcd.print("Sending Alert");
  }

  // Process incoming SMS and commands
  while (sim800.available()) {
    parseData(sim800.readString());
  }

  // Forward serial input to SIM800 module
  while (Serial.available()) {
    sim800.println(Serial.readString());
  }
}

/*****************************************************************************************
 * Impact() function
 *****************************************************************************************/
void Impact() {
  time1 = micros(); // Resets time value

  // Store previous axis readings for comparison
  int oldx = xaxis;
  int oldy = yaxis;
  int oldz = zaxis;

  // Read new values from MPU6050
  xaxis = mpu.getAccelerationX() / 16384.0; // Convert raw values to g
  yaxis = mpu.getAccelerationY() / 16384.0;
  zaxis = mpu.getAccelerationZ() / 16384.0;
  
  // Loop counter prevents false triggering
  vibration--;
  if (vibration < 0) vibration = 0;
  
  if (vibration > 0) return;

  deltx = xaxis - oldx;
  delty = yaxis - oldy;
  deltz = zaxis - oldz;
  
  // Magnitude to calculate force of impact
  magnitude = sqrt(sq(deltx) + sq(delty) + sq(deltz));

  // Adjusted sensitivity threshold
  if (magnitude >= 1.5) { // Impact detected (adjust threshold as needed)
    updateflag = 1; // Set updateflag when impact is detected
    vibration = devibrate; // Reset anti-vibration counter
    Serial.println("Impact detected!!");
    Serial.print("Magnitude: "); Serial.println(magnitude);
  } else {
    magnitude = 0; // Reset magnitude of impact to 0
  }
}

/*****************************************************************************************
 * parseData() function
 *****************************************************************************************/
void parseData(String buff) {
  Serial.println(buff);

  unsigned int len, index;

  // Remove sent "AT Command" from the response string
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();

  if (buff != "OK") {
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index + 2);

    if (cmd == "+CMTI") {
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      sim800.println(temp);
    } else if (cmd == "+CMGR") {
      if (buff.indexOf(EMERGENCY_PHONE) > 1) {
        buff.toLowerCase();
        if (buff.indexOf("get gps") > 1) {
          getGps();
          String sms_data;
          sms_data = "GPS Location Data\r";
          sms_data += "http://maps.google.com/maps?q=loc:";
          sms_data += latitude + "," + longitude;
        
          sendSms(sms_data);
        }
      }
    }
  }
}

/*****************************************************************************************
 * getGps() Function
*****************************************************************************************/
void getGps() {
  // Can take up to 60 seconds
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
        break;
      }
    }
  }
  
  if (newData) { // If newData is true
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
    newData = false;
  } else {
    Serial.println("No GPS data is available");
    latitude = "";
    longitude = "";
  }

  Serial.print("Latitude= "); Serial.println(latitude);
  Serial.print("Longitude= "); Serial.println(longitude);
}

/*****************************************************************************************
* sendAlert() function
*****************************************************************************************/
void sendAlert() {
  String sms_data;
  sms_data = "Accident Alert!!\r";
  sms_data += "http://maps.google.com/maps?q=loc:";
  sms_data += latitude + "," + longitude;

  sendSms(sms_data);
}

/*****************************************************************************************
* makeCall() function
*****************************************************************************************/
void makeCall() {
  Serial.println("calling....");
  sim800.println("ATD" + EMERGENCY_PHONE + ";");
  delay(20000); // 20 sec delay
  sim800.println("ATH");
  delay(1000); // 1 sec delay
}

/*****************************************************************************************
 * sendSms() function
 *****************************************************************************************/
void sendSms(String text) {
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\"" + EMERGENCY_PHONE + "\"\r");
  delay(1000);
  sim800.print(text);
  delay(100);
  sim800.write(0x1A); // ASCII code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}

/*****************************************************************************************
 * SendAT() function
 *****************************************************************************************/
boolean SendAT(String at_command, String expected_answer, unsigned int timeout) {
  uint8_t x = 0;
  boolean answer = 0;
  String response;
  unsigned long previous;
    
  // Clean the input buffer
  while (sim800.available() > 0) sim800.read();

  sim800.println(at_command);
    
  x = 0;
  previous = millis();

  // This loop waits for the answer with time out
  do {
    // If there are data in the UART input buffer, reads it and checks for the answer
    if (sim800.available() != 0) {
      response += sim800.read();
      x++;
      // Check if the desired answer (OK) is in the response of the module
      if (response.indexOf(expected_answer) > 0) {
        answer = 1;
        break;
      }
    }
  } while ((answer == 0) && ((millis() - previous) < timeout));

  Serial.println(response);
  return answer;
}
