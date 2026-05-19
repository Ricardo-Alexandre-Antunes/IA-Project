// -------------- LIBRARIES --------------
#include "FastLED.h"

// -------------- CONSTANTS --------------

// Operations Consts
#define ADD 0;
#define SUBTRACT 1;
#define MULTIPLY 2;
#define DIVIDE 3;

// Difficulty Consts
#define EASY 0;
#define MEDIUM 1;
#define HARD 2;

// Game State Consts
#define SETUP 0;
#define GAME 1;
#define RESULTS 2;

// Pin Consts
#define OPERATIONS_LED 2;
#define LEDS_STRIP 3;
#define EMPTY_PIN_4 4;
#define EMPTY_PIN_5 5;
#define OPERATION_DISPLAY 6;
#define CENTRAL_BUTTON 7;
#define EASY_BUTTON 8; // Shift Register
#define MEDIUM_BUTTON 9;
#define HARD_BUTTON 10;
#define latchPin 11; // Shift Register
#define dataPin 12; // Shift Register
#define clockPin 13;


// Shift Register Pins
#define SHIFT_PIN_0 0;
#define SHIFT_PIN_1 1;
#define SHIFT_PIN_2 2;
#define SHIFT_PIN_3 3;
#define SHIFT_PIN_4 4;
#define SHIFT_PIN_5 5;
#define SHIFT_PIN_6 6;
#define SHIFT_PIN_7 7;
#define SHIFT_PIN_15 15;

// Enderecable LEDS
#define NUM_LEDS 8

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
  setDifficulty(EASY);

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
  shiftOut(dataPin, clockPin, LSBFIRST, shift_pins);
  digitalWrite(latchPin, HIGH);
}

// By default the value is LOW, use this method to set value to HIGH
void setShiftRegisterPinHIGH(int pin) {
  bitSet(shift_pins, pin);
}

// SETUP


// GAMEPLAY

// Method that displays a number in a certain display
void displayNumber(int displayID, int number) {
  if (number > 9999 || number < 0) return -1; // cannot show number outside of range

}

// Method that generates a new operation and displays it
void newOperation() {
  int operation = random(4);
  int first_number;
  int second_number;
  switch (currentDifficulty){
    case EASY:
      switch (operation){
        case ADD:
          first_number = random(100);
          second_number = random(100);
        case SUBTRACT:
          first_number = random(100);
          second_number = random(first_number + 1);
        case MULTIPLY:
          first_number = random(9) + 2;
          second_number = random(9) + 2;
        case DIVIDE:
          do {
            first_number = random(100);
            second_number = random(9) + 1;
          } while (first_number % second_number != 0);
      }
    case MEDIUM:
      switch (operation){
        case ADD:
          first_number = random(1000);
          second_number = random(1000);
        case SUBTRACT:
          first_number = random(1000);
          second_number = random(first_number + 1);
        case MULTIPLY:
          do {
            first_number = random(998) + 2;
            second_number = random(998) + 2;
          } while (first_number * second_number > 999);
        case DIVIDE:
          do {
            first_number = random(1000);
            second_number = random(99) + 1;
          } while (first_number % second_number != 0);
      }
    case HARD:
      switch (operation){
        case ADD:
          first_number = random(10000);
          second_number = random(9999 - first_number);
        case SUBTRACT:
          first_number = random(10000);
          second_number = random(first_number + 1);
        case MULTIPLY:
          do {
            first_number = random(9998) + 2;
            second_number = random(9998) + 2;
          } while (first_number * second_number > 9999);
        case DIVIDE:
          do {
            first_number = random(10000);
            second_number = random(9999) + 1;
          } while (first_number % second_number != 0);
      }
  }
}

// Method that calculates scores based on the current operation and the values on each player's display
void updateScores() {

}

// RESULTS


// -------------- END HELPER METHODS --------------

// -------------- LOOPS --------------

// Loop relevant for things happening during the setup of the game
void setupLoop() {
  // Display current operations in operation leds
  Difficulty difficulty = getDifficulty(currentDifficulty);

  // Display current dfficulty index in operation
  displayNumber(OPERATION_DISPLAY, currentDifficulty);

  // Read difficulty buttons to change difficulty
  if (digitalRead(EASY_BUTTON)) {
    currentDifficulty = EASY;
  }

  if (digitalRead(MEDIUM_BUTTON)) {
    currentDifficulty = MEDIUM;
  }

  if (digitalRead(HARD_BUTTON)) {
    currentDifficulty = HARD;
  }


  // Read central button value, if 1 go over to game mode
  if (digitalRead(CENTRAL_BUTTON) == HIGH) {
    currentGameState = GAME;
  }

}

// Loop relevant for things happening during the game itself
void gameLoop() {

}

// Loop relevant for things happening after the game is done
void resultsLoop() {

  // Flicker scoring LED's to showcase winning

  // Store current score between the 2 players (could this be stored forever?)

  // 

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


