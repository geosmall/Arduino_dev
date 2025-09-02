#pragma once

#include <Arduino.h>
#include <SPI.h>

#ifdef __cplusplus
extern "C" {
#endif

// SPI SD card initialization function
// Returns true if successful, false otherwise
bool sd_spi_initialize(uint8_t cs_pin, SPIClass *spi_port);

// Get current SPI settings
uint8_t sd_spi_get_cs_pin(void);
SPIClass* sd_spi_get_port(void);

// SPI speed configuration (call before initialization)
// speed_hz: SPI frequency in Hz (e.g., 1000000 for 1MHz)
void sd_spi_set_speed(uint32_t speed_hz);
uint32_t sd_spi_get_speed(void);

// Predefined speed constants
#define SD_SPI_SPEED_SLOW    400000    // 400kHz - very safe for long wires
#define SD_SPI_SPEED_NORMAL  1000000   // 1MHz - breadboard safe
#define SD_SPI_SPEED_FAST    8000000   // 8MHz - production/short traces only

#ifdef __cplusplus
}
#endif