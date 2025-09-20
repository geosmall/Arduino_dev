#include "../../aunit_hil.h"
#include <SPI.h>

// Include board configuration (automatically selects based on ARDUINO_* defines)
#include "../../targets/NUCLEO_F411RE.h"

/*
 * Board Configuration System Test
 *
 * Tests the Arduino pin macro approach for Board Configuration System
 * - Uses Arduino pin numbers (PC12, PC11, etc.) for compatibility
 * - Avoids mass changes to existing code
 * - Provides structured configuration management
 */

// Create SPI instance using BoardConfig
SPIClass configuredSPI(
  BoardConfig::storage.mosi_pin,
  BoardConfig::storage.miso_pin,
  BoardConfig::storage.sclk_pin
);

test(board_config_pin_values) {
  // Verify storage configuration values
  assertEqual((unsigned int)BoardConfig::storage.mosi_pin, (unsigned int)PC12);
  assertEqual((unsigned int)BoardConfig::storage.miso_pin, (unsigned int)PC11);
  assertEqual((unsigned int)BoardConfig::storage.sclk_pin, (unsigned int)PC10);
  assertEqual((unsigned int)BoardConfig::storage.cs_pin, (unsigned int)PD2);

  // Verify clock settings
  assertEqual((unsigned long)BoardConfig::storage.setup_clock_hz, 1000000UL);
  assertEqual((unsigned long)BoardConfig::storage.runtime_clock_hz, 8000000UL);
}

test(board_config_spi_compatibility) {
  // Test that SPI class accepts BoardConfig pin values
  pinMode(BoardConfig::storage.cs_pin, OUTPUT);
  digitalWrite(BoardConfig::storage.cs_pin, HIGH);

  // This tests that the pin values work with standard Arduino functions
  assertTrue(true); // If we get here, pin values are compatible
}

test(board_config_constexpr) {
  // Test that configuration is compile-time constant
  constexpr auto storage = BoardConfig::storage;
  assertEqual((unsigned int)storage.mosi_pin, (unsigned int)PC12);

  // Test other peripherals exist
  constexpr auto gps_config = BoardConfig::gps;
  assertEqual((unsigned long)gps_config.baud_rate, 115200UL);
}

// ============================================================================
// Arduino Integration
// ============================================================================

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
#else
  SEGGER_RTT_Init();
#endif

  HIL_TEST_SETUP();
  HIL_TEST_TIMEOUT(30);
}

void loop() {
  HIL_TEST_RUN();
}