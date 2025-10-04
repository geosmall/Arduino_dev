#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE + SPI Flash HIL Test Rig Configuration
// Hardware: STM32F411RE Nucleo with SPI flash breadboard
// Validated: W25Q128JV 16MB SPI flash chip
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: SPI Flash for LittleFS (breadboard setup)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PC12, PC11, PC10, PD2, 1000000};
  //                                 MOSI  MISO  SCLK  CS   Freq

  // IMU: Available SPI pins for sensors (if needed)
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
  static constexpr IMUConfig imu{imu_spi, PC4};

  // GPS: UART communication (if needed)
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Available for additional sensors (if needed)
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

  // Servo: TIM3 @ 50 Hz (hardware validated with PWM_Verification)
  namespace Servo {
    static inline TIM_TypeDef* const timer = TIM3;
    static constexpr uint32_t frequency_hz = 50;

    struct Channel {
      uint32_t pin;
      uint32_t ch;
      uint32_t min_us;
      uint32_t max_us;
    };

    static constexpr Channel pwm_output = {PB4, 1, 1000, 2000};  // TIM3_CH1 (D5) - PWM output
    static constexpr Channel input_capture = {PA0, 1, 1000, 2000};  // TIM2_CH1 (A0) - Input capture
  };
}