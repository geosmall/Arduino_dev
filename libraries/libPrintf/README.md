# libPrintf - Embedded Printf Library for Arduino STM32

A lightweight, embedded-friendly printf implementation that eliminates nanofp confusion and provides reliable float formatting for STM32 Arduino projects.

## Overview

libPrintf is based on [eyalroz/printf v6.2.0](https://github.com/eyalroz/printf) and provides a complete replacement for the standard printf family of functions. It's specifically designed to solve the common nanofp/nano library confusion in STM32 Arduino development while providing significant binary size reductions.

## Key Benefits

- ✅ **Eliminates nanofp confusion**: No more complex FQBN configurations or rtlib settings
- ✅ **Reduces binary size**: ~20% reduction compared to nanofp (typically 8KB+ savings)
- ✅ **Reliable float formatting**: Works without build configuration complexity
- ✅ **Factory code compatible**: Seamless integration with existing printf/fprintf calls
- ✅ **Thread-safe**: Suitable for embedded real-time applications
- ✅ **Complete printf family**: Supports printf, sprintf, fprintf, snprintf, vprintf, etc.

## Quick Start

### Installation

Copy the `libPrintf` directory to your Arduino `libraries/` folder, or place it in your project's libraries directory.

### Usage

```cpp
#include <libPrintf.h>

void setup() {
  Serial.begin(115200);  // Required: libPrintf routes to Serial by default

  // All standard printf functions now work with float support
  printf("Pi = %.6f\n", 3.14159265);
  printf("Integer: %d, String: %s\n", 42, "Hello");

  // Works with sprintf too
  char buffer[64];
  sprintf(buffer, "Formatted: %.2f%%", 85.75);
  printf("Buffer: %s\n", buffer);
}
```

### RTT Integration

For HIL testing with RTT, implement a custom `putchar_()` function:

```cpp
#include <libPrintf.h>

extern "C" void putchar_(char c) {
#ifdef USE_RTT
  char buf[2] = {c, '\0'};
  SEGGER_RTT_WriteString(0, buf);
#else
  Serial.print(c);
#endif
}
```

## Technical Details

### Function Aliasing

libPrintf uses `PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_SOFT` to automatically replace standard printf functions when the library is included. This means:

- `printf()` → `printf_()`
- `sprintf()` → `sprintf_()`
- `fprintf()` → `fprintf_()`
- `snprintf()` → `snprintf_()`
- And all other printf family functions...

This aliasing is completely transparent - your existing code works unchanged once you include `<libPrintf.h>`.

### Binary Size Comparison

| Configuration | Binary Size | Savings |
|---------------|-------------|---------|
| Standard newlib | ~42KB | - |
| newlib nano + nanofp | ~42KB | 0% |
| newlib nano + libPrintf | ~34KB | **17%** |

### Supported Format Specifiers

libPrintf supports all standard printf format specifiers:

- **Integers**: `%d`, `%i`, `%o`, `%x`, `%X`, `%u`
- **Floats**: `%f`, `%F`, `%e`, `%E`, `%g`, `%G`
- **Characters**: `%c`, `%s`
- **Pointers**: `%p`
- **Width/Precision**: `%10s`, `%.2f`, `%*.*f`
- **Flags**: `%-`, `%+`, `%#`, `%0`, `% ` (space)

## Examples

### Basic Usage

See `examples/BasicUsage/BasicUsage.ino` for a complete demonstration of libPrintf functionality.

### Real-World Integration

The ICM42688P self-test example demonstrates libPrintf integration with factory code:

```cpp
#include <libPrintf.h>
#include "factory_driver.h"  // Uses fprintf(stderr, ...)

void setup() {
  // Factory code automatically uses libPrintf
  run_factory_selftest();  // fprintf calls work seamlessly
}
```

## Build Requirements

- **Arduino IDE**: 1.8.x or Arduino CLI
- **STM32 Core**: 2.7.1 or newer
- **FQBN**: Standard configuration (no rtlib needed!)
  ```
  STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE
  ```

## Version Information

- **libPrintf**: v1.0.0 (Arduino wrapper library)
- **Based on**: eyalroz/printf v6.2.0 (embedded implementation)
- **License**: MIT (see eyalroz/printf repository)

## Integration Notes

### Automatic Integration

Simply include `<libPrintf.h>` and all printf functions are automatically replaced:

```cpp
#include <libPrintf.h>
// No additional configuration needed!
```

### Manual Configuration

If you need to disable automatic aliasing:

```cpp
// Include the base printf.h directly
#include "printf.h"

// Use explicit function names
printf_("Manual printf: %.2f\n", 3.14);
```

## Troubleshooting

### Common Issues

1. **"undefined reference to printf"**
   - Make sure `#include <libPrintf.h>` is before any printf usage
   - Verify the library is in your libraries path

2. **Float values showing as "0.000000"**
   - This suggests the old newlib nano printf is still being used
   - Check that libPrintf.h is included in all files using printf

3. **Build size larger than expected**
   - Verify you're not including both libPrintf and nanofp
   - Check FQBN doesn't include `:rtlib=nanofp`

### Verification

To verify libPrintf is working:

```cpp
#include <libPrintf.h>

void setup() {
  printf("Test float: %.6f\n", 3.14159265);
  // Should show: Test float: 3.141593
  // If it shows: Test float: 0.000000, libPrintf is not active
}
```

## Contributing

This library is a wrapper around eyalroz/printf. For core printf functionality improvements, contribute to the upstream repository. For Arduino integration improvements, contribute to this wrapper.

## License

MIT License (inherited from eyalroz/printf)