// -------------- LIBRARIES --------------
#include "FastLED.h"
#include <TM1637.h>

// -------------- CONSTANTS --------------
TM1637 dispNum1(6, 7); // Number 1 of equation
TM1637 dispNum2(6, 8); // Number 2 of equation
TM1637 dispA(6, 9);    // Player A entry
TM1637 dispB(6, 10);   // Player B entry


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

// Analog Pins
#define PA_X = A0;
#define PA_Y = A1;
#define PB_X = A2;
#define PB_Y = A3;

// Enderecable LEDS
#define NUM_LEDS 8
CRGB leds[NUM_LEDS];

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
int currentGameState = SETUP;
int currentDifficulty = EASY;

int targetAnswer = 0;
int playerA_digits[4] = {0, 0, 0, 0};
int playerB_digits[4] = {0, 0, 0, 0};
int cursorA = 3; // 0=Thousand, 1=Hundred, 2=Ten, 3=Unit
int cursorB = 3;

int playerAScore = 3;
int playerBScore = NUM_LEDS - 4;
const int playerAColor[3] = [255, 0, 0]; // GREEN
const int playerBColor[3] = [0, 255, 0]; // RED

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
  pinMode(CENTRAL_BUTTON, INPUT_PULLUP);

  // Init displays
  dispNum1.init();
  dispNum2.init();
  dispA.init();
  dispB.init();

  // Init brightness
  dispNum1.set(2);
  dispNum2.set(2);
  dispA.set(5);
  dispB.set(5);

  randomSeed(analogRead(A5));

  // LED Strip
  FastLED.addLeds<WS2812, LEDS_STRIP, RGB>(leds, NUM_LEDS);

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

// Logic for Joystick movement: returns true if a change happened
bool handleJoystick(int xPin, int yPin, int &cursor, int digits[])
{
    int xVal = analogRead(xPin);
    int yVal = analogRead(yPin);
    bool changed = false;

    // X-Axis: Switch digit position (Left/Right)
    if (xVal > 900)
    {
        cursor++;
        if (cursor > 3)
            cursor = 0;
        delay(250);
        changed = true;
    }
    else if (xVal < 100)
    {
        cursor--;
        if (cursor < 0)
            cursor = 3;
        delay(250);
        changed = true;
    }

    // Y-Axis: Change digit value (Up/Down)
    if (yVal < 100)
    {
        digits[cursor]++;
        if (digits[cursor] > 9)
            digits[cursor] = 0;
        delay(250);
        changed = true;
    }
    else if (yVal > 900)
    {
        digits[cursor]--;
        if (digits[cursor] < 0)
            digits[cursor] = 9;
        delay(250);
        changed = true;
    }

    return changed;
}


// Method that displays a number in a certain display
void displayNumber(TM1637 &display, int number) {
  if (number > 9999 || number < 0) return -1; // cannot show number outside of range
  int digits[4] = [0, 0, 0, 0];
  digits[3] = number % 10;
  digits[2] = (number / 10) % 10
  digits[1] = (number / 100) % 10
  digits[0] = (number / 1000) % 10

  for (int i = 0; i < 4; i++)
  {
      display.display(i, digits[i]);
  }
}

// Method that generates a new operation and displays it
void newOperation() {
  int n1 = random(1, 50);
  int n2 = random(1, 50);
  targetAnswer = n1 + n2;

  Serial.print("\nNEW PROBLEM: ");
  Serial.print(n1);
  Serial.print(" + ");
  Serial.print(n2);
  Serial.print(" = ");
  Serial.println(targetAnswer);

  // Reset Player Arrays
  for (int i = 0; i < 4; i++)
  {
      playerA_digits[i] = 0;
      playerB_digits[i] = 0;
  }

  // Show problem on top displays
  dispNum1.display(2, (n1 / 10) % 10);
  dispNum1.display(3, n1 % 10);
  dispNum2.display(2, (n2 / 10) % 10);
  dispNum2.display(3, n2 % 10);

  // Clear Player displays to zero
  refreshDisplay(dispA, playerA_digits);
  refreshDisplay(dispB, playerB_digits);
}

// Method that calculates scores based on the current operation and the values on each player's display
void updateScores() {
  int valA = (playerA_digits[0] * 1000) + (playerA_digits[1] * 100) + (playerA_digits[2] * 10) + playerA_digits[3];
  int valB = (playerB_digits[0] * 1000) + (playerB_digits[1] * 100) + (playerB_digits[2] * 10) + playerB_digits[3];

  Serial.println("--- CHECKING ANSWERS ---");
  
  if (valA == targetAnswer)
  {
      Serial.println("PLAYER A WINS!");
      playerAScore++:
      if (playerAScore >= playerBScore) {
        playerBScore++; // push other player backwards
      }
      for (int i = 0; i < 4; i++)
          dispA.display(i, 8); // Success flash
  }

  if (valB == targetAnswer)
  {
      Serial.println("PLAYER B WINS!");
      playerBScore--; // GO BACKWARDS
      if (playerBScore <= playerAScore) {
        playerAScore--; // push other player backwards
      }
      for (int i = 0; i < 4; i++)
          dispB.display(i, 8); // Success flash
  }

  for (int i = 0; i < 8; i++) {
    if (i < playerAScore) {
      leds[i] = CRGB(playerAColor[0], playerAColor[1], playerAColor[2]);
    }
    if (NUM_LEDS - 1 - i > playerBScore) {
      leds[NUM_LEDS - 1 - i] = (CRGB(playerBColor[0], playerBColor[1], playerBColor[2]));
    }
  }

  if (playerAScore == NUM_LEDS || playerBScore == 0) {
    currentGameState = RESULTS
  }
}


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
    // Handle Player A Navigation & Input
    bool updateA = handleJoystick(pA_X, pA_Y, cursorA, playerA_digits);
    if (updateA)
        refreshDisplay(dispA, playerA_digits);

    // Handle Player B Navigation & Input
    bool updateB = handleJoystick(pB_X, pB_Y, cursorB, playerB_digits);
    if (updateB)
        refreshDisplay(dispB, playerB_digits);

    // Check Answer Button (Active LOW)
    if (digitalRead(buttonPin) == LOW)
    {
        checkWinCondition();
        delay(2000); // Winner display time
        generateNewProblem();
    }
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


