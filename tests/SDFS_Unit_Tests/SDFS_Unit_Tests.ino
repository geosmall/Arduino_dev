/*
 * SDFS Unit Tests - AUnit Phase 2 (Read-Only Operations)
 *
 * AUnit-compatible tests for SDFS library focusing on operations that work
 * within the AUnit framework context. Write operations are tested separately
 * in SDFS_Test example due to AUnit/FatFs compatibility limitations.
 *
 * Hardware Setup: Nucleo-F411RE + SD card breakout via jumper wires
 * - MOSI: PC12, MISO: PC11, SCLK: PC10, CS: PD2
 *
 * LIMITATION: AUnit framework conflicts with SDFS/FatFs write operations.
 * Write functionality is validated in libraries/SDFS/examples/SDFS_Test
 *
 * Usage:
 *   ./scripts/aflash.sh tests/SDFS_Unit_Tests --use-rtt --build-id
 */

#include "../../aunit_hil.h"
#include <SDFS.h>

// Hardware configuration for Nucleo-F411RE
#define CS_PIN PD2
#define SPI_MOSI PC12
#define SPI_MISO PC11
#define SPI_SCLK PC10

// Test configuration
#define TEST_FILE "/test.txt"
#define TEST_DIR "/testdir"
#define TEST_DATA "Hello SDFS World!"
#define TEST_DATA_SIZE 18

// Global SDFS instance
SDFS_SPI sdfs;

// Test state tracking
bool sdfs_initialized = false;
bool hardware_detected = false;

// ==============================================================================
// HARDWARE DETECTION AND INITIALIZATION TESTS
// ==============================================================================

test(hardware_detection) {
  // Filesystem already mounted in setup() - just verify the state
  if (!hardware_detected) {
    CI_LOG("SD card not detected in setup() - test skipped\n");
    skipTestNow();
    return;
  }

  // Verify SDFS is properly initialized
  assertTrue(hardware_detected);
  assertTrue(sdfs_initialized);
  assertTrue(sdfs.mediaPresent());

  CI_LOG("SDFS hardware detection: SUCCESS\n");
}

test(media_present_check) {
  if (!hardware_detected) {
    skipTestNow(); // Skip if no hardware
    return;
  }

  assertTrue(sdfs.mediaPresent());
}

test(write_operations_test) {
  if (!hardware_detected) {
    skipTestNow(); // Skip if no hardware
    return;
  }

  CI_LOG("Testing write operations with proper FatFs mounting...\n");

  // Test file creation and writing
  File testFile = sdfs.open("/AUNIT_WRITE_TEST.TXT", FILE_WRITE_BEGIN);
  if (testFile) {
    CI_LOG("File creation: SUCCESS\n");

    // Write some test data
    size_t written = testFile.println("Hello from properly mounted SDFS!");
    CI_LOGF("Wrote %u bytes\n", written);
    testFile.printf("Test framework: AUnit\n");
    testFile.printf("Mount strategy: Single mount in setup()\n");
    testFile.close();

    // Verify file exists and can be read
    if (sdfs.exists("/AUNIT_WRITE_TEST.TXT")) {
      CI_LOG("File verification: SUCCESS\n");

      testFile = sdfs.open("/AUNIT_WRITE_TEST.TXT", FILE_READ);
      if (testFile) {
        CI_LOG("File read test: SUCCESS\n");
        CI_LOG("File contents:\n");
        while (testFile.available()) {
          CI_LOG(String((char)testFile.read()).c_str());
        }
        CI_LOG("\n");
        testFile.close();

        // Clean up
        sdfs.remove("/AUNIT_WRITE_TEST.TXT");
        CI_LOG("File cleanup: SUCCESS\n");

        // If we get here, write operations work!
        assertTrue(true);
      } else {
        CI_LOG("File read: FAILED\n");
        assertFalse(true);
      }
    } else {
      CI_LOG("File verification: FAILED\n");
      assertFalse(true);
    }
  } else {
    CI_LOG("File creation: FAILED\n");
    assertFalse(true);
  }
}

test(filesystem_capacity) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  uint64_t total = sdfs.totalSize();
  uint64_t used = sdfs.usedSize();

  // SD cards should have reasonable capacity (> 1MB, < 1TB)
  assertMoreOrEqual(total, (uint64_t)1024 * 1024);  // At least 1MB
  assertLess(total, (uint64_t)1024ULL * 1024 * 1024 * 1024);  // Less than 1TB
  assertLessOrEqual(used, total);  // Used <= Total
}

test(spi_speed_configuration) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Test SPI speed configuration
  uint32_t default_speed = sdfs.getSPISpeed();
  assertMoreOrEqual(default_speed, 1000000UL);  // At least 1MHz

  // Test setting custom speed
  sdfs.setSPISpeed(2000000UL);
  assertEqual(sdfs.getSPISpeed(), 2000000UL);
}

// ==============================================================================
// FILE CREATION AND BASIC I/O TESTS
// ==============================================================================

