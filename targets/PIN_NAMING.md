# Arduino Core STM32 Pin Naming Guide

This document explains the different pin naming conventions in Arduino Core STM32 and why our board configuration system uses Arduino pin macros.

**TLDR**: Arduino pin macros provide **hardware clarity** while maintaining **full compatibility** with existing Arduino code, avoiding mass changes while still providing **self-documenting** and **hardware-focused** board configurations.

## Arduino Core STM32 Pin Naming Confusion

### Multiple Pin Naming Systems

The Arduino Core STM32 has **three different** ways to refer to the same physical pin, which creates confusion:

**Example: The same physical pin PC12 can be referenced as:**
1. **Arduino Pin Number**: `44` (integer - varies by board variant)
2. **Arduino Pin Macro**: `PC12` (maps to the integer above) ← **We use this**
3. **Core PinName Constant**: `PC_12` (enum value from PinNames.h)

### Why This Creates Problems

**Different Values for Same Pin:**
```cpp
// All refer to the same physical pin PC12 on STM32F411:
uint8_t pin_number = 44;        // Arduino pin number (board-specific)
#define PC12 44                 // Arduino macro (from variant) ← **We use this**
PinName pin_name = PC_12;       // Core enum (consistent across boards)
```

**Board Variant Dependency:**
```cpp
// NUCLEO_F411RE variant:
#define PC12  44

// Different board variant might have:
#define PC12  52    // Same physical pin, different number!
```

**Arduino vs Core Approaches:**
```cpp
// Arduino pin macro approach (what we use)
SPI.setMOSI(PC12);  // Clear hardware pin, works directly

// Core PinName approach (alternative)
SPI.setMOSI(PC_12); // Direct PinName enum, requires conversion for Arduino functions
```

## Why We Use Arduino Pin Macros for Board Configuration

### 1. **Hardware Clarity with Arduino Compatibility**
```cpp
// BAD: Raw pin numbers - unclear and board-variant dependent
static constexpr SPIConfig storage{44, 43, 42, 41, 1000000, 8000000};
// What pins are these? Must check variant file.

// GOOD: Arduino pin macros - clear hardware pins, Arduino compatible
static constexpr SPIConfig storage{PC12, PC11, PC10, PD2, 1000000, 8000000};
// Clear which physical STM32 pins are used, works with all Arduino functions
```

### 2. **Zero Breaking Changes**
```cpp
// Arduino pin macros work directly with all existing Arduino functions:
pinMode(BoardConfig::storage.cs_pin, OUTPUT);        // ✅ Works directly
digitalWrite(BoardConfig::storage.cs_pin, HIGH);     // ✅ Works directly
SPIClass spi(BoardConfig::storage.mosi_pin, ...);    // ✅ Works directly

// No need for conversion functions, no code changes required
```

### 3. **Self-Documenting Configuration**
```cpp
// Confusing - what are these numbers?
static constexpr SPIConfig storage{44, 43, 42, 41, 1000000, 8000000};

// Clear - exactly which STM32 pins
static constexpr SPIConfig storage{PC12, PC11, PC10, PD2, 1000000, 8000000};
```

### 4. **Matches STM32 Documentation**
```cpp
// STM32 Reference Manual says "Connect MOSI to PC12"
// Our config clearly shows: PC12
// Raw numbers show: 44 (requires lookup to understand)
```

### 5. **Board Variant Compatibility**
```cpp
// Arduino pin macros are defined consistently across board variants:
// NUCLEO_F411RE: #define PC12 44
// BlackPill_F411: #define PC12 52  (different number, same macro name)

// Our config works on both:
static constexpr SPIConfig storage{PC12, PC11, PC10, PD2, 1000000, 8000000};
// Automatically gets correct pin numbers for each board variant
```

## Summary: Arduino Pin Macro Benefits for Board Configuration

| Aspect | Raw Pin Numbers | Arduino Pin Macros |
|--------|-----------------|-------------------|
| **Hardware Clarity** | `{44, 43, 42, 41}` ❌ | `{PC12, PC11, PC10, PD2}` ✅ |
| **Arduino Compatibility** | Works but unclear ❌ | Perfect compatibility ✅ |
| **Code Changes Required** | None, but confusing ❌ | Zero breaking changes ✅ |
| **STM32 Documentation Match** | Requires lookup ❌ | Direct match ✅ |
| **Board Variant Portability** | Variant-specific ❌ | Works across variants ✅ |
| **Self-Documenting** | Numbers mean nothing ❌ | Shows actual hardware ✅ |

## Arduino Pin Macro Reference

### STM32 to Arduino Pin Macro Mapping
```cpp
// STM32 Pin → Arduino Pin Macro
PA0  → PA0       PB0  → PB0       PC0  → PC0       PD0  → PD0
PA1  → PA1       PB1  → PB1       PC1  → PC1       PD1  → PD1
PA2  → PA2       PB2  → PB2       PC2  → PC2       PD2  → PD2
...              ...              ...              ...
PA15 → PA15      PB15 → PB15      PC15 → PC15      PD15 → PD15
```

### Usage in Board Configuration
```cpp
// targets/NUCLEO_F411RE.h
namespace BoardConfig {
  // Storage: SPI flash for LittleFS
  static constexpr SPIConfig storage{PC12, PC11, PC10, PD2, 1000000, 8000000};
  //                                 MOSI  MISO  SCLK  CS

  // IMU: Accelerometer/Gyroscope via SPI
  static constexpr SPIConfig imu{PA7, PA6, PA5, PA4, 1000000, 8000000};
  //                             MOSI MISO SCLK CS
}
```

### Practical Usage Example
```cpp
#include "targets/NUCLEO_F411RE.h"

// Works directly with Arduino functions - no conversion needed
SPIClass configuredSPI(
  BoardConfig::storage.mosi_pin,    // PC12
  BoardConfig::storage.miso_pin,    // PC11
  BoardConfig::storage.sclk_pin     // PC10
);

pinMode(BoardConfig::storage.cs_pin, OUTPUT);     // PD2
digitalWrite(BoardConfig::storage.cs_pin, HIGH);  // PD2
```
