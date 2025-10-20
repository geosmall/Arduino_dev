#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE development board configuration
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: No storage hardware attached by default on base Nucleo
  // Use NUCLEO_F411RE_LITTLEFS.h or NUCLEO_F411RE_SDFS.h for storage testing
  static constexpr StorageConfig storage{StorageBackend::NONE, 0, 0, 0, 0, 0};

  // IMU: SPI connections via jumpers (reduced speed for reliability)
  // Single frequency for development/testing
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
  static constexpr IMUConfig imu{imu_spi, PC4};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // RC Receiver: IBus on USART1 (RX=PA10, TX=PA9)
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 115200, 1000, 300};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}