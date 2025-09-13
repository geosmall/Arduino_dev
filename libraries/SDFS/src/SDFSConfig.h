#ifndef SDFS_CONFIG_H
#define SDFS_CONFIG_H

// SDFS Configuration File
// This file defines configurable constants to replace hardcoded magic numbers
// Following SdFat library patterns for professional configuration management

//==============================================================================
// SPI Communication Settings
//==============================================================================

// Initial SPI speed for SD card initialization (400kHz standard)
#ifndef SDFS_SPI_INIT_SPEED_HZ
#define SDFS_SPI_INIT_SPEED_HZ 400000
#endif

// Maximum SPI speed for data transfer (4MHz safe default for most cards)
#ifndef SDFS_SPI_MAX_SPEED_HZ
#define SDFS_SPI_MAX_SPEED_HZ 4000000
#endif

// Medium speed for SDHC detection phase (2MHz)
#ifndef SDFS_SPI_MID_SPEED_HZ
#define SDFS_SPI_MID_SPEED_HZ 2000000
#endif

//==============================================================================
// Timeout Settings
//==============================================================================

// SD card initialization timeout in milliseconds
#ifndef SDFS_INIT_TIMEOUT_MS
#define SDFS_INIT_TIMEOUT_MS 1000
#endif

// Command response timeout in milliseconds
#ifndef SDFS_CMD_TIMEOUT_MS
#define SDFS_CMD_TIMEOUT_MS 500
#endif

// Data block read/write timeout in milliseconds
#ifndef SDFS_DATA_TIMEOUT_MS
#define SDFS_DATA_TIMEOUT_MS 300
#endif

// Busy wait timeout in milliseconds
#ifndef SDFS_BUSY_TIMEOUT_MS
#define SDFS_BUSY_TIMEOUT_MS 100
#endif

//==============================================================================
// Buffer and Path Settings
//==============================================================================

// Maximum file path length
#ifndef SDFS_MAX_PATH_LENGTH
#define SDFS_MAX_PATH_LENGTH 256
#endif

// SD card sector size (typically 512 bytes, but can be runtime detected)
#ifndef SDFS_SECTOR_SIZE
#define SDFS_SECTOR_SIZE 512
#endif

// Number of retries for failed operations
#ifndef SDFS_MAX_RETRIES
#define SDFS_MAX_RETRIES 3
#endif

//==============================================================================
// Debug and Logging Settings
//==============================================================================

// Enable debug logging (0=disabled, 1=basic, 2=verbose)
#ifndef SDFS_DEBUG_LEVEL
#define SDFS_DEBUG_LEVEL 0
#endif

// Enable RTT debugging support
#ifndef SDFS_USE_RTT
#define SDFS_USE_RTT 0
#endif

//==============================================================================
// Platform-Specific Settings
//==============================================================================

// Default CS pin (can be overridden at begin())
#ifndef SDFS_DEFAULT_CS_PIN
#define SDFS_DEFAULT_CS_PIN 10
#endif

// SPI mode for SD card communication (Mode 0 is standard)
#ifndef SDFS_SPI_MODE
#define SDFS_SPI_MODE SPI_MODE0
#endif

#endif // SDFS_CONFIG_H