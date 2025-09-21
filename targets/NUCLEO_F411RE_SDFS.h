#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE + SD Card HIL Test Rig Configuration
// Hardware: STM32F411RE Nucleo with SD card module/breakout
// Validated: Standard SD/SDHC cards via SPI interface
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: SD Card for SDFS (SPI module/breakout)
  static constexpr StorageConfig storage{StorageBackend::SDFS, PC12, PC11, PC10, PD2, 1000000, 8000000};
  //                                     Backend        MOSI  MISO  SCLK  CS   Setup   Runtime

  // IMU: Available SPI pins for sensors (if needed)
  static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 8000000};

  // GPS: UART communication (if needed)
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // I2C: Available for additional sensors (if needed)
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}