// aunit_hil.h - AUnit HIL Integration Shim
// Integrates AUnit v1.7.1 with existing ci_log.h RTT/Serial abstraction and HIL workflow
#pragma once

#include "ci_log.h"
#include <AUnit.h>

// Custom printer that redirects AUnit output to ci_log system
class HILPrinter : public Print {
public:
  size_t write(uint8_t c) override {
    char str[2] = {(char)c, '\0'};
    CI_LOG(str);
    return 1;
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    for (size_t i = 0; i < size; i++) {
      write(buffer[i]);
    }
    return size;
  }
};

// HIL-specific TestRunner wrapper that adds exit wildcard functionality
namespace aunit_hil {

class HILTestRunner {
private:
  static HILPrinter hilPrinter;
  static bool isSetup;

public:
  // Initialize HIL testing environment
  static void setup() {
    if (isSetup) return;

    // Set AUnit to use our HIL printer
    aunit::TestRunner::setPrinter(&hilPrinter);

    // Enable all verbosity for comprehensive test output
    aunit::TestRunner::setVerbosity(aunit::Verbosity::kAll);

    isSetup = true;

    CI_LOG("=== AUnit HIL Test Runner ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();
  }

  // Run tests with HIL integration
  static void run() {
    setup();

    // Run AUnit tests
    aunit::TestRunner::run();

    // Check if all tests are complete
    // AUnit sets Test::getRoot() to nullptr when all tests are finished
    if (*aunit::Test::getRoot() == nullptr) {
      emitTestCompletion();
    }
  }

  // Emit HIL-compatible test completion signal
  static void emitTestCompletion() {
    CI_LOG("=== AUnit HIL Test Complete ===\n");

    // Get test statistics from TestRunner internals
    // Note: We'll need to access private members via friendship or public accessors
    CI_LOG("Tests completed - check output above for results\n");

    // Emit exit wildcard for deterministic HIL completion
    CI_LOG("*STOP*\n");
  }

  // Convenience methods that delegate to AUnit TestRunner
  static void exclude(const char* pattern) {
    aunit::TestRunner::exclude(pattern);
  }

  static void include(const char* pattern) {
    aunit::TestRunner::include(pattern);
  }

  static void setTimeout(uint16_t seconds) {
    aunit::TestRunner::setTimeout(seconds);
  }
};

// Static member definitions
HILPrinter HILTestRunner::hilPrinter;
bool HILTestRunner::isSetup = false;

} // namespace aunit_hil

// Convenience macros for HIL testing
#define HIL_TEST_SETUP() aunit_hil::HILTestRunner::setup()
#define HIL_TEST_RUN() aunit_hil::HILTestRunner::run()
#define HIL_TEST_EXCLUDE(pattern) aunit_hil::HILTestRunner::exclude(pattern)
#define HIL_TEST_INCLUDE(pattern) aunit_hil::HILTestRunner::include(pattern)
#define HIL_TEST_TIMEOUT(seconds) aunit_hil::HILTestRunner::setTimeout(seconds)