#include "sd_spi_diskio.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"

// SPI SD card interface globals
static uint8_t spi_cs_pin = 0;
static SPIClass *spi_port = nullptr;
static bool spi_initialized = false;
static bool is_sdhc_card = false;  // Track if card uses block addressing (SDHC) or byte addressing (SD)
static bool spi_session_active = false;  // Track persistent SPI session (like SdFat)

// SPI speed configuration
#define SPI_SPEED_INIT_HZ       400000    // 400kHz for initialization (safe/standard)
#define SPI_SPEED_OPERATION_HZ  4000000   // 4MHz for normal operation (matches SD library SPI_HALF_SPEED)
#define SPI_SPEED_FAST_HZ       8000000   // 8MHz for production (short traces only)

// Current speed mode selection - can be changed for different hardware setups
static uint32_t current_spi_speed = SPI_SPEED_OPERATION_HZ;

// SD card commands
#define CMD0    (0x40 + 0)     // GO_IDLE_STATE
#define CMD1    (0x40 + 1)     // SEND_OP_COND (MMC)
#define CMD8    (0x40 + 8)     // SEND_IF_COND
#define CMD9    (0x40 + 9)     // SEND_CSD
#define CMD10   (0x40 + 10)    // SEND_CID
#define CMD12   (0x40 + 12)    // STOP_TRANSMISSION
#define CMD16   (0x40 + 16)    // SET_BLOCKLEN
#define CMD17   (0x40 + 17)    // READ_SINGLE_BLOCK
#define CMD18   (0x40 + 18)    // read_MULTIPLE_BLOCK
#define CMD23   (0x40 + 23)    // SET_BLOCK_COUNT (MMC)
#define CMD24   (0x40 + 24)    // WRITE_BLOCK
#define CMD25   (0x40 + 25)    // WRITE_MULTIPLE_BLOCK
#define CMD41   (0x40 + 41)    // SEND_OP_COND (SDC)
#define CMD55   (0x40 + 55)    // APP_CMD
#define CMD58   (0x40 + 58)    // READ_OCR

// SD card responses
#define R1_READY_STATE          0x00
#define R1_IDLE_STATE           0x01
#define R1_ILLEGAL_COMMAND      0x04

// SD card data tokens
#define TOKEN_SINGLE_MULTI_BLOCK_READ   0xFE
#define TOKEN_SINGLE_BLOCK_WRITE        0xFE
#define TOKEN_MULTI_BLOCK_WRITE         0xFC
#define TOKEN_STOP_MULTI_BLOCK_WRITE    0xFD

// Forward declarations
static void spi_cs_low(void);
static void spi_cs_high(void);
static uint8_t spi_transfer(uint8_t data);
static void spi_transfer_block(uint8_t *buffer, uint16_t length);
static uint8_t sd_send_command(uint8_t cmd, uint32_t arg);
static bool sd_wait_ready(uint32_t timeout_ms = 500);
static bool sd_read_block(uint8_t *buffer, uint32_t block_num);
static bool sd_write_block(const uint8_t *buffer, uint32_t block_num);

// SPI session management functions (like SdFat)
static void spi_session_start(void)
{
    if (!spi_session_active) {
        spi_port->beginTransaction(SPISettings(current_spi_speed, MSBFIRST, SPI_MODE0));
        digitalWrite(spi_cs_pin, LOW);
        spi_port->transfer(0xFF);  // Dummy byte to drive MISO busy status
        spi_session_active = true;
    }
}

static void spi_session_stop(void)
{
    if (spi_session_active) {
        digitalWrite(spi_cs_pin, HIGH);
        spi_port->transfer(0xFF);  // Ensure MISO goes to low Z
        spi_port->endTransaction();
        spi_session_active = false;
    }
}

// Backward compatibility functions
static void spi_cs_low(void)
{
    spi_session_start();
}

static void spi_cs_high(void)
{
    spi_session_stop();
}

static uint8_t spi_transfer(uint8_t data)
{
    return spi_port->transfer(data);
}

static void spi_transfer_block(uint8_t *buffer, uint16_t length)
{
    spi_port->transfer(buffer, length);
}

