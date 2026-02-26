/*
 * =====================================================
 *   Single-Axis Solar Tracker
 * =====================================================
 * Description : Automatically rotates a solar panel
 *               East-to-West to track the sun using
 *               two LDR sensors and a servo motor.
 *
 * Board        : Arduino UNO
 * Components   : 2x LDR + 10kΩ voltage dividers
 *                SG90 Servo Motor
 *                Small Solar Panel (5V)
 *
 * Wiring       : LDR East → A0
 *                LDR West → A1
 *                Servo    → D9 (PWM)
 * =====================================================
 */

#include <Servo.h>

Servo trackerServo;

// --- Pin Definitions ---
const int LDR_EAST_PIN = A0;    // East-facing LDR
const int LDR_WEST_PIN = A1;    // West-facing LDR
const int SERVO_PIN    = 9;     // Servo signal pin (PWM)

// --- Tuning Parameters ---
const int THRESHOLD    = 25;    // Minimum delta to trigger movement.
                                // Reliable because averaged readings
                                // filter out noise before comparison.
const int STEP_SIZE    = 5;     // Degrees to rotate per control cycle
const int MIN_ANGLE    = 0;     // Hard limit — East (sunrise)
const int MAX_ANGLE    = 180;   // Hard limit — West (sunset)

// --- Averaging Parameters ---
const int AVG_SAMPLES  = 10;    // Readings per LDR per cycle
const int AVG_DELAY_MS = 100;     // Delay between samples (ms)
                                // Filters electrical noise and
                                // random LDR fluctuations

// --- State ---
int currentAngle = 0;           // Start facing East at sunrise


// =====================================================
//   readLDR()
//   Samples an LDR pin multiple times and returns the
//   average — eliminates noise before comparison.
//   Note: Lower value = brighter light
//         (LDR resistance drops with more light,
//          pulling the voltage divider output lower)
// =====================================================
int readLDR(int pin) {
  long total = 0;
  for (int i = 0; i < AVG_SAMPLES; i++) {
    total += analogRead(pin);
    delay(AVG_DELAY_MS);
  }
  return total / AVG_SAMPLES;
}


// =====================================================
//   setup()
// =====================================================
void setup() {
  Serial.begin(9600);
  trackerServo.attach(SERVO_PIN);
  trackerServo.write(currentAngle);
  Serial.println("========================================");
  Serial.println("  Single-Axis Solar Tracker");
  Serial.println("  Starting position: East (0 deg)");
  Serial.println("========================================");
}


// =====================================================
//   loop()
// =====================================================
void loop() {
  int ldrEast = readLDR(LDR_EAST_PIN);   // Averaged East reading
  int ldrWest = readLDR(LDR_WEST_PIN);   // Averaged West reading
  int delta   = ldrWest - ldrEast;       // +ve = West brighter
                                         // -ve = East brighter

  // --- Serial Monitor Output ---
  Serial.print("East: ");    Serial.print(ldrEast);
  Serial.print(" | West: "); Serial.print(ldrWest);
  Serial.print(" | Delta: "); Serial.print(delta);
  Serial.print(" | Angle: "); Serial.print(currentAngle);
  Serial.println(" deg");

  // --- Tracking Logic ---
  if (delta > THRESHOLD && currentAngle < MAX_ANGLE) {
    // West side is brighter → rotate toward West
    currentAngle += STEP_SIZE;
    trackerServo.write(currentAngle);

  } else if (delta < -THRESHOLD && currentAngle > MIN_ANGLE) {
    // East side is brighter → rotate toward East
    currentAngle -= STEP_SIZE;
    trackerServo.write(currentAngle);

  }
  // If |delta| <= THRESHOLD → panel is balanced, hold position

  delay(1500);   // 1500ms between cycles
}
