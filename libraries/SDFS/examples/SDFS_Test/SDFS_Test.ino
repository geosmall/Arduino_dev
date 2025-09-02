#include <SDFS.h>

void Local_Error_Handler()
{
    asm("BKPT #0\n"); // break into the debugger
}

// SPI bus configuration - explicit pin definitions for different boards
#if defined(ARDUINO_BLACKPILL_F411CE)
//              MOSI  MISO  SCLK
SPIClass SPIbus(PA7,  PA6,  PA5);   // SPI1 peripheral
#define CS_PIN PA4
#else
//              MOSI  MISO  SCLK  
SPIClass SPIbus(PC12, PC11, PC10);  // SPI3 peripheral (default for Nucleo-64)
#define CS_PIN PD2
#endif

// Create SDFS instance
SDFS_SPI sdfs;

void setup() {
  bool res;
  
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  
  // Send ready signal and wait for sync
  Serial.println("READY_FOR_SYNC");
  Serial.flush();
  
  // Wait for sync character ('S') from test script
  while (Serial.available() == 0 || Serial.read() != 'S') {
    delay(10);
  }
  
  Serial.println("SDFS Test Starting...");
  
  // Ensure the CS pin is pulled HIGH
  pinMode(CS_PIN, OUTPUT); 
  digitalWrite(CS_PIN, HIGH);
  
  delay(10); // Wait a bit to make sure SD card is ready
  
  // Configure SPI speed for breadboard-friendly operation (1MHz instead of 8MHz)
  Serial.println("Setting SPI speed to 1MHz for breadboard compatibility...");
  sdfs.setSPISpeed(1000000); // 1MHz - breadboard safe
  
  Serial.print("SPI Speed: ");
  Serial.print(sdfs.getSPISpeed());
  Serial.println(" Hz");
  
  // Initialize SDFS with explicit CS pin and SPI bus
  res = sdfs.begin(CS_PIN, SPIbus);
  if (res) {
    Serial.println("SDFS initialization successful!");
    Serial.print("Media: ");
    Serial.println(sdfs.getMediaName());
    
    // Debug: Check if filesystem is mounted
    Serial.print("Media Present: ");
    Serial.println(sdfs.mediaPresent() ? "YES" : "NO");
    
    Serial.print("Total Size: ");
    uint64_t total = sdfs.totalSize();
    Serial.print(total);
    Serial.println(" bytes");
    Serial.print("Used Size: ");
    uint64_t used = sdfs.usedSize();
    Serial.print(used);
    Serial.println(" bytes");
    Serial.print("Free Size: ");
    Serial.print(total - used);
    Serial.println(" bytes");
    
    // Debug size calculation issues
    if (total == 0) {
      Serial.println("DEBUG: totalSize() returned 0 - filesystem mount issue");
    }
    if (used == 0 && total > 0) {
      Serial.println("DEBUG: usedSize() returned 0 - may be normal for empty card");
    }
    
    // Test basic file operations
    testFileOperations();
  } else {
    Serial.println("SDFS initialization failed!");
    Serial.println("Check SD card connection and format (FAT32 recommended)");
    Local_Error_Handler();
  }
}

void loop() {
  delay(5000);
  Serial.println("SDFS Test running... (heartbeat every 5s)");
}

void testFileOperations() {
  Serial.println("\n=== Testing Basic File Operations ===");
  
  // Test file creation and writing
  File testFile = sdfs.open("/test.txt", FILE_WRITE_BEGIN);
  if (testFile) {
    Serial.println("Created test file successfully");
    testFile.println("Hello from SDFS!");
    testFile.print("Current millis: ");
    testFile.println(millis());
    testFile.close();
    Serial.println("Written data to test file");
  } else {
    Serial.println("Failed to create test file");
    return;
  }
  
  // Test file reading
  testFile = sdfs.open("/test.txt", FILE_READ);
  if (testFile) {
    Serial.println("Reading test file contents:");
    Serial.println("--- File Contents ---");
    while (testFile.available()) {
      Serial.write(testFile.read());
    }
    Serial.println("--- End of File ---");
    testFile.close();
  } else {
    Serial.println("Failed to read test file");
  }
  
  // Test file size
  if (sdfs.exists("/test.txt")) {
    testFile = sdfs.open("/test.txt", FILE_READ);
    Serial.print("Test file size: ");
    Serial.print(testFile.size());
    Serial.println(" bytes");
    testFile.close();
  }
  
  Serial.println("=== Basic File Operations Complete ===\n");
}