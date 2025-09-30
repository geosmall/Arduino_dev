#pragma once

// Configuration type definitions for board-specific settings
// Uses Arduino pin macros for compatibility with existing code

// Include STM32 pinmap for NP definition (No Pin)
#include "stm32/pinmap.h"
// Include pins_arduino for PNUM_NOT_DEFINED
#include "pins_arduino.h"

// Storage backend types
enum class StorageBackend {
    NONE,      // No storage hardware attached
    LITTLEFS,  // SPI flash storage
    SDFS       // SD card storage
};


// SPI chip select control modes
enum class CS_Mode {
    SOFTWARE,  // Software-controlled CS pin
    HARDWARE   // Hardware-controlled CS via SPI peripheral
};

namespace BoardConfig {
  struct SPIConfig {
    constexpr SPIConfig(uint32_t mosi, uint32_t miso, uint32_t sclk, uint32_t cs,
                       uint32_t frequency_hz = 1000000,
                       CS_Mode mode = CS_Mode::SOFTWARE)
      : mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk), cs_pin(cs),
        freq_hz(frequency_hz), cs_mode(mode) {}

    const uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t freq_hz;
    const CS_Mode cs_mode;

    // Helper: Get SSEL pin for SPIClass constructor
    // SW mode: returns PNUM_NOT_DEFINED (disables hardware SSEL)
    // HW mode: returns cs_pin (STM32 SPI peripheral controls CS)
    constexpr uint32_t get_ssel_pin() const {
      return (cs_mode == CS_Mode::HARDWARE) ? cs_pin : PNUM_NOT_DEFINED;
    }
  };

  struct UARTConfig {
    constexpr UARTConfig(uint32_t tx, uint32_t rx, uint32_t baud)
      : tx_pin(tx), rx_pin(rx), baud_rate(baud) {}

    const uint32_t tx_pin, rx_pin;
    const uint32_t baud_rate;
  };

  struct I2CConfig {
    constexpr I2CConfig(uint32_t sda, uint32_t scl, uint32_t frequency_hz = 100000)
      : sda_pin(sda), scl_pin(scl), freq_hz(frequency_hz) {}

    const uint32_t sda_pin, scl_pin;
    const uint32_t freq_hz;
  };

  struct StorageConfig {
    constexpr StorageConfig(StorageBackend backend, uint32_t mosi, uint32_t miso,
                           uint32_t sclk, uint32_t cs, uint32_t frequency_hz = 1000000)
      : backend_type(backend), mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk),
        cs_pin(cs), freq_hz(frequency_hz) {}

    const StorageBackend backend_type;
    const uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t freq_hz;
  };

  struct IMUConfig {
    constexpr IMUConfig(const SPIConfig& spi_config, uint32_t interrupt_pin = 0,
                       uint32_t setup_freq_hz = 0)
      : spi(spi_config), int_pin(interrupt_pin), setup_freq_hz(setup_freq_hz) {}

    const SPIConfig spi;
    const uint32_t int_pin;        // 0 = no interrupt
    const uint32_t setup_freq_hz;  // 0 = use spi.freq_hz for setup (slow initialization)

    // Helper: Get effective setup frequency (slow initialization)
    constexpr uint32_t get_setup_freq() const {
      return (setup_freq_hz > 0) ? setup_freq_hz : spi.freq_hz;
    }

    // Helper: Get effective runtime frequency (normal operation)
    constexpr uint32_t get_runtime_freq() const {
      return spi.freq_hz;  // Always use the SPI config frequency for runtime
    }
  };
}