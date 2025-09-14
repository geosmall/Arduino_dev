# AUnit Testing Framework Integration

AUnit v1.7.1 integration with STM32 Arduino HIL testing workflow.

## Architecture

**AUnit Core**
- TestRunner singleton manages execution lifecycle
- Tests complete when `*Test::getRoot() == nullptr`
- Configurable output via `TestRunner::setPrinter()`

**HIL Integration (aunit_hil.h)**
- HILPrinter redirects AUnit output through `ci_log.h`
- HILTestRunner wraps TestRunner with exit wildcard emission
- Zero-modification approach preserves AUnit compatibility

## Usage

**Basic Test Structure**
```cpp
#include "../../aunit_hil.h"

test(basic_arithmetic) {
  assertEqual(2 + 2, 4);
  assertTrue(true);
}

void setup() {
  HIL_TEST_SETUP();
}

void loop() {
  HIL_TEST_RUN();
}
```

**Build Commands**
```bash
# RTT Mode (HIL testing)
./scripts/aflash.sh tests/AUnit_Pilot_Test --use-rtt --build-id

# Serial Mode (Arduino IDE)
./scripts/build.sh tests/AUnit_Pilot_Test --build-id

# Clean cache (file moves, CI/CD)
./scripts/aflash.sh tests/AUnit_Pilot_Test --clean-cache --use-rtt --build-id
```

## Assertion Types

```cpp
// Equality
assertEqual(expected, actual);
assertNotEqual(expected, actual);

// Boolean
assertTrue(condition);
assertFalse(condition);

// Comparison
assertLess(a, b);
assertMore(a, b);
assertLessOrEqual(a, b);
assertMoreOrEqual(a, b);

// Floating point
assertNear(expected, actual, tolerance);
```

## Type Compatibility

```cpp
// String handling (avoid type ambiguity)
assertEqual((int)str.length(), 5);
assertTrue(str.equals("expected"));

// Numeric types: bool, char, int, long, double supported
```

## HIL Workflow

**Expected Output**
```
=== AUnit HIL Test Runner ===
Build: 128f229 (2025-09-14T21:52:37Z)
READY NUCLEO_F411RE 128f229 2025-09-14T21:52:37Z
TestRunner started on 3 test(s).
Test basic_arithmetic passed.
*STOP*
```

**Performance**
- RTT Mode: 13,272 bytes, deterministic completion
- Serial Mode: 11,680 bytes, Arduino IDE compatible

## Directory Structure

```
/Arduino/
├── aunit_hil.h           # HIL integration shim
├── tests/                # All project tests
│   └── AUnit_Pilot_Test/ # Demonstration
└── libraries/AUnit-1.7.1/  # External library
```

## Advanced Features

**Test Filtering**
```cpp
HIL_TEST_INCLUDE("arithmetic*");
HIL_TEST_EXCLUDE("slow_*");
HIL_TEST_TIMEOUT(30);
```

**Custom Classes**
```cpp
class CustomTest : public aunit::TestOnce {
  void setup() override { /* setup */ }
  void teardown() override { /* cleanup */ }
};

testF(CustomTest, my_test) {
  // Test with custom setup/teardown
}
```

## Troubleshooting

**Common Issues**
```cpp
// Type errors
assertEqual((int)str.length(), 5);  // Cast for clarity

// Include paths
#include "../../aunit_hil.h"  // From tests/TestName/
```

**Cache Issues**
```bash
# Stale file paths in output
./scripts/build.sh tests/TestName --clean-cache --use-rtt
```

**Hardware Issues**
```bash
# J-Link problems
lsusb | grep -i segger
pkill -f JLinkGDBServer
```

## Integration Benefits

- Single codebase works in Arduino IDE and HIL modes
- Deterministic completion via exit wildcards
- Build traceability with git SHA + timestamps
- Leverages existing J-Run/RTT infrastructure
- Ready for Phase 2 library-level testing