/*
 * SDFS RTT Test
 * 
 * Test SDFS filesystem using SEGGER RTT for real-time debugging.
 * No serial port pollution - all output via J-Link RTT.
 * 
 * Hardware:
 * - STM32F411RE Nucleo Board
 * - SPI SD Card Module
 *   - CS:   D10 (PA11) 
 *   - MOSI: D11 (PA7)
 *   - MISO: D12 (PA6) 
 *   - SCK:  D13 (PA5)
 * 
 * Usage:
 *   1. Compile and upload sketch
 *   2. Start RTT client: JLinkRTTClient -Device STM32F411RE -If SWD -Speed 4000
 *   3. View real-time test results without serial interference
 */

#include <SPI.h>
#include <SDFS.h>
#include <SEGGER_RTT.h>

// SPI settings optimized for breadboard connections
const int CS_PIN = 10;
const uint32_t SPI_SPEED = 1000000;  // 1MHz - breadboard friendly

// Create SDFS instance
SDFS_SPI SDFS;

void setup() {
  // Initialize RTT for debugging
  SEGGER_RTT_Init();
  SEGGER_RTT_printf(0, "\n=== SDFS RTT Test Suite ===\n");
  SEGGER_RTT_printf(0, "Hardware: STM32F411RE + SPI SD Card\n");
  SEGGER_RTT_printf(0, "SPI Speed: %lu Hz\n", SPI_SPEED);
  SEGGER_RTT_printf(0, "CS Pin: D%d\n\n", CS_PIN);
  
  // Initialize SPI with custom speed
  SPI.begin();
  
  // Run comprehensive test suite
  test_sdfs_initialization();
  test_basic_file_operations();
  test_filesystem_info();
  test_directory_operations();
  test_error_handling();
  
  SEGGER_RTT_printf(0, "\n=== Test Suite Complete ===\n");
  SEGGER_RTT_printf(0, "Monitor via: JLinkRTTClient -Device STM32F411RE -If SWD -Speed 4000\n");
}

void loop() {
  // Test complete - idle loop
  delay(5000);
  SEGGER_RTT_printf(0, ".");  // Heartbeat every 5 seconds
}

void test_sdfs_initialization() {
  SEGGER_RTT_printf(0, "--- Testing SDFS Initialization ---\n");
  
  // Setup CS pin
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  delay(10);
  
  // Configure SPI speed
  SDFS.setSPISpeed(SPI_SPEED);
  SEGGER_RTT_printf(0, "SPI Speed: %lu Hz\n", SDFS.getSPISpeed());
  
  // Test SDFS initialization 
  bool result = SDFS.begin(CS_PIN);
  
  if (result) {
    SEGGER_RTT_printf(0, "PASS: SDFS initialization successful\n");
    SEGGER_RTT_printf(0, "      Media: %s\n", SDFS.getMediaName());
    SEGGER_RTT_printf(0, "      Media Present: %s\n", SDFS.mediaPresent() ? "YES" : "NO");
  } else {
    SEGGER_RTT_printf(0, "FAIL: SDFS initialization failed\n");
    SEGGER_RTT_printf(0, "      Check: SD card inserted, wiring, power, FAT32 format\n");
    return; // Skip remaining tests if initialization fails
  }
  
  SEGGER_RTT_printf(0, "\n");
}

void test_basic_file_operations() {
  SEGGER_RTT_printf(0, "--- Testing Basic File Operations ---\n");
  
  const char* test_file = "/rtt_test.txt";
  const char* test_data = "RTT Test Data - No Serial Pollution!";
  
  // Test file creation and writing
  File file = SDFS.open(test_file, FILE_WRITE);
  if (file) {
    size_t bytes_written = file.print(test_data);
    file.close();
    SEGGER_RTT_printf(0, "PASS: File write - %d bytes written\n", bytes_written);
  } else {
    SEGGER_RTT_printf(0, "FAIL: Could not create test file\n");
    return;
  }
  
  // Test file reading
  file = SDFS.open(test_file, FILE_READ);
  if (file) {
    String content = file.readString();
    file.close();
    
    if (content.equals(test_data)) {
      SEGGER_RTT_printf(0, "PASS: File read - content matches\n");
    } else {
      SEGGER_RTT_printf(0, "FAIL: File read - content mismatch\n");
      SEGGER_RTT_printf(0, "      Expected: %s\n", test_data);
      SEGGER_RTT_printf(0, "      Got:      %s\n", content.c_str());
    }
  } else {
    SEGGER_RTT_printf(0, "FAIL: Could not read test file\n");
  }
  
  // Test file existence
  bool exists = SDFS.exists(test_file);
  SEGGER_RTT_printf(0, "%s: File exists check\n", exists ? "PASS" : "FAIL");
  
  // Test file removal
  bool removed = SDFS.remove(test_file);
  SEGGER_RTT_printf(0, "%s: File removal\n", removed ? "PASS" : "FAIL");
  
  // Verify removal
  exists = SDFS.exists(test_file);
  SEGGER_RTT_printf(0, "%s: File removal verification\n", !exists ? "PASS" : "FAIL");
  
  SEGGER_RTT_printf(0, "\n");
}