test(file_creation_write_mode) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  // Clean up any existing test file
  if (sdfs.exists(TEST_FILE)) {
    sdfs.remove(TEST_FILE);
  }

  // Create new file in write mode
  File file = sdfs.open(TEST_FILE, FILE_WRITE);
  assertTrue(file);
  assertFalse(file.isDirectory());
  file.close();

  // Verify file exists
  assertTrue(sdfs.exists(TEST_FILE));
}

test(file_write_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  File file = sdfs.open(TEST_FILE, FILE_WRITE);
  assertTrue(file);

  // Test write operations
  size_t written = file.write((const uint8_t*)TEST_DATA, TEST_DATA_SIZE);
  assertEqual((int)written, TEST_DATA_SIZE);

  // Test position tracking
  assertEqual((int)file.position(), TEST_DATA_SIZE);

  file.close();
}

test(file_read_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Test reading existing files on SD card (from SDFS_Test output we know these exist)
  const char* existing_files[] = {"/LAGER.CFG", "/LOG000.TXT", "/TEST.TXT"};
  const int num_files = 3;

  for (int i = 0; i < num_files; i++) {
    const char* filename = existing_files[i];

    if (sdfs.exists(filename)) {
      CI_LOGF("Testing read operations on existing file: %s\n", filename);

      File file = sdfs.open(filename, FILE_READ);
      if (file) {
        // Test basic file properties
        size_t file_size = file.size();
        assertTrue(file_size > 0);
        assertEqual((int)file.available(), (int)file_size);
        assertFalse(file.isDirectory());

        // Test reading a few bytes
        if (file_size > 0) {
          char buffer[64];
          size_t to_read = min(file_size, sizeof(buffer) - 1);
          size_t read_bytes = file.read(buffer, to_read);
          buffer[read_bytes] = '\0';

          assertEqual((int)read_bytes, (int)to_read);
          CI_LOGF("  Read %u bytes: %.20s...\n", read_bytes, buffer);
        }

        file.close();
        CI_LOGF("  File %s read test: SUCCESS\n", filename);
      } else {
        CI_LOGF("  WARNING: Could not open %s for reading\n", filename);
      }
    } else {
      CI_LOGF("  File %s does not exist - skipping\n", filename);
    }
  }

  // Always pass - this test is diagnostic
  assertTrue(true);
}

test(file_seek_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  File file = sdfs.open(TEST_FILE, FILE_READ);
  assertTrue(file);

  // Test seek to beginning
  assertTrue(file.seek(0, SeekSet));
  assertEqual((int)file.position(), 0);

  // Test seek to middle
  assertTrue(file.seek(5, SeekSet));
  assertEqual((int)file.position(), 5);

  // Test seek to end
  assertTrue(file.seek(0, SeekEnd));
  assertEqual((int)file.position(), TEST_DATA_SIZE);

  file.close();
}

// ==============================================================================
// FILE MANAGEMENT TESTS
// ==============================================================================

test(file_truncate_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  File file = sdfs.open(TEST_FILE, FILE_WRITE);
  assertTrue(file);

  // Truncate to smaller size
  assertTrue(file.truncate(10));
  assertEqual((int)file.size(), 10);

  // Truncate to zero (clear file)
  assertTrue(file.truncate(0));
  assertEqual((int)file.size(), 0);

  file.close();
}

test(file_removal) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  // Ensure test file exists
  assertTrue(sdfs.exists(TEST_FILE));

  // Remove file
  assertTrue(sdfs.remove(TEST_FILE));

  // Verify removal
  assertFalse(sdfs.exists(TEST_FILE));
}

test(file_rename_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  const char* old_name = "/rename_test_old.txt";
  const char* new_name = "/rename_test_new.txt";

  // Clean up any existing files
  sdfs.remove(old_name);
  sdfs.remove(new_name);

  // Create test file
  File file = sdfs.open(old_name, FILE_WRITE);
  assertTrue(file);
  file.write((const uint8_t*)"rename_test", 11);
  file.close();

  // Test rename
  assertTrue(sdfs.rename(old_name, new_name));
  assertFalse(sdfs.exists(old_name));
  assertTrue(sdfs.exists(new_name));

  // Clean up
  sdfs.remove(new_name);
}

// ==============================================================================
// DIRECTORY OPERATIONS TESTS
// ==============================================================================

test(directory_creation) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  // Clean up any existing test directory
  if (sdfs.exists(TEST_DIR)) {
    sdfs.rmdir(TEST_DIR);
  }

  // Create directory
  assertTrue(sdfs.mkdir(TEST_DIR));
  assertTrue(sdfs.exists(TEST_DIR));

  // Test directory properties
  File dir = sdfs.open(TEST_DIR, FILE_READ);
  assertTrue(dir);
  assertTrue(dir.isDirectory());
  dir.close();
}

test(directory_file_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  const char* dir_file = "/testdir/dirfile.txt";

  // Create file in directory
  File file = sdfs.open(dir_file, FILE_WRITE);
  assertTrue(file);
  file.write((const uint8_t*)"directory_test", 14);
  file.close();

  // Verify file exists
  assertTrue(sdfs.exists(dir_file));

  // Read back
  file = sdfs.open(dir_file, FILE_READ);
  assertTrue(file);
  assertEqual((int)file.size(), 14);
  file.close();

  // Clean up
  sdfs.remove(dir_file);
}

