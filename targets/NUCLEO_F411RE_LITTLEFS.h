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
  static constexpr IMUConfig imu = IMUConfig::spi_imu(imu_spi, 0, PC4);

  // GPS: UART communication (if needed)
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Available for additional sensors (if needed)
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}