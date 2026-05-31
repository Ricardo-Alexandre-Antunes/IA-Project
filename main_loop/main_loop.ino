// -------------- LIBRARIES --------------
#include <FastLED.h>
#include <TM1637TinyDisplay.h>

// -------------- OPERATION CONSTANTS --------------
#define ADD 0
#define SUBTRACT 1
#define MULTIPLY 2
#define DIVIDE 3

// -------------- DIFFICULTY CONSTANTS --------------
#define EASY 0
#define MEDIUM 1
#define HARD 2

// -------------- GAME STATE CONSTANTS --------------
#define SELECT_DIFFICULTY 0
#define COUNTDOWN 1
#define GAME 2
#define RESULTS 3

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
TM1637TinyDisplay dispNum1(DISPLAY_CLKS, OPERATION_DISPLAY_1, 0, false);
TM1637TinyDisplay dispNum2(DISPLAY_CLKS, OPERATION_DISPLAY_2, 0, true);
TM1637TinyDisplay dispA(DISPLAY_CLKS, PLAYER_A_DISPLAY, 0, false);
TM1637TinyDisplay dispB(DISPLAY_CLKS, PLAYER_B_DISPLAY, 0, true);

// -------------- GAME VARIABLES --------------
int currentGameState = SELECT_DIFFICULTY;
int currentDifficulty = EASY;

int currentOperation = ADD;
long firstNumber = 0;
long secondNumber = 0;
long targetAnswer = 0;

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

CRGB playerAColor = CRGB::Red;
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

// -------------- COUNTDOWN VARIABLES --------------
unsigned long countdownPreviousTime = 0;
int countdownStep = 0;

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

  // Your LED strip is GRB, not RGB
  FastLED.addLeds<WS2812, LED_STRIP, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  randomSeed(analogRead(A5));

  showDifficultySelection();
}

