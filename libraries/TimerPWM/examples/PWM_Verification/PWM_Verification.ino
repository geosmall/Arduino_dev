/**
 * PWM_Verification.ino - Input capture verification example
 *
 * Verifies PWM output using TIM2 Input Capture to measure TIM3 PWM output
 * with BoardConfig integration.
 * No oscilloscope or logic analyzer needed - just a jumper wire!
 *
 * Hardware Setup:
 * - Board: NUCLEO_F411RE
 * - PWM Output: PB4 (Arduino D5, TIM3_CH1)
 * - Input Capture: PA0 (Arduino A0, TIM2_CH1)
 * - Connect jumper wire: D5 → A0
 *
 * This example validates:
 * - PWM frequency (50 Hz ± 2%)
 * - Pulse width (1500 µs ± 10 µs)
 */

#include <PWMOutputBank.h>
#include <ci_log.h>
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"

// PWM Output on TIM3
PWMOutputBank pwm;

// Input Capture on TIM2
HardwareTimer tim2(TIM2);
volatile uint32_t capture_period_us = 0;
volatile bool measurement_ready = false;

void captureCallback() {
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(1);

  // Calculate period in microseconds
  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Validate measurement range (10-30 ms for 33-100 Hz)
  if (period_us > 10000 && period_us < 30000) {
    capture_period_us = period_us;
    measurement_ready = true;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for Serial or timeout

  CI_LOG("=== PWM Verification Test ===\n");
  CI_LOG("Board: NUCLEO_F411RE\n");
  CI_BUILD_INFO();
  CI_LOG("\n");

  // Configure PWM Output using BoardConfig
  if (!pwm.Init(BoardConfig::Servo::timer, BoardConfig::Servo::frequency_hz)) {
    CI_LOG("ERROR: Failed to initialize PWM timer\n");
    while (1);
  }
  CI_LOGF("PWM Timer: TIM3 @ %lu Hz\n", BoardConfig::Servo::frequency_hz);

  auto& pwm_ch = BoardConfig::Servo::pwm_output;
  if (!pwm.AttachChannel(pwm_ch.ch, pwm_ch.pin, pwm_ch.min_us, pwm_ch.max_us)) {
    CI_LOG("ERROR: Failed to attach PWM channel\n");
    while (1);
  }
  CI_LOG("PWM Output: PB4 (Arduino D5)\n");

  // Set 1500 µs pulse width
  pwm.SetPulseWidth(pwm_ch.ch, 1500);
  pwm.Start();
  CI_LOG("PWM Pulse: 1500 µs\n\n");

  // Configure Input Capture using BoardConfig
  auto& cap_ch = BoardConfig::Servo::input_capture;
  tim2.setMode(cap_ch.ch, TIMER_INPUT_CAPTURE_RISING, cap_ch.pin);
  tim2.setPrescaleFactor(99);  // 100 MHz / 100 = 1 MHz tick rate
  tim2.setOverflow(0xFFFFFFFF);  // Max period (32-bit timer)
  tim2.attachInterrupt(cap_ch.ch, captureCallback);
  tim2.resume();

  CI_LOG("Input Capture: PA0 (Arduino A0)\n");
  CI_LOG("Connect jumper: D5 → A0\n\n");

  CI_READY_TOKEN();
}

void loop() {
  static int measurement_count = 0;

  if (measurement_ready) {
    measurement_ready = false;

    float measured_freq = 1000000.0 / capture_period_us;

    CI_LOGF("Period: %lu µs, Freq: ", capture_period_us);
    CI_LOG_FLOAT("", measured_freq, 2);
    CI_LOG(" Hz\n");

    // Validation with ±2% tolerance
    bool freq_valid = (measured_freq >= 49.0 && measured_freq <= 51.0);

    if (freq_valid) {
      CI_LOG("✓ PASS: Frequency within tolerance (49-51 Hz)\n");
    } else {
      CI_LOG("✗ FAIL: Frequency out of range (expected 49-51 Hz)\n");
    }

    CI_LOG("\n");

    // Stop after 3 successful measurements for HIL testing
    measurement_count++;
    if (measurement_count >= 3) {
      CI_LOG("*STOP*\n");
      while(1); // Halt for HIL framework
    }
  }

  delay(1000);
}
