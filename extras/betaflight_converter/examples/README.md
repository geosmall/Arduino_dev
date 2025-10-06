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

### 2. pwm_motor_servo

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

2. **Copy generated files** to your sketch folder:
   - `output/YOUR-BOARD.h` - Generated BoardConfig
   - `../../../targets/config/ConfigTypes.h` - Config type definitions

   ```cpp
   #include "YOUR-BOARD.h"
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
