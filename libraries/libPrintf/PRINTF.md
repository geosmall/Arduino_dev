# Printf Integration for STM32 Arduino Core

This document explains the embedded printf implementation that provides automatic, system-wide printf functionality for all STM32 Arduino sketches.

## Table of Contents
- [Overview](#overview)
- [Integration Architecture](#integration-architecture)
- [Output Routing](#output-routing)
- [Customization](#customization)
- [Advanced Configuration](#advanced-configuration)
- [Comparison with Standard Printf](#comparison-with-standard-printf)
- [Implementation Details](#implementation-details)
- [Troubleshooting](#troubleshooting)
- [Examples](#examples)
- [Version Information](#version-information)

## Overview

The STM32 Arduino Core includes **eyalroz/printf v6.2.0** embedded printf library, which replaces the problematic newlib printf system. This provides:

- **✅ Automatic Operation**: Works in all sketches without configuration
- **✅ No FQBN Dependency**: Works regardless of rtlib setting (nano/nanofp/full)
- **✅ Binary Savings**: 20KB+ reduction vs nanofp approach
- **✅ Reliable Float Formatting**: Consistent float display without build complexity
- **✅ Factory Code Compatibility**: Preserves existing manufacturer drivers

## Integration Architecture

### Core Files
- **`arduino_printf.h`**: Main integration header with hard aliasing
- **`printf.c`**: eyalroz/printf v6.2.0 implementation
- **`printf.h`**: Function declarations and configuration
- **`../syscalls.c`**: putchar_() implementation for output routing

### Automatic Activation
```cpp
// Arduino.h automatically includes:
#include "libraries/printf/arduino_printf.h"

// Which enables hard aliasing:
#define PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_HARD 1
```

All standard printf functions are automatically replaced:
- `printf()` → `printf_()`
- `sprintf()` → `sprintf_()`
- `snprintf()` → `snprintf_()`
- `vprintf()` → `vprintf_()`

## Output Routing

### Default Flow Path
Printf output routes through multiple layers to reach hardware:

```
printf() → printf_() → putchar_() → uart_debug_write() → HAL_UART_Transmit() → Hardware
```

### Core Files and Functions

#### 1. **printf.c** - Entry Point
- **Location**: `cores/arduino/libraries/printf/printf.c`
- **Function**: `int printf_(const char* format, ...)`
- **Role**: Main printf implementation, processes format string and arguments
- **Calls**: `putchar_(char c)` for each output character

#### 2. **syscalls.c** - Output Routing
- **Location**: `cores/arduino/syscalls.c`
- **Function**: `void putchar_(char c)`
- **Role**: Routes characters to hardware output system
- **Default Implementation**:
```cpp
void putchar_(char c) {
#if defined(HAL_UART_MODULE_ENABLED) && !defined(HAL_UART_MODULE_ONLY)
    uart_debug_write((uint8_t *)&c, 1);
#else
    (void)c;  // Discard if no UART available
#endif
}
```

#### 3. **stm32/uart.c** - Hardware Interface
- **Location**: `cores/arduino/stm32/uart.c`
- **Function**: `size_t uart_debug_write(uint8_t *data, uint32_t size)`
- **Role**: Manages debug UART and transmits data via STM32 HAL
- **Key Operations**:
  - Auto-detects and initializes debug UART
  - Routes to `HAL_UART_Transmit()`

#### 4. **Board Variant** - Pin Configuration
- **Location**: `variants/STM32F4xx/F411R(C-E)T/variant_NUCLEO_F411RE.h`
- **Defines**: Pin mappings for debug UART
- **Location**: `variants/STM32F4xx/F411R(C-E)T/PeripheralPins.c`
- **Mapping**: Pin to UART peripheral relationships

### Default Configuration Chain

#### Debug UART Detection (uart.c)
```cpp
// Default DEBUG_UART selection
#if !defined(DEBUG_UART)
  #if defined(PIN_SERIAL_TX)
    #define DEBUG_UART pinmap_peripheral(digitalPinToPinName(PIN_SERIAL_TX), PinMap_UART_TX)
  #else
    #define DEBUG_UART NP  // No peripheral
  #endif
#endif
```

#### Board Pin Definitions (variant_NUCLEO_F411RE.h)
```cpp
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX         PA2    // USART2 TX pin
#endif
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX         PA3    // USART2 RX pin
#endif
```

#### Pin-to-Peripheral Mapping (PeripheralPins.c)
```cpp
WEAK const PinMap PinMap_UART_TX[] = {
  {PA_2,  USART2, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART2)},
  {PA_9,  USART1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},
  // ...
};
```

#### UART Configuration (uart.c)
```cpp
#if !defined(DEBUG_UART_BAUDRATE)
#define DEBUG_UART_BAUDRATE 9600
#endif

// In uart_debug_init():
uart_init(&serial_debug, DEBUG_UART_BAUDRATE, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
```

### Complete Default Flow for NUCLEO_F411RE

1. **PIN_SERIAL_TX** = PA2 (variant header)
2. **DEBUG_UART** = pinmap_peripheral(PA2) = **USART2** (uart.c + PeripheralPins.c)
3. **DEBUG_UART_BAUDRATE** = **9600** (uart.c)
4. **printf()** → **putchar_()** → **uart_debug_write()** → **USART2 @ 9600 baud**
5. **PA2 pin** → **ST-Link** → **USB Virtual COM Port**

## Customization

### Customization Hierarchy

The printf output can be customized at four different levels, from simple sketch overrides to deep hardware modifications:

#### Level 1: Override putchar_() (Sketch Level)
**Recommended for most use cases**

⚠️ **Important**: Always ensure your target output is properly initialized before printf usage.

```cpp
extern "C" void putchar_(char c) {
    Serial1.write(c);  // Route to different Serial port
}

void setup() {
    Serial1.begin(115200);  // Initialize before using printf
    printf("Safe to use printf now\n");
}
```

#### Level 2: Redefine DEBUG_UART (Sketch Level)
**For changing UART peripheral and configuration**
```cpp
#define DEBUG_UART USART1
#define DEBUG_PINNAME_TX PA9
#define DEBUG_PINNAME_RX PA10
#define DEBUG_UART_BAUDRATE 115200
#include <Arduino.h>
```

#### Level 3: Modify Board Variant (Board Level)
**For permanent board-wide changes**
```cpp
// In variant_BOARD.h
#define PIN_SERIAL_TX         PA9   // Use USART1 instead of USART2
#define PIN_SERIAL_RX         PA10
```

#### Level 4: Custom PeripheralPins.c (Hardware Level)
**For custom hardware or non-standard pin assignments**
```cpp
// Modify pin-to-peripheral mappings
WEAK const PinMap PinMap_UART_TX[] = {
  {PA_9,  USART1, STM_PIN_DATA(STM_MODE_AF_PP, GPIO_PULLUP, GPIO_AF7_USART1)},
  // Custom pin assignments
};
```

### File Dependencies

The printf integration follows this dependency chain:

```
Arduino.h
  ↓ includes
arduino_printf.h
  ↓ includes
printf.h (function declarations)
printf.c (implementation)
  ↓ calls
putchar_() in syscalls.c
  ↓ calls
uart_debug_write() in stm32/uart.c
  ↓ uses
DEBUG_UART from variant header + PeripheralPins.c
  ↓ transmits via
HAL_UART_Transmit() to hardware UART
```

#### NUCLEO_F411RE Default Configuration
| Parameter | Value | Source |
|-----------|-------|--------|
| **Debug UART** | USART2 | PeripheralPins.c mapping |
| **TX Pin** | PA2 | variant_NUCLEO_F411RE.h |
| **RX Pin** | PA3 | variant_NUCLEO_F411RE.h |
| **Baud Rate** | 9600 | DEBUG_UART_BAUDRATE |
| **Format** | 8N1 | uart_debug_init() |
| **Output** | ST-Link Virtual COM | Hardware routing |

### Custom Output Routing Examples

Override `putchar_()` in your sketch to redirect printf output:

#### Route to Different Serial Port
```cpp
extern "C" void putchar_(char c) {
    Serial1.write(c);  // Route to USART1
}

void setup() {
    Serial1.begin(115200);
    printf("Output goes to Serial1\n");
}
```

#### Route to RTT (Real-Time Transfer)
```cpp
#include <SEGGER_RTT.h>

extern "C" void putchar_(char c) {
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);
}

void setup() {
    SEGGER_RTT_Init();
    printf("Output goes to RTT\n");
}
```

#### Runtime Selection
```cpp
enum PrintfDest { UART_DEBUG, UART1, RTT };
static PrintfDest dest = UART_DEBUG;

extern "C" void putchar_(char c) {
    switch(dest) {
        case UART_DEBUG:
            uart_debug_write((uint8_t*)&c, 1);
            break;
        case UART1:
            Serial1.write(c);
            break;
        case RTT: {
            char buf[2] = {c, '\0'};
            SEGGER_RTT_WriteString(0, buf);
            break;
        }
    }
}

void setPrintfDestination(PrintfDest d) { dest = d; }
```

## Advanced Configuration

### Custom Debug UART
Define before including Arduino.h:
```cpp
#define DEBUG_UART USART1
#define DEBUG_PINNAME_TX PA9
#define DEBUG_PINNAME_RX PA10
#define DEBUG_UART_BAUDRATE 115200
#include <Arduino.h>
```

### Board Variant Modification
Modify `PIN_SERIAL_TX`/`PIN_SERIAL_RX` in variant header:
```cpp
// In variant_BOARD.h
#define PIN_SERIAL_TX         PA9   // USART1 instead of USART2
#define PIN_SERIAL_RX         PA10
```

## Comparison with Standard Printf

| Feature | Standard (nano) | Standard (nanofp) | Embedded Printf |
|---------|----------------|-------------------|-----------------|
| **Float Support** | ❌ None | ✅ Full | ✅ Full |
| **Binary Size** | Small | +10KB bloat | Optimized |
| **FQBN Dependency** | N/A | Required | ❌ None |
| **Output Routing** | None | _write() only | Flexible putchar_() |
| **Configuration** | N/A | Complex | Simple override |

## Implementation Details

### Function Override Method
- **Hard Aliasing**: `PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_HARD` replaces all standard printf functions
- **Early Inclusion**: Arduino.h includes printf headers before system headers to ensure override
- **Weak Functions**: `putchar_()` uses weak linkage allowing user override without conflicts
- **No Runtime Overhead**: Function replacement happens at compile/link time

### Output Flow
1. **printf()** calls embedded printf implementation
2. **Embedded printf** calls `putchar_(char c)` for each character
3. **putchar_()** routes to uart_debug_write() by default
4. **uart_debug_write()** transmits via STM32 HAL to hardware UART
5. **Hardware UART** outputs to ST-Link virtual COM port

### Memory Usage

| Metric | nano | nanofp | Embedded Printf | Savings |
|--------|------|--------|-----------------|----------|
| **Code Size** | ~2KB | ~16KB | ~6KB | 10KB vs nanofp |
| **Float Support** | None | Full | Full | - |
| **RAM Usage** | Minimal | Minimal | Minimal | No impact |
| **Total Savings** | N/A | +14KB | Baseline | 20KB+ vs nanofp |

*Note: Savings vary by sketch complexity and printf usage*

## Troubleshooting

### Printf Not Appearing
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No output in Serial Monitor | Wrong COM port selected | Check Arduino IDE port selection |
| Output to wrong destination | Custom putchar_() not initialized | Ensure Serial.begin() called before printf |
| Partial output | UART buffer overflow | Add delays or increase baud rate |
| Garbled output | Wrong baud rate | Match Serial Monitor baud to UART config |

### Compilation Errors
| Error | Cause | Solution |
|-------|-------|----------|
| `'printf' was not declared` | Arduino.h not included first | Move `#include <Arduino.h>` to top |
| `multiple definition of printf` | Conflicting printf definitions | Remove custom printf declarations |
| `putchar_ conflicts` | Non-weak putchar_() declaration | Use `extern "C" void putchar_(char c)` |

### Debug Steps
1. **Verify Integration**: Check that printf output works with default settings
2. **Test Custom Routing**: Start with simple Serial.write() override
3. **Check Initialization**: Ensure target Serial port is initialized with begin()
4. **Validate Hardware**: Confirm UART pins and connections are correct

### Float Formatting Issues
- **Not a Problem**: Embedded printf handles floats correctly regardless of rtlib setting
- **No FQBN Changes**: Standard builds work automatically
- **Consistent Output**: Same formatting across Serial and RTT output

## Examples

### Basic Usage
```cpp
void setup() {
    // No initialization required - printf works immediately
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
    // or your configured output destination
}
```

### Custom Output with Fallback
```cpp
extern "C" void putchar_(char c) {
    if (Serial1.availableForWrite()) {
        Serial1.write(c);
    } else {
        // Fallback to debug UART
        uart_debug_write((uint8_t*)&c, 1);
    }
}
```

### Conditional Routing
```cpp
extern "C" void putchar_(char c) {
#ifdef USE_RTT_OUTPUT
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);
#elif defined(USE_SERIAL1)
    Serial1.write(c);
#else
    // Default to debug UART
    uart_debug_write((uint8_t*)&c, 1);
#endif
}
```

## Version Information
- **eyalroz/printf Library**: v6.2.0
- **Integration Version**: STM32 Arduino Core 2.7.1+
- **Upstream Repository**: [github.com/eyalroz/printf](https://github.com/eyalroz/printf)
- **License**: MIT (eyalroz/printf) + LGPL 2.1 (Arduino Core)
- **Documentation**: This implementation follows eyalroz/printf API with Arduino-specific integration