# SDFS Hardware Testing Guide

This guide provides instructions for testing the SDFS library on real hardware with STM32 boards and SD card breakouts.

## Overview

The SDFS library provides SD card filesystem implementation with FS.h API compatibility. This testing guide validates SPI communication, FatFS filesystem operations, and file I/O functionality.

## Hardware Requirements

### Required Components
- **STM32 Nucleo F411RE** or **BlackPill F411CE** board
- **SD card breakout board** (3.3V compatible)
- **SD card** (SDHC cards up to 32GB supported)
- **Jumper wires** for connections
- **USB cable** for programming and serial monitor

### SD Card Recommendations
- **Format**: FAT32 (fully compatible with FatFS backend)
- **Size**: Up to 32GB (SDHC supported)
- **Speed Class**: Class 4 or higher recommended
- **Type**: Standard SD, microSD, or SDHC cards

## Wiring Connections

### Nucleo F411RE Configuration

The SDFS library uses **SPI3** peripheral on Nucleo F411RE:

```
SD Card Breakout    →    Nucleo F411RE Pin
VCC (3.3V)          →    3.3V
GND                 →    GND
CS (Chip Select)    →    PD2
MOSI (Data Out)     →    PC12
MISO (Data In)      →    PC11
SCLK (Clock)        →    PC10
```

### BlackPill F411CE Configuration

For **BlackPill F411CE** boards, the library automatically uses **SPI1**:

```
SD Card Breakout    →    BlackPill F411CE Pin
VCC (3.3V)          →    3.3V
GND                 →    GND
CS (Chip Select)    →    PA4
MOSI (Data Out)     →    PA7
MISO (Data In)      →    PA6
SCLK (Clock)        →    PA5
```

### Wiring Best Practices
- **Power**: Always use 3.3V supply (5V will damage SD cards)
- **Connections**: Keep wires short (<10cm) for reliable SPI communication
- **CS Pin**: Ensure solid connection - critical for proper operation
- **Pullups**: Most breakout boards include required pullup resistors

## Testing Procedure

### 1. Hardware Setup
1. Wire SD card breakout to your STM32 board as shown above
2. Insert formatted SD card into breakout board
3. Connect board to computer via USB

### 2. Software Setup
1. Open Arduino IDE or use arduino-cli
2. Select board: **STMicroelectronics STM32 → Nucleo-64 → Nucleo F411RE**
3. Open SDFS_Test example: `File → Examples → SDFS → SDFS_Test`
4. Upload sketch to board

### 3. Serial Monitor Testing
1. Open Serial Monitor at **115200 baud**
2. Press reset button on board
3. Observe comprehensive test output

## Expected Test Results

### Successful Operation
```
SDFS Library Example
====================
Initializing SD card... SUCCESS
Media: SD Card (SPI)
Total Size: 30436 MB
Used Size: 144 MB

Testing file operations:
Writing test file... OK
Reading test file... OK
File contents:
Hello from SDFS!
Line 2

Root directory listing:
DIR  SYSTEM~1
FILE test.txt (23 bytes)
FILE LAGER.CFG (2048 bytes)

All tests completed!
```

### Initialization Failure
```
SDFS Library Example
====================
Initializing SD card... FAILED
Check connections and card insertion
```

## Functionality Tested

### ✅ Feature Validation
- **SPI Communication**: Arduino SPI integration (2MHz→4MHz)
- **Card Detection**: Automatic SDHC/SD type detection and speed optimization  
- **Filesystem Mount**: FatFS mounting with filesystem access
- **File Operations**: Create, read, write, append, delete operations
- **Directory Operations**: Create, remove, list directory contents
- **File Information**: Size, existence, type queries
- **Data Integrity**: Write→read verification
- **Error Handling**: Error reporting and recovery
- **Multi-Board Support**: Automatic pin configuration for different boards

### 🎯 Features
- **FS.h API Compatibility**: Compatible with LittleFS API
- **Automatic Speed Optimization**: 2MHz initialization → 4MHz operation
- **FatFS Backend**: Reliable filesystem implementation
- **Memory Efficient**: ~29KB flash, ~2KB RAM usage
- **No Interrupts Required**: Polling-based operation

## Troubleshooting

### Common Issues and Solutions

#### 1. "Initializing SD card... FAILED"
**Root Causes:**
- Incorrect wiring (most common)
- SD card not inserted or defective
- Incompatible SD card format
- Power supply insufficient (must be 3.3V)
- SPI timing issues on long wires

