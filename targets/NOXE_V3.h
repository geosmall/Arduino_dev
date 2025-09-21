#pragma once
#include "config/ConfigTypes.h"

// NOXE V3 flight controller production configuration
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: High-speed SPI flash for configuration and logging (LittleFS backend)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB12, 1000000, 8000000};

  // IMU: High-performance IMU for flight control
  static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 8000000};

  // GPS: High-precision GPS module
  static constexpr UARTConfig gps{PC6, PC7, 115200};

  // I2C: Environmental sensors
  static constexpr I2CConfig sensors{PB6, PB7, 400000};
}