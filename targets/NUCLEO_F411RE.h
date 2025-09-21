#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE development board configuration
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: No storage hardware attached by default on base Nucleo
  // Use NUCLEO_F411RE_LITTLEFS.h or NUCLEO_F411RE_SDFS.h for storage testing
  static constexpr StorageConfig storage{StorageBackend::NONE, 0, 0, 0, 0, 0, 0};

  // IMU: SPI connections via jumpers (reduced speed for reliability)
  static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 2000000};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}