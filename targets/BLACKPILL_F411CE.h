#pragma once
#include "config/ConfigTypes.h"

// BLACKPILL F411CE development board configuration
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: SPI flash hardwired (can use higher speed)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PA7, PA6, PA5, PA4, 8000000};

  // IMU: SPI connections via jumpers (reduced speed for reliability)
  static constexpr SPIConfig imu_spi{PB15, PB14, PB13, PB12, 1000000};
  static constexpr IMUConfig imu{imu_spi, PC13};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA2, PA3, 115200};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB7, PB6, 400000};
}