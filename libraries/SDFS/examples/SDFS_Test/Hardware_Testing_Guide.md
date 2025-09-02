# SDFS Hardware Testing Guide

This guide provides instructions for testing the SDFS library on real hardware with a Nucleo F411RE board and SD card breakout.

## Overview

The SDFS_Test example provides comprehensive validation of the SDFS library's core functionality, including SPI communication, FatFs filesystem operations, and basic file I/O. This Phase 1 implementation contains sufficient functionality for meaningful hardware testing.

## Hardware Requirements

### Required Components
- **STM32 Nucleo F411RE** board (or compatible STM32F411 board)
- **SD card breakout board** (3.3V compatible)
- **SD card** (FAT32 formatted recommended, 32GB or smaller)
- **Jumper wires** for connections
- **USB cable** for programming and serial monitor

### SD Card Recommendations
- **Format**: FAT32 (most compatible with FatFs)
- **Size**: 32GB or smaller (for FAT32 compatibility)
- **Speed Class**: Class 4 or higher recommended
- **Type**: Standard SD or microSD with adapter

## Wiring Connections

### Nucleo F411RE Pin Configuration

The SDFS_Test example is configured to use **SPI3** peripheral with the following connections:

```
SD Card Breakout    →    Nucleo F411RE Pin
VCC (3.3V)          →    3.3V
GND                 →    GND
CS (Chip Select)    →    PD2
MOSI (Data Out)     →    PC12
MISO (Data In)      →    PC11
SCLK (Clock)        →    PC10
```

### Alternative Board Support

For **BlackPill F411CE** boards, the example automatically switches to **SPI1**:

```
SD Card Breakout    →    BlackPill F411CE Pin
VCC (3.3V)          →    3.3V
GND                 →    GND
CS (Chip Select)    →    PA4
MOSI (Data Out)     →    PA7
MISO (Data In)      →    PA6
SCLK (Clock)        →    PA5
```

### Wiring Tips
- **Power**: Use 3.3V supply (5V may damage SD cards)
- **Connections**: Keep wires short to minimize SPI timing issues
- **CS Pin**: Ensure good connection as it's critical for SPI communication
- **Pullup**: Most breakout boards have built-in pullups on necessary pins

## Testing Procedure

### 1. Hardware Setup
1. Wire SD card breakout to Nucleo F411RE as shown above
2. Insert formatted SD card into breakout board
3. Connect Nucleo to computer via USB

### 2. Software Setup
1. Open Arduino IDE 1.8.19 or use arduino-cli
2. Select board: **STMicroelectronics STM32 → Nucleo-64 → Nucleo F411RE**
3. Open SDFS_Test example: `File → Examples → SDFS → SDFS_Test`
4. Upload sketch to board

### 3. Serial Monitor Testing
1. Open Serial Monitor at **115200 baud**
2. Press reset button on Nucleo board
3. Observe test output

## Expected Test Results

### Successful Operation
```
SDFS Test Starting...
SDFS initialization successful!
Media: SD Card (SPI)
Total Size: 32212254720 bytes
Used Size: 147456 bytes
Free Size: 32212107264 bytes

=== Testing Basic File Operations ===
Created test file successfully
Written data to test file
Reading test file contents:
--- File Contents ---
Hello from SDFS!
Current millis: 12345
--- End of File ---
Test file size: 32 bytes
=== Basic File Operations Complete ===

SDFS Test running... (heartbeat every 5s)
```

### Initialization Failure
```
SDFS Test Starting...
SDFS initialization failed!
Check SD card connection and format (FAT32 recommended)
```

## Functionality Tested

### ✅ Core Operations Validated
- **SPI Communication**: Low-level SD card protocol
- **Card Detection**: Presence and compatibility check
- **Filesystem Mount**: FatFs mounting and filesystem access
- **File Creation**: Creating new files on SD card
- **File Writing**: Writing text and numeric data
- **File Reading**: Reading back written data
- **File Management**: Size queries and existence checks
- **Data Integrity**: Write→read verification
- **Error Handling**: Clear success/failure reporting

### 📋 Not Yet Implemented (Future Phases)
- Directory operations (`mkdir`, directory listing)
- File operations (`remove`, `rename`)
- Format functionality
- Advanced timestamps
- Multiple file handling

## Troubleshooting

### Common Issues and Solutions

#### 1. "SDFS initialization failed!"
**Possible Causes:**
- Incorrect wiring (check all connections)
- SD card not inserted or faulty
- SD card format not supported (try FAT32)
- Power supply issues (ensure 3.3V)
- SPI timing issues (try shorter wires)

**Debug Steps:**
1. Verify all wiring connections
2. Try different SD card
3. Format card as FAT32 on computer
4. Check power supply voltage
5. Add delay after power-on

#### 2. "Failed to create test file"
**Possible Causes:**
- SD card write-protected
- Insufficient space on card
- Filesystem corruption
- Card compatibility issues

**Debug Steps:**
1. Check write-protect tab on SD card
2. Verify free space available
3. Reformat card as FAT32
4. Try different brand/type of card

#### 3. Serial Monitor Shows Nothing
**Possible Causes:**
- Wrong baud rate (should be 115200)
- USB cable or port issues
- Board not properly connected
- Sketch not uploaded correctly

**Debug Steps:**
1. Verify Serial Monitor baud rate
2. Try different USB cable/port
3. Re-upload sketch
4. Press reset button on board

### Hardware Debugging Tips

1. **Use Multimeter**: Verify 3.3V power supply voltage
2. **Check Continuity**: Ensure all wiring connections are solid
3. **Try Different Cards**: Test with known-good SD cards
4. **Reduce SPI Speed**: Modify `sd_spi_diskio.cpp` for slower clock if needed
5. **Add Debug Output**: Uncomment debug prints in SPI layer for detailed analysis

## Performance Expectations

### Memory Usage
- **Flash**: ~29KB (5% of STM32F411RE)
- **RAM**: ~2KB (1% of STM32F411RE)
- **Stack**: Minimal additional usage during operations

### Speed Performance
- **Initialization**: 1-3 seconds typical
- **File Creation**: <100ms per file
- **Read/Write**: Several KB/s typical for SPI mode
- **SD Card Class**: Higher class cards will be faster

### Resource Usage
- **SPI Peripheral**: Uses one SPI peripheral (SPI3 on Nucleo)
- **GPIO Pins**: Uses 4 pins (CS, MOSI, MISO, SCLK)
- **Timers**: No timers required
- **Interrupts**: No interrupts used (polling mode)

## Success Criteria

### Phase 1 Testing Goals
The hardware test is successful if:
- ✅ SD card initializes without errors
- ✅ File creation succeeds
- ✅ Data can be written to file
- ✅ Data can be read back correctly
- ✅ File size is reported accurately
- ✅ No system crashes or hangs occur

### Ready for Next Phases
If Phase 1 testing succeeds, the hardware setup is validated and ready for implementing additional phases:
- **Phase 2**: Directory operations and file management
- **Phase 3**: Advanced features (format, timestamps)
- **Phase 4**: Performance optimization and examples

## Additional Resources

- **FatFs Integration**: See `FatFs_Integration.md` for technical details
- **LittleFS Comparison**: Compare with LittleFS examples for API compatibility
- **STM32 SPI**: Reference STM32F411 datasheet for SPI peripheral details
- **SD Card Specs**: Consult SD card specifications for compatibility info

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2024-08-21 | 1.0.0 | Initial hardware testing guide for Phase 1 |