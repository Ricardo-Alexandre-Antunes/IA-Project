// -------------- CONSTANTS --------------

// Operations Consts
const int ADD = 0;
const int SUBTRACT = 1;
const int MULTIPLY = 2;
const int DIVIDE = 3;

// Difficulty Consts
const int EASY = 0;
const int MEDIUM = 1;
const int HARD = 2;

// Game State Consts
const int SETUP = 0;
const int GAME = 1;
const int RESULTS = 2;

// Pin Consts

// Shift Register Pins
const int latchPin = 8;
const int clockPin = 12;
const int dataPin = 11;

byte shift_pins = 0;

// -------------- END CONSTANTS --------------

// -------------- CLASSES --------------

class Difficulty {
  private:
    int operationsAllowed[4]; // 0 if operation is not allowed, 1 if operation is allowed

  public:

    // Default constructor
    Difficulty() {
      for (int i = 0; i < 4; i++) {
        operationsAllowed[i] = 0;
      }
    }

    // Main constructor
    Difficulty(int operations[4]) {
      for (int i = 0; i < 4; i++) {
        operationsAllowed[i] = operations[i];
      }
    }

    String allowedOperations() {

      String result = "";

      result += "ADD: ";
      result += String(operationsAllowed[ADD]);

      result += " | SUBTRACT: ";
      result += String(operationsAllowed[SUBTRACT]);

      result += " | MULTIPLY: ";
      result += String(operationsAllowed[MULTIPLY]);

      result += " | DIVIDE: ";
      result += String(operationsAllowed[DIVIDE]);

      return result;
    }

    bool isOperationAllowed(int operation) {
      if (operation < 0 || operation > 3) {
        return 0;
      }
      return operationsAllowed[operation];
    }
};

// -------------- END CLASSES --------------

// -------------- VARIABLES --------------

Difficulty difficultyList[3];

// Current States
int currentGameState = 0;
int currentDifficulty = 0;

// -------------- END VARIABLES --------------

// -------------- GETTERS / SETTERS --------------

// Getters / Setters for states
void setDifficulty(int difficulty) {
  currentDifficulty = difficulty;
}

Difficulty getDifficulty(int difficulty) {
  return difficultyList[difficulty];
}

void nextGameState() {
  currentGameState++;
  currentGameState = currentGameState % 3;
}

// -------------- END GETTERS / SETTERS --------------

// -------------- SETUP --------------

// Main Setup (Do not remove!!!)
void setup() {

  int easyOps[4] = {1, 0, 0, 0}; // Only addition allowed
  int mediumOps[4] = {1, 1, 0, 0}; // Addition and Subtraction allowed
  int hardOps[4] = {1, 1, 1, 1}; // Addition, Subtraction, Multiplication and Division allowed

  difficultyList[0] = Difficulty(easyOps);
  difficultyList[1] = Difficulty(mediumOps);
  difficultyList[2] = Difficulty(hardOps);

  setupPins();

  Serial.begin(9600);
}

// Setup all pins (INPUT / OUTPUT)
void setupPins() {

  // Shift Register Pins
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

// -------------- END SETUP --------------

// -------------- HELPER METHODS --------------

// Shift Register

void updateShiftRegister() {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, leds);
  digitalWrite(latchPin, HIGH);
}

// By default the value is LOW, use this method to set value to HIGH
void setShiftRegisterPinHIGH(int pin) {
  bitSet(shift_pins, pin);
}

// SETUP


// GAMEPLAY

// Method that displays a number in a certain display
void displayNumber(int displayID) {

}

// Method that generates a new operation and displays it
void newOperation() {

}

// Method that calculates scores based on the current operation and the values on each player's display
void updateScores() {

}

// RESULTS


// -------------- END HELPER METHODS --------------

// -------------- LOOPS --------------

// Loop relevant for things happening during the setup of the game
void setupLoop() {

}

// Loop relevant for things happening during the game itself
void gameLoop() {

}

// Loop relevant for things happening after the game is done
void resultsLoop() {

}

// Main Loop (Do not remove!!!)
void loop() {
  switch(currentGameState) {
    case SETUP:
      setupLoop();
      break;
    case GAME:
      gameLoop();
      break;
    case RESULTS:
      resultsLoop();
      break;
  }
}

// -------------- END LOOPS --------------


