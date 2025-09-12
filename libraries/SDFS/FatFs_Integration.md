# FatFs Integration Documentation

This document explains how the SDFS library integrates with the FatFs filesystem and provides guidance for future maintenance.

## Overview

The SDFS library provides an Arduino-compatible interface for SD card access using the FatFs filesystem library with a custom SPI disk I/O layer. The implementation follows the LittleFS API pattern to allow seamless switching between SPI flash (LittleFS) and SD card (SDFS) storage.

## FatFs Source Files

### Current Version
- **FatFs Version**: R0.15 (Revision 80286)
- **Source Date**: Copied from STMicroelectronics FatFs library v4.0.0
- **Original Author**: ChaN (http://elm-chan.org/fsw/ff/)

### Files Included

The following FatFs files are embedded in `src/fatfs/`:

| File | Source | Purpose |
|------|--------|---------|
| `ff.h` | FatFs core | Main FatFs header file |
| `ff.c` | FatFs core | Main FatFs implementation |
| `ffconf.h` | Modified from `ffconf_template.h` | FatFs configuration |
| `diskio.h` | FatFs core | Disk I/O abstraction layer header |
| `integer.h` | FatFs core | FatFs integer type definitions |

### Configuration Changes

The `ffconf.h` file has been modified from the original template:

```c
#define FF_USE_MKFS    1    // Enable f_mkfs() for formatting support
```

All other settings remain at their default values for basic functionality.

## Architecture Overview

### Layer Structure

```
┌─────────────────────────────────────────┐
│            Arduino Sketch               │
├─────────────────────────────────────────┤
│              SDFS Library               │
│  ┌──────────────┐  ┌─────────────────┐  │
│  │     SDFS     │  │    SDFSFile     │  │
│  │     (FS)     │  │   (FileImpl)    │  │
│  └──────────────┘  └─────────────────┘  │
├─────────────────────────────────────────┤
│               FatFs Core                │
│  ┌───────────────────────────────────┐  │
│  │           ff.c / ff.h             │  │
│  └───────────────────────────────────┘  │
├─────────────────────────────────────────┤
│           Disk I/O Abstraction          │
│  ┌───────────────────────────────────┐  │
│  │         sd_spi_diskio.cpp         │  │
│  └───────────────────────────────────┘  │
├─────────────────────────────────────────┤
│            Hardware Layer               │
│  ┌───────────────────────────────────┐  │
│  │       STM32 SPI + SD Card         │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

### FatFs Disk I/O Abstraction Layer

FatFs uses a standardized disk I/O interface defined in `diskio.h`. Our implementation in `sd_spi_diskio.cpp` provides:

#### Required Functions

```c
DSTATUS disk_initialize(BYTE pdrv);    // Initialize disk drive
DSTATUS disk_status(BYTE pdrv);        // Get disk status  
DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);    // Read sectors
DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);  // Write sectors
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);   // I/O control operations
DWORD get_fattime(void);               // Get current time for timestamps
```

#### Drive Mapping

- **Drive 0**: SPI SD Card (configured via `SDFS_SPI::begin()`)
- **Drives 1-9**: Not used (return `STA_NOINIT`)

#### SPI SD Card Implementation

The `sd_spi_diskio.cpp` file implements:

1. **Low-level SPI Communication**: Direct SPI commands to SD card
2. **SD Card Initialization**: CMD0, CMD8, ACMD41 sequence  
3. **Block Read/Write**: Single and multiple block operations
4. **Error Handling**: Timeout and response validation

Key features:
- **Configurable SPI**: CS pin and SPI port selection
- **Speed Management**: Slow initialization (400kHz), fast operation (4MHz for SDHC)
- **Standard Compliance**: SD Card specification v2.0 commands

## API Mapping

### SDFS to FatFs Function Mapping

| SDFS Method | FatFs Function | Purpose |
|-------------|----------------|---------|
| `open()` | `f_open()` | Open files and directories |
| `read()` | `f_read()` | Read file data |
| `write()` | `f_write()` | Write file data |
| `seek()` | `f_lseek()` | File positioning |
| `mkdir()` | `f_mkdir()` | Create directory |
| `remove()` | `f_unlink()` | Delete files/directories |
| `rename()` | `f_rename()` | Rename files/directories |
| `exists()` | `f_stat()` | Check file existence |
| `format()` | `f_mkfs()` | Format filesystem |
| `usedSize()` | `f_getfree()` | Get space usage |

### File Handle Management

- **FIL structures**: Dynamically allocated for file operations
- **DIR structures**: Dynamically allocated for directory operations  
- **Reference Counting**: Handled by Arduino FS.h FileImpl base class

## Updating FatFs

### When to Update

Consider updating FatFs when:
- Security vulnerabilities are discovered
- New features are needed (exFAT, long filename support, etc.)
- Bug fixes are available
- Performance improvements are released

### Update Procedure

1. **Download Latest FatFs**
   ```bash
   wget http://elm-chan.org/fsw/ff/arc/ff15.zip
   unzip ff15.zip
   ```

2. **Backup Current Implementation**
   ```bash
   cp -r src/fatfs src/fatfs_backup_$(date +%Y%m%d)
   ```

3. **Replace Core Files**
   ```bash
   cp ff15/source/ff.h src/fatfs/
   cp ff15/source/ff.c src/fatfs/
   cp ff15/source/diskio.h src/fatfs/
   cp ff15/source/integer.h src/fatfs/
   ```

4. **Update Configuration**
   - Compare `ff15/source/ffconf.h` with `src/fatfs/ffconf.h`
   - Merge any new configuration options
   - Ensure `FF_USE_MKFS 1` remains enabled
   - Check for API changes in configuration structure

5. **Check API Compatibility**
   - Review `ff15/doc/updates.txt` for breaking changes
   - Update SDFS wrapper functions if needed
   - Pay attention to:
     - Function signature changes
     - New error codes
     - Configuration option changes

6. **Test Compilation**
   ```bash
   arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE examples/SDFS_Test
   ```

7. **Validate Functionality**
   - Test basic file operations (create, read, write, delete)
   - Test directory operations  
   - Test formatting
   - Verify no memory leaks
   - Check performance impact

8. **Update Documentation**
   - Update version information in this file
   - Document any API changes
   - Update examples if needed

### Common Update Issues

1. **Configuration Changes**
   - New options may be added to `ffconf.h`
   - Some options may be renamed or deprecated
   - Solution: Carefully merge configurations

2. **API Changes**  
   - Function parameters may change
   - New error codes may be introduced
   - Solution: Update wrapper functions in SDFS.cpp

3. **Memory Layout Changes**
   - FIL/DIR structure sizes may change
   - Solution: Usually transparent due to dynamic allocation

4. **Disk I/O Interface Changes**
   - diskio.h interface is stable but may add new commands
   - Solution: Implement new commands or return appropriate errors

### Version History

| Date | FatFs Version | Notes |
|------|---------------|-------|
| 2024-08-21 | R0.15 (80286) | Initial implementation from STM32 FatFs v4.0.0 |
| 2024-09-09 | R0.15 (80286) | Production release with SDHC optimization |

## Troubleshooting

### Common Issues

1. **Compilation Errors**
   - Check `ffconf.h` configuration matches FatFs version
   - Verify all required files are present
   - Check for conflicting FatFs installations

2. **Runtime Errors**  
   - Verify SPI connections and CS pin configuration
   - Check SD card compatibility (SDHC vs SDXC)
   - Monitor SPI timing and power supply

3. **Performance Issues**
   - Adjust SPI clock frequency in `sd_spi_diskio.cpp` (current: 400kHz init, 4MHz SDHC operation)
   - Consider cluster size optimization in `f_mkfs()`
   - Profile disk I/O operations

### Debug Information

Enable debug output by adding to `sd_spi_diskio.cpp`:
```cpp
#define DEBUG_SD_SPI 1
// Add Serial.print() statements in critical functions
```

## References

- **FatFs Official Site**: http://elm-chan.org/fsw/ff/
- **FatFs Documentation**: http://elm-chan.org/fsw/ff/doc/00index_e.html  
- **SD Card Specification**: https://www.sdcard.org/downloads/pls/
- **Arduino FS Library**: https://github.com/arduino/ArduinoCore-API/blob/master/api/FS.h
- **LittleFS Reference**: https://github.com/ARMmbed/littlefs