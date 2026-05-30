// -------------- LIBRARIES --------------
#include <FastLED.h>
#include <TM1637TinyDisplay.h>

// -------------- OPERATION CONSTANTS --------------
#define ADD 0
#define SUBTRACT 1
#define MULTIPLY 2

// -------------- GAME STATE CONSTANTS --------------
#define GAME 0
#define RESULTS 1

// -------------- PIN CONSTANTS --------------
#define OPERATION_DISPLAY_1 2
#define PLAYER_A_DISPLAY 4
#define OPERATIONS_LED_R 5
#define OPERATION_DISPLAY_2 6
#define DISPLAY_CLKS 7
#define PLAYER_B_DISPLAY 8
#define OPERATIONS_LED_G 9
#define OPERATIONS_LED_B 10
#define LED_STRIP 11
#define CENTRAL_BUTTON 13

// -------------- ANALOG PINS --------------
#define PA_X A0
#define PA_Y A1
#define PB_X A2
#define PB_Y A3

// -------------- LED STRIP --------------
#define NUM_LEDS 8
#define POINTS_TO_WIN 4

CRGB leds[NUM_LEDS];

// -------------- DISPLAYS --------------
// If a display is upside down, change the final true/false value.
TM1637TinyDisplay dispNum1(DISPLAY_CLKS, OPERATION_DISPLAY_1, 0, false);
TM1637TinyDisplay dispNum2(DISPLAY_CLKS, OPERATION_DISPLAY_2, 0, true);
TM1637TinyDisplay dispA(DISPLAY_CLKS, PLAYER_A_DISPLAY, 0, false);
TM1637TinyDisplay dispB(DISPLAY_CLKS, PLAYER_B_DISPLAY, 0, true);

// -------------- GAME VARIABLES --------------
int currentGameState = GAME;

int currentOperation = ADD;
int firstNumber = 0;
int secondNumber = 0;
int targetAnswer = 0;

int playerAValue = 0;
int playerBValue = 0;

// Selected place:
// 0 = units
// 1 = tens
// 2 = hundreds
// 3 = thousands
int playerAPlace = 0;
int playerBPlace = 0;

int playerAScore = 0;
int playerBScore = 0;

// Player A = red, left side
CRGB playerAColor = CRGB::Red;

// Player B = blue, right side
CRGB playerBColor = CRGB::Blue;

// -------------- JOYSTICK SETTINGS --------------
const int JOY_LOW = 200;
const int JOY_HIGH = 800;
const int JOY_CENTER_LOW = 400;
const int JOY_CENTER_HIGH = 600;

struct JoystickState
{
  bool xLocked;
  bool yLocked;
};

JoystickState joystickA = {false, false};
JoystickState joystickB = {false, false};

// -------------- BUTTON DEBOUNCE --------------
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long lastButtonChangeTime = 0;
const unsigned long buttonDebounceDelay = 50;

// -------------- SETUP --------------
void setup()
{
  Serial.begin(9600);

  pinMode(CENTRAL_BUTTON, INPUT_PULLUP);

  pinMode(OPERATIONS_LED_R, OUTPUT);
  pinMode(OPERATIONS_LED_G, OUTPUT);
  pinMode(OPERATIONS_LED_B, OUTPUT);

  dispNum1.begin(true);
  dispNum2.begin(true);
  dispA.begin(true);
  dispB.begin(true);

  dispNum2.flipDisplay(true);
  dispB.flipDisplay(true);

  dispNum1.setBrightness(2);
  dispNum2.setBrightness(2);
  dispA.setBrightness(2);
  dispB.setBrightness(2);

  FastLED.addLeds<WS2812, LED_STRIP, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  randomSeed(analogRead(A5));

  newOperation();
}

// -------------- DISPLAY HELPERS --------------
void showNumber4(TM1637TinyDisplay &display, int number)
{
  if (number < 0)
    number = 0;

  if (number > 9999)
    number = 9999;

  // true = show leading zeroes
  // Example: 1 appears as 0001
  display.showNumber(number, true);
}

// -------------- RGB OPERATION LED --------------
void setOperationColor(int operation)
{
  if (operation == ADD)
  {
    // Blue = +
    analogWrite(OPERATIONS_LED_R, 0);
    analogWrite(OPERATIONS_LED_G, 0);
    analogWrite(OPERATIONS_LED_B, 255);
  }
  else if (operation == SUBTRACT)
  {
    // Red = -
    analogWrite(OPERATIONS_LED_R, 255);
    analogWrite(OPERATIONS_LED_G, 0);
    analogWrite(OPERATIONS_LED_B, 0);
  }
  else if (operation == MULTIPLY)
  {
    // Green = x
    analogWrite(OPERATIONS_LED_R, 0);
    analogWrite(OPERATIONS_LED_G, 255);
    analogWrite(OPERATIONS_LED_B, 0);
  }
}

