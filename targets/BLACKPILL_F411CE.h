#pragma once
#include "config/ConfigTypes.h"

// BLACKPILL F411CE development board configuration
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: SPI flash hardwired (can use higher speed)
  static constexpr SPIConfig storage{PA7, PA6, PA5, PA4, 1000000, 8000000};

  // IMU: SPI connections via jumpers (reduced speed for reliability)
  static constexpr SPIConfig imu{PB15, PB14, PB13, PB12, 1000000, 2000000};

  // GPS: UART communication
  static constexpr UARTConfig gps{PA2, PA3, 115200};

  // I2C: Magnetometer, barometer
  static constexpr I2CConfig sensors{PB7, PB6, 400000};
}