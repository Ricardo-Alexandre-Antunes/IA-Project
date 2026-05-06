int button1Pin = 2;
int button10Pin = 3;
int number = 0;
int value1 = 0;
int value10 = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(button1Pin, INPUT);
  pinMode(button10Pin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  number += max(digitalRead(button1Pin) - value1, 0);
  number += max(digitalRead(button10Pin) - value10, 0) * 10;
  value1 = digitalRead(button1Pin);
  value10 = digitalRead(button10Pin);
  Serial.println(number);
}
