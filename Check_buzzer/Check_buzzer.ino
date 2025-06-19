const int buttonPin = 2;    // Push button connected to digital pin 2
const int buzzerPin = 9;    // Buzzer connected to digital pin 9

int buttonState = 0;        // Variable to hold the push button state

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // Set the push button pin as input with internal pull-up resistor
  pinMode(buzzerPin, OUTPUT);       // Set the buzzer pin as output
}

void loop() {
  buttonState = digitalRead(buttonPin);  // Read the state of the push button

  if (buttonState == LOW) {              // If the push button is pressed (active low)
    tone(buzzerPin, 10000);               // Generate a 1kHz tone on the buzzer pin
  } else {
    noTone(buzzerPin);                   // Stop generating the tone
  }
}
