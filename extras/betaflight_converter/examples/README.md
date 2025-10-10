# Betaflight BoardConfig Examples

These examples demonstrate how to use generated Betaflight BoardConfig headers in your Arduino sketches.

## Examples

### 1. basic_config_usage

**Purpose**: Learn how to access generated configuration data

Shows how to:
- Read storage, IMU, UART, ADC, and LED configurations
- Print configuration information to Serial
- Initialize status LEDs

**Hardware**: Any board (uses JHEF-JHEF411 config)

**Run this first** to understand the configuration structure.

---

### 2. motor_pwm_verification

**Purpose**: Hardware validation of motor PWM outputs using input capture

Shows how to:
- Verify auto-generated motor timer bank configuration
- Use TIM2 input capture to measure motor frequencies
- Validate timer separation (TIM1 vs TIM3)
- Implement hardware validation without oscilloscope

**Hardware**: JHEF411 (NOXE V3) or compatible F411 board

**Requires**: PWMOutputBank library (`libraries/TimerPWM`)

**Jumper Connections**:
- PA8 → PA0 (Motor1/TIM1 to TIM2_CH1 capture)
- PB0 → PA1 (Motor4/TIM3 to TIM2_CH2 capture)

**Validation Criteria**:
- Motor frequency: 1000 Hz ± 2% (980-1020 Hz)
- Demonstrates timer bank grouping from Betaflight converter
- Follows embedded hardware validation standards (see CLAUDE.md)

---

### 3. simple_pwm_test

**Purpose**: Baseline PWM test using direct HardwareTimer API

Shows how to:
- Configure PWM using STM32 HardwareTimer API directly
- Generate 1 kHz PWM signals on TIM1 and TIM3
- Validate timer output with GPIO polling
- Measure duty cycle accuracy

**Hardware**: Any F411 board (NUCLEO_F411RE, BLACKPILL_F411CE)

**Jumper Connections**:
- PA8 → PA0 (TIM1_CH1 output to test input)
- PB0 → PA1 (TIM3_CH3 output to test input)

**Use Case**:
- Reference implementation for working timer configuration
- Debugging tool for PWMOutputBank library issues
- Baseline test for timer hardware validation

**Note**: This example uses `HardwareTimer::setPWM()` which works correctly,
unlike PWMOutputBank which currently has initialization issues.

---

### 4. pwm_motor_servo

**Purpose**: Control motors and servos using TimerPWM library

Shows how to:
- Initialize servo timer banks (50 Hz standard PWM)
- Initialize motor timer banks (1000 Hz OneShot125)
- Attach channels with min/max pulse widths
- Control servo positions and motor speeds

**Hardware**: MATEKH743 (or any board with servo outputs)

**Requires**: TimerPWM library (`libraries/TimerPWM`)

**Connections**:
- Servos: PE5, PE6 (TIM15)
- Motors: PA0-PA3 (TIM5)

---

## Usage Pattern

1. **Generate BoardConfig** from Betaflight unified target:
   ```bash
   cd extras/betaflight_converter
   python3 convert.py data/YOUR-BOARD.config
   ```

2. **Include generated config** directly from output directory:
   ```cpp
   // Reference the generated file directly (always up-to-date)
   #include "../../output/JHEF-JHEF411.h"
   ```

   Or copy to your sketch folder if preferred:
   ```cpp
   #include "JHEF-JHEF411.h"
   ```

3. **Access configuration** via `BoardConfig` namespace:
   ```cpp
   auto pin = BoardConfig::storage.cs_pin;
   auto freq = BoardConfig::Motor::frequency_hz;
   ```

4. **Initialize hardware** using config values:
   ```cpp
   pinMode(BoardConfig::status_leds.led1_pin, OUTPUT);
   ```

## Configuration Structure

Generated configs include:

- **StorageConfig** - SPI flash or SD card
- **IMUConfig** - Gyro/accelerometer with interrupt
- **I2CConfig** - I2C peripherals (baro, mag, etc.)
- **UARTConfig** - Serial ports
- **ADCConfig** - Battery voltage/current monitoring
- **LEDConfig** - Status LEDs (GPIO)
- **Servo** namespace - PWM servo outputs (50 Hz)
- **Motor** namespace - High-speed motor outputs (DSHOT/OneShot)

## See Also

- Main converter README: `../README.md`
- Generated output files: `../output/`
- TimerPWM library: `../../../libraries/TimerPWM/`
