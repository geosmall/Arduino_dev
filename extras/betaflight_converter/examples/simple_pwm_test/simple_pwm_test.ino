/**
 * Simple PWM Test - Direct HardwareTimer API
 * Bypasses PWMOutputBank to test raw timer functionality
 */

#include <ci_log.h>

HardwareTimer tim1(TIM1);
HardwareTimer tim3(TIM3);

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== Simple PWM Test (Direct Timer API) ===\n");
  CI_BUILD_INFO();
  CI_LOG("\n");

  // Test 1: TIM1_CH1 on PA8 @ 1 kHz, 50% duty cycle
  CI_LOG("Configuring TIM1_CH1 (PA8) @ 1 kHz, 50% duty...\n");
  tim1.setPWM(1, PA8, 1000, 50);  // Channel 1, PA8, 1 kHz, 50% duty
  CI_LOG("✓ TIM1 configured\n\n");

  // Test 2: TIM3_CH3 on PB0 @ 1 kHz, 50% duty cycle
  CI_LOG("Configuring TIM3_CH3 (PB0_ALT1) @ 1 kHz, 50% duty...\n");
  tim3.setPWM(3, PB0_ALT1, 1000, 50);  // Channel 3, PB0_ALT1, 1 kHz, 50% duty
  CI_LOG("✓ TIM3 configured\n\n");

  // Setup GPIO polling inputs
  pinMode(PA0, INPUT);
  pinMode(PA1, INPUT);

  CI_LOG("Jumper Connections:\n");
  CI_LOG("  1. PA8 → PA0 (TIM1_CH1)\n");
  CI_LOG("  2. PB0 → PA1 (TIM3_CH3)\n\n");

  CI_LOG("Starting 10-second sampling test...\n");
  CI_READY_TOKEN();
}

void loop() {
  static int sample_count = 0;
  static uint32_t pa0_high_count = 0;
  static uint32_t pa1_high_count = 0;

  if (sample_count >= 10000) {  // 10000 samples at 1ms = 10 seconds
    CI_LOG("\n=== Test Results (10000 samples over 10s) ===\n");
    CI_LOGF("PA0 (TIM1/PA8): %lu HIGH samples\n", pa0_high_count);
    CI_LOGF("PA1 (TIM3/PB0): %lu HIGH samples\n", pa1_high_count);

    // At 50% duty cycle, expect ~5000 HIGH samples
    float pa0_duty = (float)pa0_high_count / sample_count * 100.0f;
    float pa1_duty = (float)pa1_high_count / sample_count * 100.0f;

    CI_LOG("\nDuty Cycle Analysis:\n");
    CI_LOG_FLOAT("  TIM1/PA8: ", pa0_duty, 1);
    CI_LOG("% - ");
    if (pa0_duty > 40.0f && pa0_duty < 60.0f) {
      CI_LOG("✓ PASS (50% duty detected)\n");
    } else if (pa0_high_count == 0) {
      CI_LOG("✗ FAIL (no signal)\n");
    } else {
      CI_LOGF("✗ FAIL (expected ~50%%, got ");
      CI_LOG_FLOAT("", pa0_duty, 1);
      CI_LOG("%)\n");
    }

    CI_LOG_FLOAT("  TIM3/PB0: ", pa1_duty, 1);
    CI_LOG("% - ");
    if (pa1_duty > 40.0f && pa1_duty < 60.0f) {
      CI_LOG("✓ PASS (50% duty detected)\n");
    } else if (pa1_high_count == 0) {
      CI_LOG("✗ FAIL (no signal)\n");
    } else {
      CI_LOGF("✗ FAIL (expected ~50%%, got ");
      CI_LOG_FLOAT("", pa1_duty, 1);
      CI_LOG("%)\n");
    }

    CI_LOG("\n");
    if (pa0_high_count > 0 || pa1_high_count > 0) {
      CI_LOG("✓ At least one timer is generating PWM\n");
    } else {
      CI_LOG("✗ No PWM signals detected - timer configuration issue\n");
    }

    CI_LOG("*STOP*\n");
    while(1);
  }

  // Sample GPIO inputs
  if (digitalRead(PA0)) pa0_high_count++;
  if (digitalRead(PA1)) pa1_high_count++;
  sample_count++;

  // Progress indicator every 2000 samples (~2 seconds)
  if (sample_count % 2000 == 0) {
    CI_LOGF("Sampling... %d/10000\n", sample_count);
  }

  delayMicroseconds(1000);  // 1ms between samples
}
