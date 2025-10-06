/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-10-06 06:17:04
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Copy this file to your sketch folder, or add targets/ to include path
#include "ConfigTypes.h"  // Expects ConfigTypes.h in same directory

// Board: MATEKH743
// Manufacturer: MTKS
// MCU: STM32H743
// Gyro: MPU6500, MPU6000, ICM42688P, ICM42605
namespace BoardConfig {
  // IMU: MPU6500, MPU6000, ICM42688P, ICM42605 on SPI1
  static constexpr SPIConfig imu_spi{PD_7, PA_6, PA_5, PC_15, 8000000, CS_Mode::HARDWARE};
  static constexpr IMUConfig imu{imu_spi, PB_2, 1000000};

  // I2C1: Airspeed sensor, external compass
  static constexpr I2CConfig airspeed{PB_6, PB_7, 400000};

  // I2C2: Barometer, compass
  static constexpr I2CConfig baro{PB_10, PB_11, 400000};

  // LPUART1: Serial port
  static constexpr UARTConfig uart1{PA_9, PA_10, 115200};

  // USART2: Serial port
  static constexpr UARTConfig uart2{PD_5, PD_6, 115200};

  // USART3: Serial port
  static constexpr UARTConfig uart3{PD_8, PD_9, 115200};

  // UART4: Serial port
  static constexpr UARTConfig uart4{PB_9, PB_8, 115200};

  // USART6: Serial port
  static constexpr UARTConfig uart6{PC_6, PC_7, 115200};

  // UART7: Serial port
  static constexpr UARTConfig uart7{PE_8, PE_7, 115200};

  // UART8: Serial port
  static constexpr UARTConfig uart8{PE_1, PE_0, 115200};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PC_0, PC_1, 110, 250};

  // Status LEDs
  static constexpr LEDConfig status_leds{PE_3, PE_4};

  // Servos: Standard PWM (50 Hz)
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    // TIM15 Bank: Servos 1, 2
    namespace TIM15_Bank {
      static inline TIM_TypeDef* const timer = TIM15;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel servo1 = {PE_5, 1, 1000, 2000};  // TIM15_CH1
      static constexpr Channel servo2 = {PE_6, 2, 1000, 2000};  // TIM15_CH2
    };

  };
  // Motors: ONESHOT125 protocol
  namespace Motor {
    static constexpr uint32_t frequency_hz = 1000;

    // TIM3 Bank: Motors 1, 2
    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor1 = {PB_0, 3, 125, 250};  // TIM3_CH3
      static constexpr Channel motor2 = {PB_1, 4, 125, 250};  // TIM3_CH4
    };

    // TIM4 Bank: Motors 7, 8
    namespace TIM4_Bank {
      static inline TIM_TypeDef* const timer = TIM4;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor7 = {PD_12, 1, 125, 250};  // TIM4_CH1
      static constexpr Channel motor8 = {PD_13, 2, 125, 250};  // TIM4_CH2
    };

    // TIM5 Bank: Motors 3, 4, 5, 6
    namespace TIM5_Bank {
      static inline TIM_TypeDef* const timer = TIM5;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor3 = {PA_0, 1, 125, 250};  // TIM5_CH1
      static constexpr Channel motor4 = {PA_1, 2, 125, 250};  // TIM5_CH2
      static constexpr Channel motor5 = {PA_2, 3, 125, 250};  // TIM5_CH3
      static constexpr Channel motor6 = {PA_3, 4, 125, 250};  // TIM5_CH4
    };

  };
}