/*
 * SDFS Library Example
 * 
 * Demonstrates basic usage of SDFS (SD card File System) with SPI interface.
 * Compatible with STM32 Arduino Core and follows FS.h interface.
 * 
 * Hardware connections:
 * - MOSI: PC12 (or PA7 for BlackPill)  
 * - MISO: PC11 (or PA6 for BlackPill)
 * - SCLK: PC10 (or PA5 for BlackPill)  
 * - CS:   PD2  (or PA4 for BlackPill)
 */

#include <SDFS.h>

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
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("SDFS Library Example");
  Serial.println("====================");
  
  // Configure SPI pins
  SPI.setMOSI(SPI_MOSI);
  SPI.setMISO(SPI_MISO);
  SPI.setSCLK(SPI_SCLK);
  
  // Initialize SDFS
  Serial.print("Initializing SD card...");
  if (sdfs.begin(CS_PIN)) {
    Serial.println(" SUCCESS");
    
    // Display card information
    Serial.print("Media: ");
    Serial.println(sdfs.getMediaName());
    Serial.print("Total Size: ");
    Serial.print(sdfs.totalSize() / (1024 * 1024));
    Serial.println(" MB");
    Serial.print("Used Size: ");  
    Serial.print(sdfs.usedSize() / (1024 * 1024));
    Serial.println(" MB");
    
    // Test file operations
    testFileOperations();
    
  } else {
    Serial.println(" FAILED");
    Serial.println("Check connections and card insertion");
  }
}

void loop() {
  // Example complete - signal completion for HIL testing
  Serial.println("*STOP*");
  delay(5000);
}

void testFileOperations() {
  Serial.println("\nTesting file operations:");
  
  // Test 1: Write a file
  Serial.print("Writing test file...");
  File testFile = sdfs.open("/test.txt", FILE_WRITE_BEGIN);
  if (testFile) {
    testFile.println("Hello from SDFS!");
    testFile.println("Line 2");
    testFile.close();
    Serial.println(" OK");
  } else {
    Serial.println(" FAILED");
    return;
  }
  
  // Test 2: Read the file back
  Serial.print("Reading test file...");
  testFile = sdfs.open("/test.txt", FILE_READ);
  if (testFile) {
    Serial.println(" OK");
    Serial.println("File contents:");
    while (testFile.available()) {
      Serial.write(testFile.read());
    }
    testFile.close();
  } else {
    Serial.println(" FAILED");
  }
  
  // Test 3: List root directory
  Serial.println("\nRoot directory listing:");
  File root = sdfs.open("/");
  if (root) {
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;
      
      Serial.print(entry.isDirectory() ? "DIR  " : "FILE ");
      Serial.print(entry.name());
      if (!entry.isDirectory()) {
        Serial.print(" (");
        Serial.print(entry.size());
        Serial.print(" bytes)");
      }
      Serial.println();
      entry.close();
    }
    root.close();
  }
  
  Serial.println("\nAll tests completed!");
}