// SD card low-level functions
static uint8_t sd_send_command(uint8_t cmd, uint32_t arg)
{
    // Ensure SPI session is active (like SdFat does)
    spi_session_start();
    
    // Add readiness check for most commands (like SdFat does)
    if (cmd != CMD0 && cmd != CMD12) {
        if (!sd_wait_ready(300)) {  // 300ms timeout like SdFat
            Serial.print("DEBUG: Card not ready before CMD");
            Serial.println(cmd, DEC);
            return 0xFF;
        }
    }
    
    uint8_t crc = 0xFF;
    
    // Special CRC for CMD0 and CMD8
    if (cmd == CMD0) crc = 0x95;
    if (cmd == CMD8) crc = 0x87;
    
    // Send command packet
    spi_transfer(cmd);
    spi_transfer((arg >> 24) & 0xFF);
    spi_transfer((arg >> 16) & 0xFF);
    spi_transfer((arg >> 8) & 0xFF);
    spi_transfer(arg & 0xFF);
    spi_transfer(crc);
    
    // Discard first fill byte to avoid MISO pull-up problem (like SdFat)
    spi_transfer(0xFF);
    
    // Wait for response with proper timeout (like SdFat's response loop)
    uint8_t response = 0xFF;
    for (int i = 0; i < 10; i++) {
        response = spi_transfer(0xFF);
        if ((response & 0x80) == 0) break;  // SdFat-style response check
    }
    
    return response;
}

static bool sd_wait_ready(uint32_t timeout_ms)
{
    uint32_t start = millis();
    
    while ((millis() - start) < timeout_ms) {
        if (spi_transfer(0xFF) == 0xFF) return true;
        delay(1);
    }
    
    return false;
}

static bool sd_read_block(uint8_t *buffer, uint32_t block_num)
{
    if (!spi_initialized || !buffer) {
        Serial.println("DEBUG: sd_read_block() - not initialized or null buffer");
        return false;
    }
    
    Serial.print("DEBUG: sd_read_block() - reading block ");
    Serial.println(block_num);
    
    // Convert block number to address based on card type (like SdFat does)
    uint32_t address;
    if (is_sdhc_card) {
        // SDHC cards use block addressing
        address = block_num;
        Serial.println("DEBUG: Using SDHC block addressing");
    } else {
        // SD cards use byte addressing (block * 512)
        address = block_num << 9;  // Multiply by 512
        Serial.print("DEBUG: Using SD byte addressing, address = ");
        Serial.println(address);
    }
    
    // Send READ_SINGLE_BLOCK command (session managed by sd_send_command)
    uint8_t response = sd_send_command(CMD17, address);
    if (response != 0x00) {
        Serial.print("DEBUG: CMD17 failed with response: 0x");
        Serial.println(response, HEX);
        spi_session_stop();  // End session on failure
        return false;
    }
    
    // Wait for data token
    uint32_t start = millis();
    bool token_found = false;
    while ((millis() - start) < 500) {
        uint8_t token = spi_transfer(0xFF);
        if (token == TOKEN_SINGLE_MULTI_BLOCK_READ) {
            token_found = true;
            break;
        }
        if (token != 0xFF) {
            Serial.print("DEBUG: Unexpected data token: 0x");
            Serial.println(token, HEX);
            break;
        }
    }
    
    if (!token_found) {
        Serial.println("DEBUG: Data token timeout");
        spi_session_stop();
        return false;
    }
    
    // Read data block
    spi_transfer_block(buffer, 512);
    
    // Read CRC (ignore)
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    
    spi_session_stop();  // End session after successful read
    return true;
}

static bool sd_write_block(const uint8_t *buffer, uint32_t block_num)
{
    if (!spi_initialized || !buffer) return false;
    
    // Convert block number to address based on card type (like SdFat does)
    uint32_t address;
    if (is_sdhc_card) {
        // SDHC cards use block addressing
        address = block_num;
    } else {
        // SD cards use byte addressing (block * 512)
        address = block_num << 9;  // Multiply by 512
    }
    
    spi_cs_low();
    
    // Send WRITE_BLOCK command
    uint8_t response = sd_send_command(CMD24, address);
    if (response != 0x00) {
        spi_cs_high();
        return false;
    }
    
    // Send data token
    spi_transfer(TOKEN_SINGLE_BLOCK_WRITE);
    
    // Send data block
    for (int i = 0; i < 512; i++) {
        spi_transfer(buffer[i]);
    }
    
    // Send dummy CRC
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    
    // Wait for data response
    response = spi_transfer(0xFF) & 0x1F;
    if (response != 0x05) {
        spi_cs_high();
        return false;
    }
    
    // Wait for write completion
    if (!sd_wait_ready(500)) {
        spi_cs_high();
        return false;
    }
    
    spi_cs_high();
    return true;
}