**Resolution Steps:**
1. Double-check all wiring connections against pin diagrams
2. Try a different, known-good SD card
3. Format card as FAT32 using computer
4. Verify 3.3V power supply with multimeter
5. Shorten jumper wires to <10cm
6. Try slower SPI speed by calling `sdfs.setSPISpeed(1000000)` before `begin()`

#### 2. "Writing test file... FAILED"
**Root Causes:**
- SD card write-protected
- Card full or corrupted filesystem
- Hardware connection issues during operation

**Resolution Steps:**
1. Check SD card write-protect tab position
2. Verify sufficient free space on card
3. Reformat card as FAT32
4. Check for loose connections during operation

#### 3. Serial Output Corruption or Missing
**Root Causes:**
- Incorrect Serial Monitor baud rate
- USB connection issues
- Board not properly reset after upload

**Resolution Steps:**
1. Set Serial Monitor to exactly 115200 baud
2. Try different USB cable or port
3. Press reset button after opening Serial Monitor
4. Re-upload sketch if necessary

### Advanced Debugging

#### SPI Signal Analysis
- Use oscilloscope to verify SPI signals if available
- Check for proper clock signal on SCLK pin
- Verify CS signal goes LOW during communication
- MOSI should show command data, MISO should show responses

#### Performance Optimization
- For breadboard setups, reduce to 1MHz: `sdfs.setSPISpeed(1000000)`
- Use shorter, higher-quality jumper wires
- Add 100nF bypass capacitor near SD card VCC/GND
- Ensure stable 3.3V power supply with low ripple

## Performance Specifications

### Memory Usage
- **Flash**: ~29KB (5% of STM32F411RE, 15% reduction from debug version)
- **RAM**: ~2KB dynamic (1% of STM32F411RE)
- **Stack**: <1KB additional during file operations

### Speed Performance
- **Initialization**: 1-2 seconds typical (automatic speed negotiation)
- **File Creation**: <50ms per file
- **Read/Write**: 100-500 KB/s depending on card class and SPI speed
- **Directory Operations**: <10ms for typical directories

### Compatibility
- **SD Cards**: 2GB and smaller (FAT16/FAT32)
- **SDHC Cards**: 4GB to 32GB (FAT32)
- **File Systems**: FAT16, FAT32 (automatic detection)
- **Arduino Core**: STM32 Arduino Core v2.7.1+

## Success Criteria

### Testing Validation
The hardware test is successful if all items pass:
- ✅ SD card initializes without errors
- ✅ Card information is displayed correctly (size, type)
- ✅ File creation and writing succeed
- ✅ File reading returns correct data
- ✅ Directory listing shows files
- ✅ No system crashes or hangs occur
- ✅ Memory usage is within specifications

### Use Cases
If all tests pass, the SDFS library is validated for:
- **Flight Controller Data Logging**: Real-time sensor data storage
- **Configuration Management**: Settings and parameter storage
- **Firmware Updates**: Storing and loading firmware images
- **General Embedded Applications**: Any FS.h compatible use case

## Integration Examples

### Basic Usage Pattern
```cpp
#include <SDFS.h>
SDFS_SPI sdfs;

void setup() {
  // Configure SPI pins
  SPI.setMOSI(PC12); SPI.setMISO(PC11); SPI.setSCLK(PC10);
  
  if (sdfs.begin(PD2)) {
    // Ready for file operations
    File dataLog = sdfs.open("/flight_data.log", FILE_WRITE);
    dataLog.println("Flight log started");
    dataLog.close();
  }
}
```

### Drop-in LittleFS Replacement
```cpp
// Simply replace LittleFS_SPIFlash with SDFS_SPI
// #include <LittleFS.h>
// LittleFS_SPIFlash myfs;

#include <SDFS.h>
SDFS_SPI myfs;  // Same API, different storage medium
```

## Additional Resources

- **SDFS Library Documentation**: See `libraries/SDFS/README.md`
- **FS.h API Reference**: Arduino FS library documentation
- **STM32 Arduino Core**: [stm32duino/Arduino_Core_STM32](https://github.com/stm32duino/Arduino_Core_STM32)
- **FatFS Documentation**: [elm-chan.org/fsw/ff](http://elm-chan.org/fsw/ff/)

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2024-08-21 | 1.0.0 | Initial hardware testing guide |
| 2025-09-10 | 2.0.0 | Updated for completed SDFS library |