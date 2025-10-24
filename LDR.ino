// Pin definitions
const int ldrPin = A0;      // LDR connected to analog pin A0
const int ledPin = 13;       // LED connected to digital pin 9

// Threshold for darkness (adjust as needed)
const int lightThreshold = 500;

void setup() {
  pinMode(ledPin, OUTPUT);   // Set LED pin as output
  Serial.begin(9600);        // Start serial monitor for debugging
}

void loop() {
  int ldrValue = analogRead(ldrPin);   // Read LDR value (0 - 1023)

  Serial.print("LDR Value: ");
  Serial.println(ldrValue);            // Print value to Serial Monitor

  // If it is dark, turn on LED
  if (ldrValue < lightThreshold) {
    digitalWrite(ledPin, HIGH);  // LED ON
  } else {
    digitalWrite(ledPin, LOW);   // LED OFF
  }

  delay(200); // Wait a bit before next reading
}
