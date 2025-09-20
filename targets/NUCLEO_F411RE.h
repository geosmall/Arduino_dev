#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE development board configuration
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: SPI flash for LittleFS (development/testing)
  static constexpr SPIConfig storage{PC12, PC11, PC10, PD2, 1000000, 8000000};

  // IMU: Accelerometer/Gyroscope via SPI
  static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 8000000};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}