// -------------- PLACE VALUE HELPERS --------------
int getStepFromPlace(int place)
{
  if (place == 0)
    return 1;

  if (place == 1)
    return 10;

  if (place == 2)
    return 100;

  return 1000;
}

int getDigitAtPlace(int value, int place)
{
  int step = getStepFromPlace(place);
  return (value / step) % 10;
}

void increaseDigitAtPlace(int &value, int place)
{
  int step = getStepFromPlace(place);
  int digit = getDigitAtPlace(value, place);

  if (digit == 9)
  {
    value -= 9 * step;
  }
  else
  {
    value += step;
  }
}

void decreaseDigitAtPlace(int &value, int place)
{
  int step = getStepFromPlace(place);
  int digit = getDigitAtPlace(value, place);

  if (digit == 0)
  {
    value += 9 * step;
  }
  else
  {
    value -= step;
  }
}

// -------------- JOYSTICK LOGIC --------------
bool joystickIsCentered(int value)
{
  return value >= JOY_CENTER_LOW && value <= JOY_CENTER_HIGH;
}

bool handleJoystickA(int xPin, int yPin, int &selectedPlace, int &playerValue, JoystickState &state)
{
  int xVal = analogRead(xPin);
  int yVal = analogRead(yPin);

  bool changed = false;

  if (joystickIsCentered(xVal))
  {
    state.xLocked = false;
  }

  if (joystickIsCentered(yVal))
  {
    state.yLocked = false;
  }

  // Player A:
  // X axis changes selected digit/place
  if (!state.xLocked)
  {
    if (xVal > JOY_HIGH)
    {
      selectedPlace++;

      if (selectedPlace > 3)
        selectedPlace = 0;

      state.xLocked = true;
      changed = true;

      Serial.print("Player A selected place: ");
      Serial.println(selectedPlace);
    }
    else if (xVal < JOY_LOW)
    {
      selectedPlace--;

      if (selectedPlace < 0)
        selectedPlace = 3;

      state.xLocked = true;
      changed = true;

      Serial.print("Player A selected place: ");
      Serial.println(selectedPlace);
    }
  }

  // Player A:
  // Y axis changes value at selected digit/place
  if (!state.yLocked)
  {
    if (yVal < JOY_LOW)
    {
      increaseDigitAtPlace(playerValue, selectedPlace);

      state.yLocked = true;
      changed = true;
    }
    else if (yVal > JOY_HIGH)
    {
      decreaseDigitAtPlace(playerValue, selectedPlace);

      state.yLocked = true;
      changed = true;
    }
  }

  return changed;
}

bool handleJoystickB(int xPin, int yPin, int &selectedPlace, int &playerValue, JoystickState &state)
{
  int xVal = analogRead(xPin);
  int yVal = analogRead(yPin);

  bool changed = false;

  if (joystickIsCentered(xVal))
  {
    state.xLocked = false;
  }

  if (joystickIsCentered(yVal))
  {
    state.yLocked = false;
  }

  // Player B:
  // Y axis changes selected digit/place
  // Direction inverted because Player B joystick is flipped
  if (!state.yLocked)
  {
    if (yVal > JOY_HIGH)
    {
      selectedPlace--;

      if (selectedPlace < 0)
        selectedPlace = 3;

      state.yLocked = true;
      changed = true;

      Serial.print("Player B selected place: ");
      Serial.println(selectedPlace);
    }
    else if (yVal < JOY_LOW)
    {
      selectedPlace++;

      if (selectedPlace > 3)
        selectedPlace = 0;

      state.yLocked = true;
      changed = true;

      Serial.print("Player B selected place: ");
      Serial.println(selectedPlace);
    }
  }

  // Player B:
  // X axis changes value at selected digit/place
  if (!state.xLocked)
  {
    if (xVal < JOY_LOW)
    {
      decreaseDigitAtPlace(playerValue, selectedPlace);

      state.xLocked = true;
      changed = true;
    }
    else if (xVal > JOY_HIGH)
    {
      increaseDigitAtPlace(playerValue, selectedPlace);

      state.xLocked = true;
      changed = true;
    }
  }

  return changed;
}

// -------------- BUTTON LOGIC --------------
bool centralButtonPressed()
{
  bool reading = digitalRead(CENTRAL_BUTTON);

  if (reading != lastButtonReading)
  {
    lastButtonChangeTime = millis();
  }

  if ((millis() - lastButtonChangeTime) > buttonDebounceDelay)
  {
    if (reading != stableButtonState)
    {
      stableButtonState = reading;

      if (stableButtonState == LOW)
      {
        lastButtonReading = reading;
        return true;
      }
    }
  }

  lastButtonReading = reading;
  return false;
}

