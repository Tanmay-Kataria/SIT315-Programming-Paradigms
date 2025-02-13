// Pins Define
const int buttonPin = 2;  // Interrupt-enabled pin for button
const int ledPin = 13;    // Onboard LED pin

// Global state: paused flag (volatile because it's modified in the ISR)
volatile bool paused = false;

// Counter for numbers
unsigned long counter = 1;

// Interrupt Service Routine (ISR)
// This function toggles the pause state and sets the LED accordingly.
void togglePause() {
  paused = !paused;
  if (paused) {
    digitalWrite(ledPin, HIGH);   // Turn LED on when paused
  } else {
    digitalWrite(ledPin, LOW);    // Turn LED off when resumed
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Setup LED pin as output
  pinMode(ledPin, OUTPUT);

  // Setup button pin as input with internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);

  // Attach interrupt to the button pin; trigger on FALLING edge (button press)
  attachInterrupt(digitalPinToInterrupt(buttonPin), togglePause, FALLING);

  Serial.println("System Initialized. Printing numbers...");
}

void loop() {
  // If not paused, print the counter value and increment it
  if (!paused) {
    Serial.println(counter);
    counter++;
  } else {
    // Optionally, print a paused message (this will print continuously while paused)
    Serial.println("Paused...");
  }
  // Delay to make the output readable (1 second interval)
  delay(1000);
}