void test_filesystem_info() {
  SEGGER_RTT_printf(0, "--- Testing Filesystem Information ---\n");
  
  uint64_t total = SDFS.totalSize();
  uint64_t used = SDFS.usedSize();
  uint64_t free = total - used;
  
  SEGGER_RTT_printf(0, "Total Space: %llu bytes (%.2f MB)\n", total, total / 1024.0 / 1024.0);
  SEGGER_RTT_printf(0, "Used Space:  %llu bytes (%.2f MB)\n", used, used / 1024.0 / 1024.0);
  SEGGER_RTT_printf(0, "Free Space:  %llu bytes (%.2f MB)\n", free, free / 1024.0 / 1024.0);
  
  // Basic sanity checks
  bool total_reasonable = (total > 0 && total < (64ULL * 1024 * 1024 * 1024)); // 0-64GB range
  bool used_reasonable = (used <= total);
  
  SEGGER_RTT_printf(0, "%s: Total size reasonable\n", total_reasonable ? "PASS" : "FAIL");
  SEGGER_RTT_printf(0, "%s: Used <= Total\n", used_reasonable ? "PASS" : "FAIL");
  
  SEGGER_RTT_printf(0, "\n");
}

void test_directory_operations() {
  SEGGER_RTT_printf(0, "--- Testing Directory Operations ---\n");
  
  const char* test_dir = "/rtt_testdir";
  
  // Test directory creation
  bool dir_created = SDFS.mkdir(test_dir);
  SEGGER_RTT_printf(0, "%s: Directory creation\n", dir_created ? "PASS" : "FAIL");
  
  // Test directory existence
  bool dir_exists = SDFS.exists(test_dir);
  SEGGER_RTT_printf(0, "%s: Directory exists check\n", dir_exists ? "PASS" : "FAIL");
  
  // Create test file in directory
  String test_file_in_dir = String(test_dir) + "/nested_file.txt";
  File file = SDFS.open(test_file_in_dir.c_str(), FILE_WRITE);
  if (file) {
    file.print("Nested file content");
    file.close();
    SEGGER_RTT_printf(0, "PASS: Created file in directory\n");
  } else {
    SEGGER_RTT_printf(0, "FAIL: Could not create file in directory\n");
  }
  
  // Test directory listing
  File dir = SDFS.open("/");
  if (dir) {
    SEGGER_RTT_printf(0, "Root directory contents:\n");
    while (true) {
      File entry = dir.openNextFile();
      if (!entry) {
        break;
      }
      SEGGER_RTT_printf(0, "  %s %s (%d bytes)\n", 
                       entry.isDirectory() ? "[DIR]" : "[FILE]",
                       entry.name(), 
                       entry.size());
      entry.close();
    }
    dir.close();
    SEGGER_RTT_printf(0, "PASS: Directory listing completed\n");
  } else {
    SEGGER_RTT_printf(0, "FAIL: Could not open root directory\n");
  }
  
  // Cleanup
  SDFS.remove(test_file_in_dir.c_str());
  bool dir_removed = SDFS.rmdir(test_dir);
  SEGGER_RTT_printf(0, "%s: Directory removal\n", dir_removed ? "PASS" : "FAIL");
  
  SEGGER_RTT_printf(0, "\n");
}

void test_error_handling() {
  SEGGER_RTT_printf(0, "--- Testing Error Handling ---\n");
  
  // Test opening non-existent file
  File file = SDFS.open("/nonexistent.txt", FILE_READ);
  if (!file) {
    SEGGER_RTT_printf(0, "PASS: Correctly failed to open non-existent file\n");
  } else {
    SEGGER_RTT_printf(0, "FAIL: Should not have opened non-existent file\n");
    file.close();
  }
  
  // Test removing non-existent file
  bool removed = SDFS.remove("/nonexistent.txt");
  SEGGER_RTT_printf(0, "%s: Correctly handled non-existent file removal\n", !removed ? "PASS" : "FAIL");
  
  // Test creating directory with invalid name
  bool invalid_dir = SDFS.mkdir("");
  SEGGER_RTT_printf(0, "%s: Correctly rejected invalid directory name\n", !invalid_dir ? "PASS" : "FAIL");
  
  SEGGER_RTT_printf(0, "\n");
}