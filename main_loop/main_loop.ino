// -------------- LIBRARIES --------------
#include "FastLED.h"
#include <TM1637.h>

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
#define OPERATION_DISPLAY_1 2;
#define LEDS_STRIP 3; // Analog
#define PLAYER_A_DISPLAY 4;
#define OPERATIONS_LED_R 5; // Analog
#define OPERATION_DISPLAY_2 6; // Analog
#define CENTRAL_BUTTON 7;
#define PLAYER_B_DISPLAY 8;
#define OPERATIONS_LED_G 9; // Analog
#define OPERATIONS_LED_B 10; // Analog
#define latchPin 11; // Shift Register / Analog
#define dataPin 12; // Shift Register
#define clockPin 13; // Shift Register


// Shift Register Pins
#define EASY_BUTTON 0;
#define MEDIUM_BUTTON 1;
#define HARD_BUTTON 2;
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

TM1637 dispNum1(6, OPERATION_DISPLAY_1); // Number 1 of equation
TM1637 dispNum2(6, OPERATION_DISPLAY_2); // Number 2 of equation
TM1637 dispA(6, PLAYER_A_DISPLAY);    // Player A entry
TM1637 dispB(6, PLAYER_B_DISPLAY);   // Player B entry

// Enderecable LEDS
#define NUM_LEDS 8
CRGB leds[NUM_LEDS];

byte shift_pins = 0;

// -------------- END CONSTANTS --------------

// -------------- VARIABLES --------------

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

int getDifficulty(int difficulty) {
  return currentDifficulty;
}

void nextGameState() {
  currentGameState++;
  currentGameState = currentGameState % 3;
}

// -------------- END GETTERS / SETTERS --------------

// -------------- SETUP --------------

// Main Setup (Do not remove!!!)
void setup() {
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

void refreshDisplay(TM1637 &display, int digits[])
{
    for (int i = 0; i < 4; i++)
    {
        display.display(i, digits[i]);
    }
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
  int operation = random(4);
  int first_number;
  int second_number;
  switch (currentDifficulty){
    case EASY:
      switch (operation){
        case ADD:
          first_number = random(100);
          second_number = random(100);
          targetAnswer = first_number + second_number;
        case SUBTRACT:
          first_number = random(100);
          second_number = random(first_number + 1);
          targetAnswer = first_number - second_number;
        case MULTIPLY:
          first_number = random(9) + 2;
          second_number = random(9) + 2;
          targetAnswer = first_number * second_number;
        case DIVIDE:
          do {
            first_number = random(100);
            second_number = random(9) + 1;
          } while (first_number % second_number != 0);
          targetAnswer = first_number / second_number;
      }
    case MEDIUM:
      switch (operation){
        case ADD:
          first_number = random(1000);
          second_number = random(1000);
          targetAnswer = first_number + second_number;
        case SUBTRACT:
          first_number = random(1000);
          second_number = random(first_number + 1);
          targetAnswer = first_number - second_number;
        case MULTIPLY:
          do {
            first_number = random(998) + 2;
            second_number = random(998) + 2;
          } while (first_number * second_number > 999);
          targetAnswer = first_number * second_number;
        case DIVIDE:
          do {
            first_number = random(1000);
            second_number = random(99) + 1;
          } while (first_number % second_number != 0);
          targetAnswer = first_number / second_number;
      }
    case HARD:
      switch (operation){
        case ADD:
          first_number = random(10000);
          second_number = random(9999 - first_number);
          targetAnswer = first_number + second_number;
        case SUBTRACT:
          first_number = random(10000);
          second_number = random(first_number + 1);
          targetAnswer = first_number - second_number;
        case MULTIPLY:
          do {
            first_number = random(9998) + 2;
            second_number = random(9998) + 2;
          } while (first_number * second_number > 9999);
          targetAnswer = first_number * second_number;
        case DIVIDE:
          do {
            first_number = random(10000);
            second_number = random(9999) + 1;
          } while (first_number % second_number != 0);
          targetAnswer = first_number / second_number;
      }
  }
  // Reset Player Arrays
  for (int i = 0; i < 4; i++)
  {
      playerA_digits[i] = 0;
      playerB_digits[i] = 0;
  }


  // Show problem on top displays
  dispNum1.display(2, (first_number / 10) % 10);
  dispNum1.display(3, first_number % 10);
  dispNum2.display(2, (second_number / 10) % 10);
  dispNum2.display(3, second_number % 10);

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
// TODO
void resultsLoop() {

  // Flicker scoring LED's to showcase winning

  // Store current score between the 2 players (could this be stored forever?)

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


