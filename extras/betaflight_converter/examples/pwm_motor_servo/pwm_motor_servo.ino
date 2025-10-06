/*
 * PWM Motor and Servo Configuration Example
 *
 * Demonstrates how to use generated motor and servo configurations
 * with the TimerPWM library for flight controller PWM outputs.
 *
 * This example uses MATEKH743 config which has both motors and servos.
 *
 * Hardware: MATEKH743 flight controller (H743-WLITE or similar)
 * Connections: Connect servos to PE5, PE6 (TIM15_Bank)
 *              Connect motors to PA0-PA3 (TIM5_Bank)
 *
 * NOTE: This example will NOT compile for F411 boards (no TIM15/TIM5).
 *       Use with STM32H743 FQBN only.
 */

#include <PWMOutputBank.h>

// Include generated config with servo support (copied to sketch folder)
#include "MTKS-MATEKH743.h"

PWMOutputBank servo_pwm;
PWMOutputBank motor_pwm;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("\n=== Motor & Servo PWM Example ===\n");

  // Initialize Servo Bank (TIM15 @ 50 Hz for standard servos)
  Serial.println("Initializing Servos (TIM15 @ 50 Hz)...");
  servo_pwm.Init(BoardConfig::Servo::TIM15_Bank::timer,
                 BoardConfig::Servo::frequency_hz);

  // Attach servo channels
  auto& servo1 = BoardConfig::Servo::TIM15_Bank::servo1;
  auto& servo2 = BoardConfig::Servo::TIM15_Bank::servo2;

  servo_pwm.AttachChannel(servo1.ch, servo1.pin, servo1.min_us, servo1.max_us);
  servo_pwm.AttachChannel(servo2.ch, servo2.pin, servo2.min_us, servo2.max_us);

  Serial.print("  Servo 1: Pin ");
  Serial.print(servo1.pin);
  Serial.print(", CH");
  Serial.print(servo1.ch);
  Serial.print(", Range: ");
  Serial.print(servo1.min_us);
  Serial.print("-");
  Serial.print(servo1.max_us);
  Serial.println(" µs");

  Serial.print("  Servo 2: Pin ");
  Serial.print(servo2.pin);
  Serial.print(", CH");
  Serial.print(servo2.ch);
  Serial.print(", Range: ");
  Serial.print(servo2.min_us);
  Serial.print("-");
  Serial.print(servo2.max_us);
  Serial.println(" µs");

  servo_pwm.Start();

  // Initialize Motor Bank (TIM5 @ 1000 Hz for OneShot125)
  Serial.println("\nInitializing Motors (TIM5 @ 1000 Hz)...");
  motor_pwm.Init(BoardConfig::Motor::TIM5_Bank::timer,
                 BoardConfig::Motor::frequency_hz);

  // Attach motor channels
  auto& motor3 = BoardConfig::Motor::TIM5_Bank::motor3;
  auto& motor4 = BoardConfig::Motor::TIM5_Bank::motor4;
  auto& motor5 = BoardConfig::Motor::TIM5_Bank::motor5;
  auto& motor6 = BoardConfig::Motor::TIM5_Bank::motor6;

  motor_pwm.AttachChannel(motor3.ch, motor3.pin, motor3.min_us, motor3.max_us);
  motor_pwm.AttachChannel(motor4.ch, motor4.pin, motor4.min_us, motor4.max_us);
  motor_pwm.AttachChannel(motor5.ch, motor5.pin, motor5.min_us, motor5.max_us);
  motor_pwm.AttachChannel(motor6.ch, motor6.pin, motor6.min_us, motor6.max_us);

  Serial.print("  Motors 3-6: OneShot125 (");
  Serial.print(motor3.min_us);
  Serial.print("-");
  Serial.print(motor3.max_us);
  Serial.println(" µs)");

  motor_pwm.Start();

  Serial.println("\n=== PWM Outputs Running ===");
  Serial.println("Servos: Sweeping 1000-2000 µs");
  Serial.println("Motors: Idle at 125 µs");
}

void loop() {
  static unsigned long last_update = 0;
  static int servo_pos = 1000;
  static int direction = 1;

  // Sweep servos back and forth every 20ms
  if (millis() - last_update > 20) {
    servo_pos += direction * 10;

    // Reverse direction at endpoints
    if (servo_pos >= 2000 || servo_pos <= 1000) {
      direction = -direction;
    }

    // Update servo positions
    servo_pwm.SetPulseWidth(BoardConfig::Servo::TIM15_Bank::servo1.ch, servo_pos);
    servo_pwm.SetPulseWidth(BoardConfig::Servo::TIM15_Bank::servo2.ch, 3000 - servo_pos); // Opposite direction

    // Keep motors at idle (125 µs for OneShot125)
    motor_pwm.SetPulseWidth(BoardConfig::Motor::TIM5_Bank::motor3.ch, 125);
    motor_pwm.SetPulseWidth(BoardConfig::Motor::TIM5_Bank::motor4.ch, 125);
    motor_pwm.SetPulseWidth(BoardConfig::Motor::TIM5_Bank::motor5.ch, 125);
    motor_pwm.SetPulseWidth(BoardConfig::Motor::TIM5_Bank::motor6.ch, 125);

    last_update = millis();
  }
}