// -------------- DISPLAY HELPERS --------------
void showNumber4(TM1637TinyDisplay &display, long number)
{
  if (number < 0)
    number = 0;

  if (number > 9999)
    number = 9999;

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
  else if (operation == DIVIDE)
  {
    // Purple = /
    analogWrite(OPERATIONS_LED_R, 255);
    analogWrite(OPERATIONS_LED_G, 0);
    analogWrite(OPERATIONS_LED_B, 255);
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

// Player A:
// X changes place
// Y changes value
bool handleJoystickA(int xPin, int yPin, int &selectedPlace, int &playerValue, JoystickState &state)
{
  int xVal = analogRead(xPin);
  int yVal = analogRead(yPin);

  bool changed = false;

  if (joystickIsCentered(xVal))
    state.xLocked = false;

  if (joystickIsCentered(yVal))
    state.yLocked = false;

  if (!state.xLocked)
  {
    if (xVal > JOY_HIGH)
    {
      selectedPlace++;

      if (selectedPlace > 3)
        selectedPlace = 0;

      state.xLocked = true;
      changed = true;
    }
    else if (xVal < JOY_LOW)
    {
      selectedPlace--;

      if (selectedPlace < 0)
        selectedPlace = 3;

      state.xLocked = true;
      changed = true;
    }
  }

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

// Player B flipped:
// Y changes place
// X changes value
bool handleJoystickB(int xPin, int yPin, int &selectedPlace, int &playerValue, JoystickState &state)
{
  int xVal = analogRead(xPin);
  int yVal = analogRead(yPin);

  bool changed = false;

  if (joystickIsCentered(xVal))
    state.xLocked = false;

  if (joystickIsCentered(yVal))
    state.yLocked = false;

  if (!state.yLocked)
  {
    if (yVal > JOY_HIGH)
    {
      selectedPlace--;

      if (selectedPlace < 0)
        selectedPlace = 3;

      state.yLocked = true;
      changed = true;
    }
    else if (yVal < JOY_LOW)
    {
      selectedPlace++;

      if (selectedPlace > 3)
        selectedPlace = 0;

      state.yLocked = true;
      changed = true;
    }
  }

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

// -------------- DIFFICULTY SELECTION --------------
void showDifficultySelection()
{
  FastLED.clear();

  if (currentDifficulty == EASY)
  {
    showNumber4(dispNum1, 1111);
    showNumber4(dispNum2, 1111);
    showNumber4(dispA, 1111);
    showNumber4(dispB, 1111);

    for (int i = 0; i < 2; i++)
    {
      leds[i] = CRGB::Green;
    }
  }
  else if (currentDifficulty == MEDIUM)
  {
    showNumber4(dispNum1, 2222);
    showNumber4(dispNum2, 2222);
    showNumber4(dispA, 2222);
    showNumber4(dispB, 2222);

    for (int i = 0; i < 5; i++)
    {
      leds[i] = CRGB::Yellow;
    }
  }
  else if (currentDifficulty == HARD)
  {
    showNumber4(dispNum1, 3333);
    showNumber4(dispNum2, 3333);
    showNumber4(dispA, 3333);
    showNumber4(dispB, 3333);

    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB::Red;
    }
  }

  FastLED.show();

  analogWrite(OPERATIONS_LED_R, 0);
  analogWrite(OPERATIONS_LED_G, 0);
  analogWrite(OPERATIONS_LED_B, 0);
}

void handleDifficultyJoystick()
{
  int yVal = analogRead(PA_Y);

  if (joystickIsCentered(yVal))
  {
    joystickA.yLocked = false;
  }

  if (!joystickA.yLocked)
  {
    if (yVal < JOY_LOW)
    {
      currentDifficulty++;

      if (currentDifficulty > HARD)
        currentDifficulty = EASY;

      joystickA.yLocked = true;
      showDifficultySelection();
    }
    else if (yVal > JOY_HIGH)
    {
      currentDifficulty--;

      if (currentDifficulty < EASY)
        currentDifficulty = HARD;

      joystickA.yLocked = true;
      showDifficultySelection();
    }
  }
}

void difficultySelectionLoop()
{
  handleDifficultyJoystick();

  if (centralButtonPressed())
  {
    startCountdown();
  }
}

// -------------- COUNTDOWN --------------
void startCountdown()
{
  currentGameState = COUNTDOWN;
  countdownStep = 0;
  countdownPreviousTime = millis();

  playerAScore = 0;
  playerBScore = 0;

  playerAValue = 0;
  playerBValue = 0;

  playerAPlace = 0;
  playerBPlace = 0;

  joystickA.xLocked = false;
  joystickA.yLocked = false;
  joystickB.xLocked = false;
  joystickB.yLocked = false;

  FastLED.clear();
  FastLED.show();

  showNumber4(dispNum1, 3);
  showNumber4(dispNum2, 3);
  showNumber4(dispA, 3);
  showNumber4(dispB, 3);
}

void countdownLoop()
{
  unsigned long currentTime = millis();

  if (currentTime - countdownPreviousTime >= 1000)
  {
    countdownPreviousTime = currentTime;
    countdownStep++;

    FastLED.clear();

    if (countdownStep == 1)
    {
      showNumber4(dispNum1, 3);
      showNumber4(dispNum2, 3);
      showNumber4(dispA, 3);
      showNumber4(dispB, 3);

      for (int i = 0; i < 2; i++)
      {
        leds[i] = CRGB::Red;
        leds[NUM_LEDS - 1 - i] = CRGB::Red;
      }
    }
    else if (countdownStep == 2)
    {
      showNumber4(dispNum1, 2);
      showNumber4(dispNum2, 2);
      showNumber4(dispA, 2);
      showNumber4(dispB, 2);

      for (int i = 0; i < 4; i++)
      {
        leds[i] = CRGB::Yellow;
        leds[NUM_LEDS - 1 - i] = CRGB::Yellow;
      }
    }
    else if (countdownStep == 3)
    {
      showNumber4(dispNum1, 1);
      showNumber4(dispNum2, 1);
      showNumber4(dispA, 1);
      showNumber4(dispB, 1);

      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CRGB::Green;
      }
    }
    else if (countdownStep >= 4)
    {
      FastLED.clear();
      FastLED.show();

      updateLedStrip();

      currentGameState = GAME;
      newOperation();
      return;
    }

    FastLED.show();
  }
}

// -------------- OPERATION GENERATION --------------

void generateEasyOperation()
{
  currentOperation = random(4);

  if (currentOperation == ADD)
  {
    // 1 + 1 up to 99 + 99
    firstNumber = random(1, 100);
    secondNumber = random(1, 100);
    targetAnswer = firstNumber + secondNumber;
  }
  else if (currentOperation == SUBTRACT)
  {
    // Result can never be negative
    firstNumber = random(1, 100);
    secondNumber = random(1, firstNumber + 1);
    targetAnswer = firstNumber - secondNumber;
  }
  else if (currentOperation == MULTIPLY)
  {
    // Tabuadas 2 to 10
    firstNumber = random(2, 11);
    secondNumber = random(2, 11);
    targetAnswer = firstNumber * secondNumber;
  }
  else if (currentOperation == DIVIDE)
  {
    // Two digit number divided by one digit number
    // Division must be exact and first number must be bigger than second
    do
    {
      firstNumber = random(10, 100);
      secondNumber = random(1, 10);
    } while (firstNumber <= secondNumber || firstNumber % secondNumber != 0);

    targetAnswer = firstNumber / secondNumber;
  }
}

void generateMediumOperation()
{
  currentOperation = random(4);

  if (currentOperation == ADD)
  {
    // 1 + 1 up to 999 + 999
    firstNumber = random(1, 1000);
    secondNumber = random(1, 1000);
    targetAnswer = firstNumber + secondNumber;
  }
  else if (currentOperation == SUBTRACT)
  {
    // Result can never be negative
    firstNumber = random(1, 1000);
    secondNumber = random(1, firstNumber + 1);
    targetAnswer = firstNumber - secondNumber;
  }
  else if (currentOperation == MULTIPLY)
  {
    // Max result must be 1000
    do
    {
      firstNumber = random(2, 1000);
      secondNumber = random(2, 1000);
      targetAnswer = firstNumber * secondNumber;
    } while (targetAnswer > 1000);
  }
  else if (currentOperation == DIVIDE)
  {
    // Three digit number divided by one or two digit number
    // Division must be exact and first number must be bigger than second
    do
    {
      firstNumber = random(100, 1000);
      secondNumber = random(1, 100);
    } while (firstNumber <= secondNumber || firstNumber % secondNumber != 0);

    targetAnswer = firstNumber / secondNumber;
  }
}

void generateHardOperation()
{
  currentOperation = random(4);

  if (currentOperation == ADD)
  {
    // Max result 9999
    firstNumber = random(1, 10000);
    secondNumber = random(1, 10000 - firstNumber);

    targetAnswer = firstNumber + secondNumber;
  }
  else if (currentOperation == SUBTRACT)
  {
    // Result can never be negative
    firstNumber = random(1, 10000);
    secondNumber = random(1, firstNumber + 1);

    targetAnswer = firstNumber - secondNumber;
  }
  else if (currentOperation == MULTIPLY)
  {
    // Generate multiplication without overflow
    // firstNumber * secondNumber can never be bigger than 9999
    firstNumber = random(2, 10000);

    long maxSecondNumber = 9999 / firstNumber;

    if (maxSecondNumber < 2)
    {
      maxSecondNumber = 2;
      firstNumber = random(2, 100);
      maxSecondNumber = 9999 / firstNumber;
    }

    secondNumber = random(2, maxSecondNumber + 1);

    targetAnswer = firstNumber * secondNumber;
  }
  else if (currentOperation == DIVIDE)
  {
    // Generate clean division directly:
    // firstNumber / secondNumber = targetAnswer
    // Max result 9999
    secondNumber = random(1, 5000);
    targetAnswer = random(2, (9999 / secondNumber) + 1);
    firstNumber = secondNumber * targetAnswer;

    // Safety fallback, should rarely be needed
    if (firstNumber > 9999 || firstNumber <= secondNumber)
    {
      secondNumber = random(1, 100);
      targetAnswer = random(2, 100);
      firstNumber = secondNumber * targetAnswer;
    }
  }
}

void newOperation()
{
  if (currentDifficulty == EASY)
  {
    generateEasyOperation();
  }
  else if (currentDifficulty == MEDIUM)
  {
    generateMediumOperation();
  }
  else if (currentDifficulty == HARD)
  {
    generateHardOperation();
  }

  playerAValue = 0;
  playerBValue = 0;

  playerAPlace = 0;
  playerBPlace = 0;

  showNumber4(dispNum1, firstNumber);
  showNumber4(dispNum2, secondNumber);

  showNumber4(dispA, playerAValue);
  showNumber4(dispB, playerBValue);

  setOperationColor(currentOperation);

  Serial.println("----- NEW OPERATION -----");
  Serial.print("Difficulty: ");
  Serial.println(currentDifficulty);

  Serial.print("First number: ");
  Serial.println(firstNumber);

  Serial.print("Second number: ");
  Serial.println(secondNumber);

  Serial.print("Operation: ");

  if (currentOperation == ADD)
    Serial.println("+");
  else if (currentOperation == SUBTRACT)
    Serial.println("-");
  else if (currentOperation == MULTIPLY)
    Serial.println("x");
  else if (currentOperation == DIVIDE)
    Serial.println("/");

  Serial.print("Answer: ");
  Serial.println(targetAnswer);
}

// -------------- SCORE LEDS --------------
void updateLedStrip()
{
  FastLED.clear();

  // Player A red lights from LEFT side
  for (int i = 0; i < playerAScore; i++)
  {
    leds[i] = playerAColor;
  }

  // Player B blue lights from RIGHT side
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

  bool playerACorrect = (long)playerAValue == targetAnswer;
  bool playerBCorrect = (long)playerBValue == targetAnswer;

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

// -------------- RESET GAME --------------
void resetGame()
{
  playerAScore = 0;
  playerBScore = 0;

  playerAValue = 0;
  playerBValue = 0;

  playerAPlace = 0;
  playerBPlace = 0;

  currentGameState = SELECT_DIFFICULTY;

  showDifficultySelection();
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

  showNumber4(dispNum1, 8888);
  showNumber4(dispNum2, 8888);
  showNumber4(dispA, 8888);
  showNumber4(dispB, 8888);

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
  case SELECT_DIFFICULTY:
    difficultySelectionLoop();
    break;

  case COUNTDOWN:
    countdownLoop();
    break;

  case GAME:
    gameLoop();
    break;

  case RESULTS:
    resultsLoop();
    break;
  }
}