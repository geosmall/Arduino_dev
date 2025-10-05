#pragma once
#include "config/ConfigTypes.h"

// NOXE V3 (JHEMCU F4 JHEF411) flight controller production configuration
// Pin assignments verified against Betaflight unified target and hardware wiring diagram
// Reference: https://github.com/betaflight/unified-targets JHEF-JHEF411.config
namespace BoardConfig {
  // Storage: W25Q128FV SPI flash on SPI2 (128Mbit / 16MB)
  // Used for configuration and blackbox logging (LittleFS backend)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB02, 8000000};

  // IMU: ICM42688P or MPU6000 on SPI1
  // Hardware CS with dual-frequency: 1MHz setup, 8MHz runtime (MPU-6000 pattern)
  // Interrupt on PB03 for data ready signal
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000, CS_Mode::HARDWARE};
  static constexpr IMUConfig imu{imu_spi, PB03, 1000000};

  // I2C1: Environmental sensors (barometer BMP280/DPS310, magnetometer)
  static constexpr I2CConfig sensors{PB08, PB09, 400000};

  // UART1: Primary serial port (RX receiver or MSP configurator)
  // TX=PB06, RX=PB07
  static constexpr UARTConfig uart1{PB06, PB07, 115200};

  // UART2: Secondary serial port (GPS, telemetry, or VTX control)
  // TX=PA02, RX=PA03
  static constexpr UARTConfig uart2{PA02, PA03, 115200};
}