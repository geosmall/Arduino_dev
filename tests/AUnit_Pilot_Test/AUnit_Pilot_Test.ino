#include "../../aunit_hil.h"

// Basic functionality tests
test(basic_arithmetic) {
  assertEqual(2 + 2, 4);
  assertEqual(10 - 3, 7);
  assertEqual(3 * 4, 12);
}

test(boolean_logic) {
  assertTrue(true);
  assertFalse(false);
  assertEqual(true && true, true);
  assertEqual(true || false, true);
}

test(string_operations) {
  String test_string = "Hello";
  assertEqual((int)test_string.length(), 5);
  assertTrue(test_string.equals("Hello"));
}

// Test that should fail (commented out by default)
// test(intentional_failure) {
//   assertEqual(1, 2);  // This will fail
// }

void setup() {
  // Initialize HIL testing environment
  HIL_TEST_SETUP();

  // Optional: Configure test filtering
  // HIL_TEST_EXCLUDE("intentional_failure");

  // Optional: Set timeout (default is 10 seconds)
  HIL_TEST_TIMEOUT(30);
}

void loop() {
  // Run tests with HIL integration
  HIL_TEST_RUN();
}