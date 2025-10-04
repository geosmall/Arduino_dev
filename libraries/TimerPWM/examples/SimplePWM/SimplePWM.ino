/**
 * SimplePWM.ino - Basic PWM output example
 *
 * Demonstrates basic PWM output on a single channel using TIM3.
 *
 * Hardware:
 * - Board: NUCLEO_F411RE
 * - PWM Output: PB4 (Arduino D5, TIM3_CH1)
 * - Connect LED + resistor to D5 to visualize PWM
 *
 * This example outputs 50 Hz PWM with varying pulse widths:
 * - 1000 µs (1 ms) minimum
 * - 1500 µs (1.5 ms) center
 * - 2000 µs (2 ms) maximum
 */

#include <PWMOutputBank.h>

// Create PWM bank instance
PWMOutputBank pwm;

// Pin definitions for NUCLEO_F411RE
const uint32_t PWM_PIN = PB4;  // TIM3_CH1 (Arduino pin D5)

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for Serial or timeout

  Serial.println("=== SimplePWM Example ===");
  Serial.println("Board: NUCLEO_F411RE");
  Serial.println("Timer: TIM3");
  Serial.println("Pin: PB4 (Arduino D5)");
  Serial.println();

  // Initialize timer at 50 Hz (standard servo frequency)
  if (!pwm.Init(TIM3, 50)) {
    Serial.println("ERROR: Failed to initialize timer");
    while (1);
  }
  Serial.println("Timer initialized: 50 Hz");

  // Attach channel 1 to PB4 with 1000-2000 µs range
  if (!pwm.AttachChannel(1, PWM_PIN, 1000, 2000)) {
    Serial.println("ERROR: Failed to attach channel");
    while (1);
  }
  Serial.println("Channel 1 attached: PB4 (1000-2000 µs range)");

  // Start PWM output
  pwm.Start();
  Serial.println("PWM started");
  Serial.println();
}

void loop() {
  // Sweep pulse width from 1000 to 2000 µs
  Serial.println("Sweeping 1000 -> 2000 µs:");

  for (uint32_t pulse = 1000; pulse <= 2000; pulse += 100) {
    pwm.SetPulseWidth(1, pulse);
    Serial.print("  Pulse: ");
    Serial.print(pulse);
    Serial.println(" µs");
    delay(500);
  }

  Serial.println();
  Serial.println("Sweeping 2000 -> 1000 µs:");

  // Sweep back from 2000 to 1000 µs
  for (uint32_t pulse = 2000; pulse >= 1000; pulse -= 100) {
    pwm.SetPulseWidth(1, pulse);
    Serial.print("  Pulse: ");
    Serial.print(pulse);
    Serial.println(" µs");
    delay(500);

    if (pulse == 1000) break; // Prevent underflow
  }

  Serial.println();
  delay(1000);
}