// -------------- NEW OPERATION --------------
void newOperation()
{
  currentOperation = random(3);

  if (currentOperation == ADD)
  {
    firstNumber = random(0, 100);
    secondNumber = random(0, 100);
    targetAnswer = firstNumber + secondNumber;
  }
  else if (currentOperation == SUBTRACT)
  {
    firstNumber = random(0, 100);
    secondNumber = random(0, firstNumber + 1);
    targetAnswer = firstNumber - secondNumber;
  }
  else if (currentOperation == MULTIPLY)
  {
    firstNumber = random(2, 13);
    secondNumber = random(2, 13);
    targetAnswer = firstNumber * secondNumber;
  }

  Serial.println("----- NEW OPERATION -----");
  Serial.print("First number: ");
  Serial.println(firstNumber);
  Serial.print("Second number: ");
  Serial.println(secondNumber);
  Serial.print("Operation: ");
  Serial.println(currentOperation);
  Serial.print("Answer: ");
  Serial.println(targetAnswer);

  showNumber4(dispNum1, firstNumber);
  showNumber4(dispNum2, secondNumber);

  setOperationColor(currentOperation);

  playerAValue = 0;
  playerBValue = 0;

  playerAPlace = 0;
  playerBPlace = 0;

  showNumber4(dispA, playerAValue);
  showNumber4(dispB, playerBValue);
}

// -------------- SCORE LEDS --------------
void updateLedStrip()
{
  FastLED.clear();

  // Player A red lights from the LEFT side
  for (int i = 0; i < playerAScore; i++)
  {
    leds[i] = playerAColor;
  }

  // Player B blue lights from the RIGHT side
  for (int i = 0; i < playerBScore; i++)
  {
    leds[NUM_LEDS - 1 - i] = playerBColor;
  }

  FastLED.show();
}
// -------------- CHECK ANSWERS --------------
void checkAnswers()
{
  Serial.println("----- CHECKING ANSWERS -----");

  Serial.print("Player A: ");
  Serial.println(playerAValue);

  Serial.print("Player B: ");
  Serial.println(playerBValue);

  Serial.print("Correct answer: ");
  Serial.println(targetAnswer);

  bool playerACorrect = playerAValue == targetAnswer;
  bool playerBCorrect = playerBValue == targetAnswer;

  if (playerACorrect && playerAScore < POINTS_TO_WIN)
  {
    playerAScore++;
    Serial.println("Player A got it right!");
  }

  if (playerBCorrect && playerBScore < POINTS_TO_WIN)
  {
    playerBScore++;
    Serial.println("Player B got it right!");
  }

  updateLedStrip();

  if (playerAScore >= POINTS_TO_WIN || playerBScore >= POINTS_TO_WIN)
  {
    currentGameState = RESULTS;
  }
  else
  {
    delay(1000);
    newOperation();
  }
}

// -------------- GAME LOOP --------------
void gameLoop()
{
  bool updateA = handleJoystickA(PA_X, PA_Y, playerAPlace, playerAValue, joystickA);

  if (updateA)
  {
    showNumber4(dispA, playerAValue);
  }

  bool updateB = handleJoystickB(PB_X, PB_Y, playerBPlace, playerBValue, joystickB);

  if (updateB)
  {
    showNumber4(dispB, playerBValue);
  }

  if (centralButtonPressed())
  {
    checkAnswers();
  }
}

//--------------- RESET THE GAME --------

void resetGame()
{
  playerAScore = 0;
  playerBScore = 0;

  playerAValue = 0;
  playerBValue = 0;

  playerAPlace = 0;
  playerBPlace = 0;

  currentGameState = GAME;

  FastLED.clear();
  FastLED.show();

  newOperation();
}

// -------------- RESULTS LOOP --------------
void resultsLoop()
{
  CRGB winnerColor;

  if (playerAScore >= POINTS_TO_WIN)
  {
    winnerColor = playerAColor;
  }
  else
  {
    winnerColor = playerBColor;
  }

  // Show 8888 on all displays
  showNumber4(dispNum1, 8888);
  showNumber4(dispNum2, 8888);
  showNumber4(dispA, 8888);
  showNumber4(dispB, 8888);

  // Blink winner color
  static unsigned long lastBlinkTime = 0;
  static bool ledsOn = false;

  if (millis() - lastBlinkTime >= 300)
  {
    lastBlinkTime = millis();
    ledsOn = !ledsOn;

    if (ledsOn)
    {
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = winnerColor;
      }
    }
    else
    {
      FastLED.clear();
    }

    FastLED.show();
  }

  // Press central button to restart
  if (centralButtonPressed())
  {
    resetGame();
  }
}

// -------------- MAIN LOOP --------------
void loop()
{
  switch (currentGameState)
  {
  case GAME:
    gameLoop();
    break;

  case RESULTS:
    resultsLoop();
    break;
  }
}