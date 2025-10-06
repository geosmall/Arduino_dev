/*
 * Auto-generated BoardConfig from Betaflight unified target
 * Generated: 2025-10-06 06:32:10
 * Generator: betaflight_target_converter.py
 */

#pragma once

// Copy this file to your sketch folder, or add targets/ to include path
#include "ConfigTypes.h"  // Expects ConfigTypes.h in same directory

// Board: JHEF411
// Manufacturer: JHEF
// MCU: STM32F411
// Gyro: MPU6000, ICM42688P
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI2
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB_15, PB_14, PB_13, PB_2, 8000000};

  // IMU: MPU6000, ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA_7, PA_6, PA_5, PA_4, 8000000, CS_Mode::HARDWARE};
  static constexpr IMUConfig imu{imu_spi, PB_3, 1000000};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB_8, PB_9, 400000};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PB_6, PB_7, 115200};

  // USART2: Serial port
  static constexpr UARTConfig uart2{PA_2, PA_3, 115200};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA_0, PA_1, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PC_13};

  // Motors: DSHOT300 protocol
  namespace Motor {
    static constexpr uint32_t frequency_hz = 1000;

    // TIM1 Bank: Motors 1, 2, 3
    namespace TIM1_Bank {
      static inline TIM_TypeDef* const timer = TIM1;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor1 = {PA_8, 1, 0, 0};  // TIM1_CH1
      static constexpr Channel motor2 = {PA_9, 2, 0, 0};  // TIM1_CH2
      static constexpr Channel motor3 = {PA_10, 3, 0, 0};  // TIM1_CH3
    };

    // TIM3 Bank: Motors 4, 5
    namespace TIM3_Bank {
      static inline TIM_TypeDef* const timer = TIM3;

      struct Channel {
        uint32_t pin;
        uint32_t ch;
        uint32_t min_us;
        uint32_t max_us;
      };

      static constexpr Channel motor4 = {PB_0, 3, 0, 0};  // TIM3_CH3
      static constexpr Channel motor5 = {PB_4, 1, 0, 0};  // TIM3_CH1
    };

  };
}