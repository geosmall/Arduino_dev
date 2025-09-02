#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include "fatfs/ff.h"

// Configuration - can be overridden at compile time
#ifndef SDFS_MAX_OPEN_FILES
#define SDFS_MAX_OPEN_FILES 2
#endif

#ifndef SDFS_MAX_OPEN_DIRS
#define SDFS_MAX_OPEN_DIRS 1
#endif

// Forward declarations
class SDFS;
class SDFSFile;

// Clock class for timestamps - matches LittleFS interface
class SDFS_clock_class
{
public:
    static unsigned long get(void) __attribute__((always_inline)) { return HAL_GetTick() / 1000; }
};
extern SDFS_clock_class SDFSClock;

// File implementation class
class SDFSFile : public FileImpl
{
private:
    // Private constructors - only SDFS class can create instances
    SDFSFile(FATFS *fsin, FIL *filin, const char *name);
    SDFSFile(FATFS *fsin, DIR *dirin, const char *name);
    friend class SDFS;
    
public:
    virtual ~SDFSFile();

    // FileImpl interface implementation
    virtual size_t read(void *buf, size_t nbyte) override;
    virtual size_t write(const void *buf, size_t size) override;
    virtual int available() override;
    virtual int peek() override;
    virtual void flush() override;
    virtual bool truncate(uint64_t size = 0) override;
    virtual bool seek(uint64_t pos, int mode) override;
    virtual uint64_t position() override;
    virtual uint64_t size() override;
    virtual void close() override;
    virtual bool isOpen() override;
    virtual const char* name() override;
    virtual bool isDirectory() override;
    virtual File openNextFile(uint8_t mode = 0) override;
    virtual void rewindDirectory() override;
    
    // DateTime support - matches LittleFS interface
    virtual bool getCreateTime(DateTimeFields &tm) override;
    virtual bool getModifyTime(DateTimeFields &tm) override;
    virtual bool setCreateTime(const DateTimeFields &tm) override;
    virtual bool setModifyTime(const DateTimeFields &tm) override;

private:
    FATFS *fs;
    FIL *file;
    DIR *dir;
    char fullpath[128];
    
    uint32_t fatTimeToUnix(WORD fdate, WORD ftime);
    void unixToFatTime(uint32_t unixTime, WORD *fdate, WORD *ftime);
};

// Main filesystem class - matches LittleFS interface
class SDFS : public FS
{
public:
    constexpr SDFS() {}
    virtual ~SDFS();

    // FS interface implementation
    virtual File open(const char *filepath, uint8_t mode = FILE_READ) override;
    virtual bool exists(const char *filepath) override;
    virtual bool mkdir(const char *filepath) override;
    virtual bool rename(const char *oldfilepath, const char *newfilepath) override;
    virtual bool remove(const char *filepath) override;
    virtual bool rmdir(const char *filepath) override;
    virtual uint64_t usedSize() override;
    virtual uint64_t totalSize() override;
    virtual bool format() override;
    virtual bool mediaPresent() override;
    virtual const char* name() override;

protected:
    bool configured = false;
    bool mounted = false;
    FATFS fatfs = {};
    char drive_path[4] = "0:/";  // FatFs drive path
    
    // Helper methods
    FRESULT mountFilesystem();
    void unmountFilesystem();
    const char* fresultToString(FRESULT fr);
};

// SPI-specific SD card implementation
class SDFS_SPI : public SDFS
{
public:
    constexpr SDFS_SPI() {}
    
    // Initialize with SPI settings - matches LittleFS_SPIFlash interface
    bool begin(uint8_t cspin, SPIClass &spiport = SPI);
    const char* getMediaName();
    
    // SPI speed configuration (call before begin())
    void setSPISpeed(uint32_t speed_hz);
    uint32_t getSPISpeed();

private:
    SPIClass *port = nullptr;
    uint8_t pin = 0;
    bool initializeSDCard();
};