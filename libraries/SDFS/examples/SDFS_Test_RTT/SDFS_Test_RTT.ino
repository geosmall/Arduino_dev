/*
 * SDFS RTT Debug Example
 * 
 * Enhanced version of SDFS_Test with RTT debugging for investigating
 * directory enumeration issues. Uses SEGGER_RTT for real-time debug output
 * to diagnose f_readdir behavior and FatFs state.
 * 
 * Hardware connections:
 * - MOSI: PC12 (or PA7 for BlackPill)  
 * - MISO: PC11 (or PA6 for BlackPill)
 * - SCLK: PC10 (or PA5 for BlackPill)  
 * - CS:   PD2  (or PA4 for BlackPill)
 * 
 * Debug via: JLinkRTTClient
 */

#include <SDFS.h>
#include <SEGGER_RTT.h>

// Pin definitions based on board type
#if defined(ARDUINO_BLACKPILL_F411CE)
#define CS_PIN PA4
#define SPI_MOSI PA7
#define SPI_MISO PA6
#define SPI_SCLK PA5
#else
#define CS_PIN PD2
#define SPI_MOSI PC12
#define SPI_MISO PC11
#define SPI_SCLK PC10
#endif

// Create SDFS instance
SDFS_SPI sdfs;

void setup() {
  // Initialize RTT
  SEGGER_RTT_Init();
  SEGGER_RTT_printf(0, "SDFS RTT Debug Example\n");
  SEGGER_RTT_printf(0, "======================\n");
  
  // Initialize Serial for compatibility
  Serial.begin(115200);
  
  // Configure SPI pins
  SPI.setMOSI(SPI_MOSI);
  SPI.setMISO(SPI_MISO);  
  SPI.setSCLK(SPI_SCLK);
  
  SEGGER_RTT_printf(0, "SPI pins configured\n");
  
  // Initialize SDFS
  SEGGER_RTT_printf(0, "Initializing SD card...");
  if (sdfs.begin(CS_PIN)) {
    SEGGER_RTT_printf(0, " SUCCESS\n");
    
    // Display card information
    SEGGER_RTT_printf(0, "Media: %s\n", sdfs.getMediaName());
    SEGGER_RTT_printf(0, "Total Size: %lu MB\n", sdfs.totalSize() / (1024 * 1024));
    SEGGER_RTT_printf(0, "Used Size: %lu MB\n", sdfs.usedSize() / (1024 * 1024));
    
    // Enhanced file operations testing
    testFileOperationsDebug();
    
  } else {
    SEGGER_RTT_printf(0, " FAILED\n");
    SEGGER_RTT_printf(0, "Check connections and card insertion\n");
  }
  
  SEGGER_RTT_printf(0, "\n*STOP*\n");
}

void loop() {
  // Test complete - signal ready
  delay(1000);
}

void testFileOperationsDebug() {
  SEGGER_RTT_printf(0, "\nTesting file operations with debug:\n");
  
  // Test 1: Write a file
  SEGGER_RTT_printf(0, "Writing test file...");
  File testFile = sdfs.open("/debug_test.txt", FILE_WRITE_BEGIN);
  if (testFile) {
    testFile.println("Hello from SDFS RTT Debug!");
    testFile.print("millis: ");
    testFile.println(millis());
    testFile.close();
    SEGGER_RTT_printf(0, " OK\n");
  } else {
    SEGGER_RTT_printf(0, " FAILED\n");
    return;
  }
  
  // Test 2: Read the file back
  SEGGER_RTT_printf(0, "Reading test file...");
  testFile = sdfs.open("/debug_test.txt", FILE_READ);
  if (testFile) {
    SEGGER_RTT_printf(0, " OK\n");
    SEGGER_RTT_printf(0, "File contents:\n");
    while (testFile.available()) {
      char c = testFile.read();
      SEGGER_RTT_printf(0, "%c", c);
    }
    testFile.close();
  } else {
    SEGGER_RTT_printf(0, " FAILED\n");
  }
  
  // Test 3: Debug directory listing
  SEGGER_RTT_printf(0, "\nDEBUG: Root directory listing:\n");
  File root = sdfs.open("/");
  if (root) {
    SEGGER_RTT_printf(0, "DEBUG: Root directory opened successfully\n");
    SEGGER_RTT_printf(0, "DEBUG: Starting file enumeration...\n");
    
    int fileCount = 0;
    while (true) {
      SEGGER_RTT_printf(0, "DEBUG: Calling openNextFile() #%d\n", fileCount + 1);
      File entry = root.openNextFile();
      
      if (!entry) {
        SEGGER_RTT_printf(0, "DEBUG: openNextFile() returned null - end of directory\n");
        break;
      }
      
      fileCount++;
      SEGGER_RTT_printf(0, "DEBUG: Found file #%d\n", fileCount);
      SEGGER_RTT_printf(0, "%s %s", entry.isDirectory() ? "DIR " : "FILE", entry.name());
      
      if (!entry.isDirectory()) {
        SEGGER_RTT_printf(0, " (%lu bytes)", entry.size());
      }
      SEGGER_RTT_printf(0, "\n");
      
      entry.close();
    }
    
    SEGGER_RTT_printf(0, "DEBUG: Total files found: %d\n", fileCount);
    root.close();
  } else {
    SEGGER_RTT_printf(0, "DEBUG: FAILED to open root directory!\n");
  }
  
  // Test 4: Try to open known existing files from SD library test
  SEGGER_RTT_printf(0, "\nDEBUG: Testing access to known files:\n");
  testKnownFile("/LAGER.CFG");
  testKnownFile("/LOG000.TXT");  
  testKnownFile("/TEST.TXT");
  
  SEGGER_RTT_printf(0, "\nAll debug tests completed!\n");
}

void testKnownFile(const char* filename) {
  SEGGER_RTT_printf(0, "DEBUG: Testing file %s...", filename);
  File file = sdfs.open(filename, FILE_READ);
  if (file) {
    SEGGER_RTT_printf(0, " EXISTS (%lu bytes)\n", file.size());
    file.close();
  } else {
    SEGGER_RTT_printf(0, " NOT FOUND\n");
  }
}