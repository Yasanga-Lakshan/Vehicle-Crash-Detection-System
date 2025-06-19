🚗 Vehicle Crash Detection System

Every day, hundreds of vehicle accidents occur worldwide. Often, lives could be saved if emergency responders arrived just a few minutes earlier. What if we could reduce that response time—even by five minutes? That’s the goal of this project: to build a Vehicle Crash Detection System that can automatically detect serious crashes and immediately alert emergency contacts with real-time location data.


🔧 Project Overview

The Vehicle Crash Detection System is an embedded solution designed to detect vehicular accidents in real-time using motion sensors and to instantly notify a pre-saved emergency contact with location details via SMS and call. It also includes manual alerting (SOS) and web-based monitoring features to enhance safety and responsiveness.


🌟 Features

1. Automatic Crash Detection 🚨 
- Detects sudden impact or acceleration changes using an **MPU6050** accelerometer and gyroscope sensor.
- Upon detection:
  - Displays "CRASH DETECTED" on the front panel.
  - Activates a buzzer for 30 seconds to alert the driver.
  - Sends an SMS with real-time GPS coordinates and a Google Maps link to an emergency contact.
  - Makes an automatic call to the emergency contact number.

2. False Alarm Cancellation ❌ 
- A cancel button allows the driver to deactivate the alert sequence within 30 seconds.
- If pressed, it sends an SMS saying "No Problem", and the call alert is canceled.

3. Manual SOS Alert 🆘 
- A dedicated SOS button enables the driver to manually trigger an emergency alert.
- Sends GPS coordinates and Google Maps link to the emergency contact.
- Initiates a voice call to ensure the contact is notified.

4. Live Location Tracking 📍 
- Allows real-time vehicle location tracking through a web interface.
- Potential use cases include stolen vehicle recovery and fleet management.

5. Web Interface for Monitoring 🌐 
- Hosted on the ESP32’s built-in Wi-Fi.
- Enables users or vehicle rental companies to:
  - Monitor vehicle location.
  - View system status in real time.

6. Emergency Contact Management 🔧 
- Web-based interface allows users to log in and update their emergency contact details.
- Also hosted directly on the ESP32 for accessibility without external servers.


🛠 Technologies Used

- **ESP32** – Microcontroller with Wi-Fi capability
- **MPU6050** – Accelerometer + Gyroscope sensor for crash detection
- **NEO M8N GPS** – High-precision GPS module
- **SIM800L** – GSM module for SMS and call functionality
- **Buzzer and Display** – For local alerts and status display
- **Web Interface** – Built using ESP32’s web server, Google Maps API (for future live tracking)


📦 Project Status

- ✅ Crash detection and alerting via SMS and call
- ✅ Manual SOS feature
- ✅ Crash alert cancellation system
- ⚙️ Live tracking interface – *Not Fully Developed*
- ⚙️ Web portal for contact management – *Prototype version working*



💡 Potential Applications

- Personal and commercial vehicles
- Rental car services
- Logistics and delivery fleets
- Elderly or teen driver monitoring systems

---