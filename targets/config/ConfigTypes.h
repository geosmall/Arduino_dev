#pragma once

// Configuration type definitions for board-specific settings
// Uses Arduino pin macros for compatibility with existing code

// Storage backend types
enum class StorageBackend {
    NONE,      // No storage hardware attached
    LITTLEFS,  // SPI flash storage
    SDFS       // SD card storage
};

// IMU bus transport types
enum class IMUTransport {
    NONE,      // No IMU hardware attached
    SPI,       // SPI bus communication
    I2C        // I2C bus communication
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
    IMUTransport transport;
    uint32_t int_pin;              // 0 = no interrupt
    uint32_t freq_override_hz;     // 0 = use bus default
    uint8_t i2c_address;           // For I2C transport

    union {
      SPIConfig spi;
      I2CConfig i2c;
    };

    // Factory constructors
    static constexpr IMUConfig spi_imu(const SPIConfig& spi_config,
                                      uint32_t freq_override = 0,
                                      uint32_t interrupt_pin = 0) {
      return {IMUTransport::SPI, interrupt_pin, freq_override, 0, {.spi = spi_config}};
    }

    static constexpr IMUConfig i2c_imu(const I2CConfig& i2c_config,
                                      uint8_t device_addr = 0x68,
                                      uint32_t freq_override = 0,
                                      uint32_t interrupt_pin = 0) {
      return {IMUTransport::I2C, interrupt_pin, freq_override, device_addr, {.i2c = i2c_config}};
    }

    static constexpr IMUConfig none() {
      return {IMUTransport::NONE, 0, 0, 0, {.spi = SPIConfig{0, 0, 0, 0, 0}}};
    }

    // Helper to get effective frequency
    uint32_t effective_frequency() const {
      if (freq_override_hz > 0) return freq_override_hz;
      return (transport == IMUTransport::SPI) ? spi.freq_hz : i2c.freq_hz;
    }
  };
}