#include <TM1637.h>

// This code is a base code for the main project
// No working leds
// Works with all 4 displays both joysticks and 1 button.
// To bridge it to the final circuit, we need to add the leds and the difficulty buttons

// --- DISPLAY CONFIGURATION ---
// All displays share CLK on Pin 6. Individual DIO pins follow:
TM1637 dispNum1(6, 7); // Number 1 of equation
TM1637 dispNum2(6, 8); // Number 2 of equation
TM1637 dispA(6, 9);    // Player A entry
TM1637 dispB(6, 10);   // Player B entry

// --- HARDWARE PINS ---
const int buttonPin = 2;
const int pA_X = A0;
const int pA_Y = A1;
const int pB_X = A2;
const int pB_Y = A3;

// --- GAME STATE ---
int targetAnswer = 0;
int playerA_digits[4] = {0, 0, 0, 0};
int playerB_digits[4] = {0, 0, 0, 0};
int cursorA = 3; // 0=Thousand, 1=Hundred, 2=Ten, 3=Unit
int cursorB = 3;

void setup()
{
    Serial.begin(9600);
    pinMode(buttonPin, INPUT_PULLUP);

    // Initialize all displays
    dispNum1.init();
    dispNum2.init();
    dispA.init();
    dispB.init();

    // Set brightness (Problem displays dim, Player displays bright)
    dispNum1.set(2);
    dispNum2.set(2);
    dispA.set(5);
    dispB.set(5);

    randomSeed(analogRead(A5));
    generateNewProblem();
    Serial.println("--- SYSTEM READY ---");
}

void loop()
{
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

void generateNewProblem()
{
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

void checkWinCondition()
{
    int valA = (playerA_digits[0] * 1000) + (playerA_digits[1] * 100) + (playerA_digits[2] * 10) + playerA_digits[3];
    int valB = (playerB_digits[0] * 1000) + (playerB_digits[1] * 100) + (playerB_digits[2] * 10) + playerB_digits[3];

    Serial.println("--- CHECKING ANSWERS ---");

    if (valA == targetAnswer)
    {
        Serial.println("PLAYER A WINS!");
        for (int i = 0; i < 4; i++)
            dispA.display(i, 8); // Success flash
    }
    else
    {
        Serial.print("Player A Wrong: ");
        Serial.println(valA);
    }

    if (valB == targetAnswer)
    {
        Serial.println("PLAYER B WINS!");
        for (int i = 0; i < 4; i++)
            dispB.display(i, 8); // Success flash
    }
    else
    {
        Serial.print("Player B Wrong: ");
        Serial.println(valB);
    }
}