# SDFS - SD Card File System Library

SD card filesystem library for STM32 Arduino Core with Arduino FS.h interface compatibility.

## Features

- **FS.h Compatible**: API compatible with other Arduino filesystem libraries
- **FatFS Backend**: Built on FatFS library for file operations
- **SPI Interface**: SPI communication with automatic speed management (2MHz→4MHz)
- **Card Type Detection**: Automatic detection and handling of SD, SDHC, and SDXC cards
- **STM32 Support**: Designed for STM32 microcontrollers with Arduino Core

## Hardware Requirements

- STM32 microcontroller with Arduino Core support
- SD card reader with SPI interface
- Connections: MOSI, MISO, SCLK, CS

## Installation

1. Copy the `SDFS` folder to your Arduino libraries directory
2. Restart Arduino IDE
3. Include the library: `#include <SDFS.h>`

## Basic Usage

```cpp
#include <SDFS.h>

SDFS_SPI sdfs;

void setup() {
  Serial.begin(115200);
  
  // Configure SPI pins (for Nucleo F411RE)
  SPI.setMOSI(PC12);
  SPI.setMISO(PC11); 
  SPI.setSCLK(PC10);
  
  // Initialize SD card
  if (sdfs.begin(PD2)) {  // CS pin
    Serial.println("SD card initialized");
    
    // Write a file
    File file = sdfs.open("/test.txt", FILE_WRITE_BEGIN);
    if (file) {
      file.println("Hello SDFS!");
      file.close();
    }
    
    // Read the file
    file = sdfs.open("/test.txt", FILE_READ);
    if (file) {
      while (file.available()) {
        Serial.write(file.read());
      }
      file.close();
    }
  }
}
```

## Pin Configurations

### Nucleo F411RE
- MOSI: PC12
- MISO: PC11
- SCLK: PC10
- CS: PD2 (configurable)

### BlackPill F411CE
- MOSI: PA7
- MISO: PA6
- SCLK: PA5
- CS: PA4 (configurable)

## API Reference

### SDFS_SPI Class

#### Initialization
- `bool begin(uint8_t csPin)` - Initialize SD card with specified CS pin
- `void setSPISpeed(uint32_t speed)` - Set SPI speed (default: auto-optimized)

#### File Operations (FS.h interface)
- `File open(const char* path, uint8_t mode)` - Open file (FILE_READ, FILE_WRITE, FILE_WRITE_BEGIN)
- `bool exists(const char* path)` - Check if file/directory exists
- `bool remove(const char* path)` - Delete file
- `bool mkdir(const char* path)` - Create directory
- `bool rmdir(const char* path)` - Remove directory

#### Information
- `uint64_t totalSize()` - Get total card capacity
- `uint64_t usedSize()` - Get used space
- `bool mediaPresent()` - Check if card is present
- `const char* getMediaName()` - Get media type string

## Performance

- Initialization: 2MHz 
- Normal operation: 4MHz
- Automatic speed negotiation based on card capabilities

## Compatibility

- Tested with SD cards (2GB and below)
- Tested with SDHC cards (4GB to 32GB)
- Compatible with FAT16 and FAT32 filesystems

## License

This library is released under the same license as the STM32 Arduino Core project.