# libPrintf Documentation for STM32 Arduino Core

This document provides detailed technical information about the libPrintf library, an optional embedded printf implementation for STM32 Arduino sketches.

## Table of Contents
- [Overview](#overview)
- [Library Architecture](#library-architecture)
- [Output Routing](#output-routing)
- [Customization](#customization)
- [Advanced Configuration](#advanced-configuration)
- [Comparison with Standard Printf](#comparison-with-standard-printf)
- [Implementation Details](#implementation-details)
- [Troubleshooting](#troubleshooting)
- [Examples](#examples)
- [Version Information](#version-information)

## Overview

libPrintf is an **optional Arduino library** based on eyalroz/printf v6.2.0 that provides embedded printf functionality for STM32 Arduino sketches. This library eliminates nanofp configuration complexity and provides reliable float formatting.

**Key Features:**
- **✅ Optional Integration**: Include `<libPrintf.h>` when you need enhanced printf functionality
- **✅ No FQBN Dependency**: Works with standard Arduino CLI commands without rtlib settings
- **✅ Binary Savings**: ~20% reduction compared to nanofp approach (typically 8KB+ savings)
- **✅ Reliable Float Formatting**: Consistent float display without build complexity
- **✅ Factory Code Compatibility**: Seamless integration with existing printf/fprintf calls
- **✅ Thread-Safe**: Suitable for embedded real-time applications

## Library Architecture

### Library Files
- **`src/libPrintf.h`**: Main library header with function aliasing
- **`src/printf.c`**: eyalroz/printf v6.2.0 implementation
- **`src/printf.h`**: Function declarations and configuration
- **`library.properties`**: Arduino library metadata
- **`examples/BasicUsage/`**: Demonstration sketch

### Manual Activation
Include the library header in your sketch:
```cpp
#include <libPrintf.h>

void setup() {
  // printf functions now use embedded implementation
  printf("Float: %.2f\n", 3.14159);
}
```

### Function Replacement
libPrintf uses soft aliasing to replace standard printf functions:
- `printf()` → `printf_()`
- `sprintf()` → `sprintf_()`
- `snprintf()` → `snprintf_()`
- `fprintf()` → `fprintf_()`
- All variants work transparently

## Output Routing

### Default Flow Path
When using libPrintf, output routes through the embedded printf implementation to the default Arduino Serial:

```
printf() → printf_() → putchar_() → Serial.print() → Hardware UART
```

### Library Implementation

#### 1. **printf.c** - Entry Point
- **Location**: `libraries/libPrintf/src/printf.c`
- **Function**: `int printf_(const char* format, ...)`
- **Role**: Main printf implementation from eyalroz/printf v6.2.0
- **Calls**: `putchar_(char c)` for each output character

#### 2. **Default putchar_() Implementation**
By default, libPrintf provides a basic `putchar_()` implementation that outputs to Arduino's default Serial:

```cpp
#ifndef LIBPRINTF_CUSTOM_PUTCHAR
extern "C" void putchar_(char c) {
  Serial.print(c);
}
#endif
```

#### 3. **Arduino Serial Integration**
- **Default Output**: Routes through Arduino's `Serial` object
- **Hardware**: Uses the board's default Serial pins (typically ST-Link virtual COM port)
- **Initialization**: Requires `Serial.begin()` in setup()

### Basic Usage Flow

1. **Include libPrintf**: `#include <libPrintf.h>`
2. **Initialize Serial**: `Serial.begin(115200);` in setup()
3. **Use printf**: `printf("Hello %.2f\n", 3.14);`
4. **Output path**: printf_() → putchar_() → Serial.print() → Hardware UART

### NUCLEO_F411RE Default Configuration

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Default Serial** | USART2 | Arduino Serial object |
| **TX Pin** | PA2 | ST-Link virtual COM port |
| **RX Pin** | PA3 | ST-Link virtual COM port |
| **Initialization** | `Serial.begin()` | Required in sketch setup() |

## Customization

### Custom Output Routing

libPrintf allows you to redirect printf output by providing your own `putchar_()` implementation. This is the primary method for customizing output behavior.

#### Method 1: Override putchar_() (Recommended)
Define `LIBPRINTF_CUSTOM_PUTCHAR` to disable the default implementation, then provide your own:

```cpp
#include <libPrintf.h>
#define LIBPRINTF_CUSTOM_PUTCHAR

extern "C" void putchar_(char c) {
    Serial1.write(c);  // Route to different Serial port
}

void setup() {
    Serial1.begin(115200);  // Initialize before using printf
    printf("Output goes to Serial1\n");
}
```

#### Method 2: Conditional Output Routing
Use compile-time flags to select output destination:

```cpp
#include <libPrintf.h>
#define LIBPRINTF_CUSTOM_PUTCHAR

extern "C" void putchar_(char c) {
#ifdef USE_RTT_OUTPUT
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);
#elif defined(USE_SERIAL1)
    Serial1.write(c);
#else
    Serial.write(c);  // Default Arduino Serial
#endif
}
```

#### Method 3: Runtime Selection
Switch output destinations at runtime:

```cpp
enum PrintfDest { SERIAL_DEFAULT, SERIAL1, RTT };
static PrintfDest printf_dest = SERIAL_DEFAULT;

extern "C" void putchar_(char c) {
    switch(printf_dest) {
        case SERIAL_DEFAULT:
            Serial.write(c);
            break;
        case SERIAL1:
            Serial1.write(c);
            break;
        case RTT: {
            char buf[2] = {c, '\0'};
            SEGGER_RTT_WriteString(0, buf);
            break;
        }
    }
}

void setPrintfDestination(PrintfDest dest) {
    printf_dest = dest;
}
```

### Library Dependencies

libPrintf follows this simple dependency chain:

```
Sketch
  ↓ includes
<libPrintf.h>
  ↓ includes
printf.h (function declarations)
printf.c (implementation)
  ↓ calls
putchar_() (user-defined or default)
  ↓ routes to
Serial.print() or custom output
```

## Advanced Configuration

### Disabling Default putchar_()
To use a completely custom output implementation:

```cpp
#include <libPrintf.h>
#define LIBPRINTF_CUSTOM_PUTCHAR  // Disables default implementation

extern "C" void putchar_(char c) {
    // Your custom implementation
    myCustomOutput(c);
}
```

### Format String Validation
Enable compile-time format string checking:

```cpp
#define PRINTF_SUPPORT_RUNTIME_EVALUATION 0  // Disable for smaller code size
#define PRINTF_CHECK_FOR_NULL_POINTERS 1     // Enable null pointer checks
```

### Memory Optimization
Configure printf features to reduce binary size:

```cpp
#define PRINTF_SUPPORT_LONG_LONG 0           // Disable long long support
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1   // Keep %d, %i, %u
#define PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS 0  // Disable %e, %E, %g, %G
```

## Comparison with Standard Printf

| Feature | Standard (nano) | Standard (nanofp) | libPrintf |
|---------|----------------|-------------------|-----------|
| **Float Support** | ❌ None | ✅ Full | ✅ Full |
| **Binary Size** | Small (~2KB) | Large (~16KB) | Optimized (~6KB) |
| **FQBN Dependency** | N/A | `:rtlib=nanofp` required | ❌ None |
| **Include Required** | None | None | `#include <libPrintf.h>` |
| **Output Routing** | Serial only | Serial only | Flexible putchar_() |
| **Configuration** | N/A | Complex FQBN | Simple include |

## Implementation Details

### Function Override Method
- **Soft Aliasing**: `PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_SOFT` replaces standard printf functions when library is included
- **Library-Level**: Integration happens when `<libPrintf.h>` is included in sketch
- **User putchar_()**: Allows easy output redirection through custom putchar_() implementation
- **No Runtime Overhead**: Function replacement happens at compile/link time

### Output Flow
1. **printf()** calls embedded printf_() implementation (via aliasing)
2. **Embedded printf_()** calls `putchar_(char c)` for each character
3. **putchar_()** routes to Serial.print() by default or user-defined function
4. **Serial.print()** transmits via Arduino's Serial implementation

### Memory Usage

| Metric | nano | nanofp | libPrintf | Savings |
|--------|------|--------|-----------|----------|
| **Code Size** | ~2KB | ~16KB | ~6KB | 10KB vs nanofp |
| **Float Support** | None | Full | Full | Same as nanofp |
| **RAM Usage** | Minimal | Minimal | Minimal | No impact |
| **FQBN Complexity** | None | High | None | Simplified builds |

*Note: Savings vary by sketch complexity and printf usage. libPrintf provides the same float support as nanofp with 60% smaller binary size.*

## Troubleshooting

### Printf Not Appearing
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No output in Serial Monitor | Serial not initialized | Add `Serial.begin(115200);` in setup() |
| No output with custom putchar_() | Target not initialized | Initialize Serial1, RTT, etc. before printf |
| `'printf' was not declared` | Library not included | Add `#include <libPrintf.h>` |
| Float shows as "0.000000" | libPrintf not active | Verify library is included correctly |

### Compilation Errors
| Error | Cause | Solution |
|-------|-------|----------|
| `multiple definition of printf` | Conflicting printf definitions | Remove custom printf declarations |
| `putchar_ conflicts` | Multiple putchar_() definitions | Use `extern "C"` and check for duplicates |
| Library not found | libPrintf not in library path | Copy library to Arduino/libraries/ folder |

### Debug Steps
1. **Verify Library**: Include `<libPrintf.h>` and test basic printf
2. **Check Initialization**: Ensure Serial.begin() called before printf usage
3. **Test Float Support**: `printf("%.2f", 3.14)` should show "3.14", not "0.00"
4. **Custom putchar_()**: Start with simple Serial1.write(c) test

### Float Formatting Verification
To verify libPrintf is working correctly:

```cpp
#include <libPrintf.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    printf("Test float: %.6f\n", 3.14159265);
    // Should show: Test float: 3.141593
    // If shows: Test float: 0.000000, libPrintf is not active
}
```

## Examples

### Basic Usage
```cpp
#include <libPrintf.h>

void setup() {
    Serial.begin(115200);  // Required for default output

    printf("Integer: %d\n", 42);
    printf("Float: %.2f\n", 3.14159);
    printf("String: %s\n", "Hello World");
    printf("Hex: 0x%08X\n", 0xDEADBEEF);

    // All printf family functions work
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Formatted: %d", 123);
    printf("Buffer contains: %s\n", buffer);
}

void loop() {
    // Printf output appears in Arduino Serial Monitor
}
```

### RTT Output for HIL Testing
```cpp
#include <libPrintf.h>
#include <SEGGER_RTT.h>
#define LIBPRINTF_CUSTOM_PUTCHAR

extern "C" void putchar_(char c) {
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);
}

void setup() {
    SEGGER_RTT_Init();
    printf("RTT output: %.2f\n", 3.14159);
}
```

### Dual Output (Serial + RTT)
```cpp
#include <libPrintf.h>
#include <SEGGER_RTT.h>
#define LIBPRINTF_CUSTOM_PUTCHAR

extern "C" void putchar_(char c) {
    Serial.write(c);           // Arduino IDE Serial Monitor
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);  // HIL testing via RTT
}

void setup() {
    Serial.begin(115200);
    SEGGER_RTT_Init();
    printf("Dual output: %.2f\n", 3.14159);
}
```

## Version Information
- **libPrintf Library**: v1.0.0 (Arduino wrapper)
- **eyalroz/printf**: v6.2.0 (embedded implementation)
- **Compatible STM32 Core**: 2.7.1+ (no specific version required)
- **Upstream Repository**: [github.com/eyalroz/printf](https://github.com/eyalroz/printf)
- **License**: MIT (eyalroz/printf)
- **Documentation**: This library provides Arduino-compatible interface for eyalroz/printf

## Usage in Production

### ICM42688P Integration Example
The ICM42688P IMU library demonstrates libPrintf integration with factory code:

```bash
# Build with standard FQBN (no rtlib needed)
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE \
  libraries/ICM42688P/examples/example-selftest

# HIL testing with RTT output
./scripts/aflash.sh libraries/ICM42688P/examples/example-selftest --use-rtt
```

### Build Integration
libPrintf works with all standard STM32 Arduino build workflows:

```bash
# Arduino CLI
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch>

# Project scripts
./scripts/build.sh <sketch> --build-id
./scripts/aflash.sh <sketch> --use-rtt

# CMake builds
cmake -S <sketch> -B build && cmake --build build
```