// Public interface functions
bool sd_spi_initialize(uint8_t cs_pin, SPIClass *spi)
{
    if (!spi) return false;
    
    spi_cs_pin = cs_pin;
    spi_port = spi;
    
    // Initialize CS pin
    pinMode(spi_cs_pin, OUTPUT);
    spi_cs_high();
    
    // Initialize SPI
    spi_port->begin();
    
    // For STM32, configure the global SPI pins if using a custom SPI bus
    // This ensures the pins are properly set up for the SPI peripheral
    if (spi_port != &SPI) {
        // If using a custom SPIClass, it should already have pins configured
        // But ensure the begin() call has properly initialized the peripheral
    } else {
        // Using the default SPI, need to configure pins explicitly
        #if defined(ARDUINO_BLACKPILL_F411CE)
        SPI.setMOSI(PA7);
        SPI.setMISO(PA6);  
        SPI.setSCLK(PA5);
        #else
        SPI.setMOSI(PC12);
        SPI.setMISO(PC11);
        SPI.setSCLK(PC10);
        #endif
        SPI.begin(); // Re-initialize after pin config
    }
    
    spi_port->beginTransaction(SPISettings(SPI_SPEED_INIT_HZ, MSBFIRST, SPI_MODE0)); // Start with slow clock for initialization
    spi_port->endTransaction();
    
    delay(10);
    
    // Send 80 dummy clocks with CS high
    for (int i = 0; i < 10; i++) {
        spi_transfer(0xFF);
    }
    
    spi_cs_low();
    
    // Send CMD0 to reset card
    uint8_t response = sd_send_command(CMD0, 0);
    if (response != R1_IDLE_STATE) {
        spi_cs_high();
        return false;
    }
    
    // Send CMD8 to check card version
    response = sd_send_command(CMD8, 0x1AA);
    bool sdhc = false;
    
    if (response == R1_IDLE_STATE) {
        // SDC V2 card
        uint32_t ocr = 0;
        for (int i = 0; i < 4; i++) {
            ocr = (ocr << 8) | spi_transfer(0xFF);
        }
        
        if ((ocr & 0x1AA) == 0x1AA) {
            sdhc = true;
            is_sdhc_card = true;  // Save globally for use in read/write functions
        } else {
            spi_cs_high();
            return false;
        }
    }
    
    // Initialize card with ACMD41
    uint32_t start = millis();
    do {
        sd_send_command(CMD55, 0); // APP_CMD
        response = sd_send_command(CMD41, sdhc ? 0x40000000 : 0);
        
        if ((millis() - start) > 1000) {
            spi_cs_high();
            return false;
        }
        delay(1);
    } while (response != R1_READY_STATE);
    
    // Set block length to 512 bytes
    response = sd_send_command(CMD16, 512);
    if (response != R1_READY_STATE) {
        spi_cs_high();
        return false;
    }
    
    spi_cs_high();
    
    // SPI speed will be set per transaction for normal operation
    
    spi_initialized = true;
    return true;
}

uint8_t sd_spi_get_cs_pin(void)
{
    return spi_cs_pin;
}

SPIClass* sd_spi_get_port(void)
{
    return spi_port;
}

// FatFs disk I/O functions
extern "C" {

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    return spi_initialized ? 0 : STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    return spi_initialized ? 0 : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv != 0 || !spi_initialized) {
        Serial.println("DEBUG: disk_read() - invalid parameters or not initialized");
        return RES_PARERR;
    }
    
    Serial.print("DEBUG: disk_read() - reading ");
    Serial.print(count);
    Serial.print(" sectors starting from ");
    Serial.println(sector);
    
    for (UINT i = 0; i < count; i++) {
        if (!sd_read_block(buff + (i * 512), sector + i)) {
            Serial.print("DEBUG: sd_read_block() failed for sector ");
            Serial.println(sector + i);
            return RES_ERROR;
        }
    }
    
    Serial.println("DEBUG: disk_read() successful");
    
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv != 0 || !spi_initialized) return RES_PARERR;
    
    for (UINT i = 0; i < count; i++) {
        if (!sd_write_block(buff + (i * 512), sector + i)) {
            return RES_ERROR;
        }
    }
    
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    if (pdrv != 0 || !spi_initialized) return RES_PARERR;
    
    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
            
        case GET_SECTOR_COUNT:
            *(DWORD*)buff = 0x10000; // Default to 32MB for now
            return RES_OK;
            
        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            return RES_OK;
            
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            return RES_OK;
            
        default:
            return RES_PARERR;
    }
}

DWORD get_fattime(void)
{
    // Return current time in FAT format
    // For now, return a fixed time (2024-01-01 12:00:00)
    return ((2024 - 1980) << 25) | (1 << 21) | (1 << 16) | (12 << 11) | (0 << 5) | (0 >> 1);
}

} // extern "C"

// SPI speed configuration functions
void sd_spi_set_speed(uint32_t speed_hz)
{
    current_spi_speed = speed_hz;
}

uint32_t sd_spi_get_speed(void)
{
    return current_spi_speed;
}