int button1Pin = 2;
int button10Pin = 3;
int number = 0;

int lastState1 = LOW;
int lastState10 = LOW;

void setup() {
  // Using INPUT_PULLUP is safer if you don't have external resistors.
  // Note: This inverts logic (LOW = pressed), but for this fix, 
  // I'll assume you have external resistors and stay with INPUT.
  pinMode(button1Pin, INPUT);
  pinMode(button10Pin, INPUT);
  
  Serial.begin(9600);
  Serial.print("Initial Value: ");
  Serial.println(number);
}

void loop() {
  // 1. Read current states
  int currentState1 = digitalRead(button1Pin);
  int currentState10 = digitalRead(button10Pin);

  bool changed = false;

  // 2. Check for Button 1 (Rising Edge)
  if (currentState1 == HIGH && lastState1 == LOW) {
    number += 1;
    changed = true;
    delay(50); // Simple debouncing
  }

  // 3. Check for Button 10 (Rising Edge)
  if (currentState10 == HIGH && lastState10 == LOW) {
    number += 10;
    changed = true;
    delay(50); // Simple debouncing
  }

  // 4. Only print if something happened
  if (changed) {
    Serial.print("Number updated: ");
    Serial.println(number);
  }

  // 5. Save states for next loop
  lastState1 = currentState1;
  lastState10 = currentState10;
}