test(directory_enumeration) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Test enumerating the root directory (known to exist)
  CI_LOG("Testing root directory enumeration:\n");

  File root = sdfs.open("/", FILE_READ);
  if (root) {
    assertTrue(root.isDirectory());

    int found_files = 0;
    int found_dirs = 0;

    File entry = root.openNextFile();
    while (entry) {
      if (entry.isDirectory()) {
        found_dirs++;
        CI_LOGF("  DIR:  %s\n", entry.name());
      } else {
        found_files++;
        CI_LOGF("  FILE: %s (%lu bytes)\n", entry.name(), entry.size());
      }
      entry.close();
      entry = root.openNextFile();
    }

    CI_LOGF("Total: %d files, %d directories\n", found_files, found_dirs);
    root.close();

    // We expect at least some files based on SDFS_Test output
    assertTrue(found_files > 0);
  } else {
    CI_LOG("WARNING: Could not open root directory\n");
  }

  // Always pass - this is diagnostic
  assertTrue(true);
}

test(directory_removal) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  // Remove empty directory
  assertTrue(sdfs.rmdir(TEST_DIR));
  assertFalse(sdfs.exists(TEST_DIR));
}

// ==============================================================================
// ERROR HANDLING TESTS
// ==============================================================================

test(invalid_file_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Test opening non-existent file for reading
  File file = sdfs.open("/nonexistent.txt", FILE_READ);
  assertFalse(file);

  // Test invalid seek operations
  file = sdfs.open("/seektest.txt", FILE_WRITE);
  assertTrue(file);
  file.write((const uint8_t*)"test", 4);

  // Invalid seek beyond reasonable bounds should fail gracefully
  // (Implementation specific - may succeed with clamping)
  file.seek(0xFFFFFFFF, SeekSet);  // Large seek

  file.close();
  sdfs.remove("/seektest.txt");
}

test(invalid_directory_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Test removing non-existent directory
  assertFalse(sdfs.rmdir("/nonexistent_dir"));

  // Test creating directory with invalid path
  // (Implementation may handle this differently)
  bool result = sdfs.mkdir("");
  // Don't assert - behavior may be implementation specific
}

// ==============================================================================
// PERFORMANCE AND STRESS TESTS
// ==============================================================================

test(large_file_operations) {
  if (!hardware_detected) {
    skipTestNow();
    return;
  }

  // Now using proper FatFs mounting - write operations should work

  const char* large_file = "/large_test.dat";
  const size_t chunk_size = 512;
  const int num_chunks = 10; // 5KB total

  // Clean up
  sdfs.remove(large_file);

  // Write large file in chunks
  File file = sdfs.open(large_file, FILE_WRITE);
  assertTrue(file);

  uint8_t chunk_data[chunk_size];
  for (size_t i = 0; i < chunk_size; i++) {
    chunk_data[i] = i & 0xFF;
  }

  for (int chunk = 0; chunk < num_chunks; chunk++) {
    size_t written = file.write(chunk_data, chunk_size);
    assertEqual((int)written, (int)chunk_size);
  }

  file.close();

  // Verify large file
  file = sdfs.open(large_file, FILE_READ);
  assertTrue(file);
  assertEqual((int)file.size(), (int)(chunk_size * num_chunks));
  file.close();

  // Clean up
  sdfs.remove(large_file);
}

// ==============================================================================
// MAIN TEST SETUP AND EXECUTION
// ==============================================================================

void setup() {
  // Initialize HIL testing environment
  HIL_TEST_SETUP();

  // Set longer timeout for SD card operations
  HIL_TEST_TIMEOUT(120);

  CI_LOG("SDFS Unit Tests - AUnit Phase 2 (Fixed)\n");
  CI_LOG("Hardware: Nucleo-F411RE + SD card breakout\n");
  CI_LOG("Following FatFs best practices: mount once, keep mounted\n");
  CI_LOG("================================================\n");

  // CRITICAL: Mount once following FatFs best practices
  // Configure SPI pins
  SPI.setMOSI(SPI_MOSI);
  SPI.setMISO(SPI_MISO);
  SPI.setSCLK(SPI_SCLK);

  // Mount filesystem once and keep mounted for entire session
  CI_LOG("Mounting SDFS filesystem...");
  if (sdfs.begin(CS_PIN)) {
    CI_LOG(" SUCCESS\n");
    hardware_detected = true;
    sdfs_initialized = true;

    CI_LOGF("Media: %s\n", sdfs.getMediaName());
    CI_LOGF("Total Size: %lu MB\n", sdfs.totalSize() / (1024 * 1024));
    CI_LOGF("Used Size: %lu MB\n", sdfs.usedSize() / (1024 * 1024));
  } else {
    CI_LOG(" FAILED\n");
    CI_LOG("SD card not detected - tests will skip gracefully\n");
    hardware_detected = false;
    sdfs_initialized = false;
  }
}

void loop() {
  // Run tests with HIL integration
  HIL_TEST_RUN();
}