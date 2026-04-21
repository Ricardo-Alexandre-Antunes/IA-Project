// LDR Laser Target Detection System
// Detects significant light increases on 4 LDR sensors

const int LDR_PINS[] = {A0, A1, A2, A3};  // Analog pins for LDRs
const int NUM_LDRS = 4;

// Detection settings
const int THRESHOLD_DIFFERENCE = 150;  // How much higher than baseline to trigger
const int SAMPLE_INTERVAL = 10;        // Milliseconds between readings
const int BASELINE_SAMPLES = 50;       // Samples to average for baseline

int baseline[NUM_LDRS];
int currentValue[NUM_LDRS];
bool wasTriggered[NUM_LDRS] = {false, false, false, false};

void setup() {
  Serial.begin(9600);
  
  for (int i = 0; i < NUM_LDRS; i++) {
    pinMode(LDR_PINS[i], INPUT);
  }
  
  Serial.println("Calibrating baselines... Keep lasers OFF");
  delay(1000);
  calibrateBaselines();
  
  Serial.println("Calibration complete. Ready for laser hits!");
  Serial.println("----------------------------------------");
  Serial.println("Time (ms)\tLDR\tValue\tBaseline\tDelta");
  Serial.println("----------------------------------------");
}

void loop() {
  unsigned long timestamp = millis();
  
  // Read all LDRs
  for (int i = 0; i < NUM_LDRS; i++) {
    currentValue[i] = analogRead(LDR_PINS[i]);
  }
  
  // Check for significant increases
  for (int i = 0; i < NUM_LDRS; i++) {
    int delta = currentValue[i] - baseline[i];
    
    if (delta >= THRESHOLD_DIFFERENCE) {
      if (!wasTriggered[i]) {
        // New hit detected
        wasTriggered[i] = true;
        reportHit(timestamp, i, currentValue[i], delta);
      }
    } else {
      // Reset trigger state when laser moves away
      wasTriggered[i] = false;
    }
  }
  
  delay(SAMPLE_INTERVAL);
}

void calibrateBaselines() {
  long sums[NUM_LDRS] = {0, 0, 0, 0};
  
  for (int s = 0; s < BASELINE_SAMPLES; s++) {
    for (int i = 0; i < NUM_LDRS; i++) {
      sums[i] += analogRead(LDR_PINS[i]);
    }
    delay(10);
  }
  
  Serial.println("\nBaseline values:");
  for (int i = 0; i < NUM_LDRS; i++) {
    baseline[i] = sums[i] / BASELINE_SAMPLES;
    Serial.print("  LDR ");
    Serial.print(i + 1);
    Serial.print(" (Pin A");
    Serial.print(i);
    Serial.print("): ");
    Serial.println(baseline[i]);
  }
  Serial.println();
}

void reportHit(unsigned long timestamp, int ldrIndex, int value, int delta) {
  // TODO

  Serial.print(timestamp);
  Serial.print("\t\tLDR ");
  Serial.print(ldrIndex + 1);
  Serial.print("\t");
  Serial.print(value);
  Serial.print("\t");
  Serial.print(baseline[ldrIndex]);
  Serial.print("\t\t+");
  Serial.println(delta);
}