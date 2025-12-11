# dRehmFlight Business Plan & IP Protection Strategy

## Executive Summary

This document captures the business model analysis and IP protection strategy for commercializing dRehmFlight flight controller firmware. The chosen approach follows the **Teensy/PJRC model**: protected bootloader as the revenue moat, with open/free mixer ecosystem to drive community engagement.

**Key Decision**: Hardware sales as primary revenue, bootloader protection as the moat.

---

## Table of Contents

1. [Business Model Evolution](#business-model-evolution)
2. [IP Protection Analysis](#ip-protection-analysis)
3. [Target Market](#target-market)
4. [Bootloader Strategy](#bootloader-strategy)
5. [Architecture Overview](#architecture-overview)
6. [Implementation Roadmap](#implementation-roadmap)
7. [Risk Analysis](#risk-analysis)

---

## Business Model Evolution

### Initial Approach (Rejected)

**Precompiled Libraries with Hardware Binding**
- Distribute mixer libraries as `.a` files (PrecompLib pattern)
- Bind to specific hardware via STM32 UID
- Charge per-mixer or subscription

**Why Rejected:**
- Precompiled `.a` files provide no real protection - anyone can copy them
- Hardware binding adds friction for legitimate users
- Per-mixer charging creates adversarial relationship with community
- Support burden for licensing/activation issues

### Chosen Approach: Inverted Model

**Protected Bootloader + Open Mixer Ecosystem**

| Component | Protection | Distribution | Revenue |
|-----------|------------|--------------|---------|
| **Bootloader** | RDP Level 1, closed source | Ships on hardware only | Hardware sales |
| **Mixers** | Ed25519 signed, open source | Community-created, free | Community growth |
| **Signing Tool** | Open source | Free download | Ecosystem enabler |
| **Hardware** | Physical product | Direct sales | **Primary revenue** |

**Rationale:**
- Follows proven Teensy/PJRC model (15+ years success)
- Community creates value through free mixers
- Low friction encourages adoption
- Hardware is the natural monetization point
- Bootloader is the moat - can't run on clones without it

---

## IP Protection Analysis

### STM32 Hardware Security Features by Family

| Family | Hardware Crypto | UID | RDP | OTP | Best For |
|--------|----------------|-----|-----|-----|----------|
| **F4** | None | 96-bit | L0/L1/L2 | No | Cost-sensitive |
| **G4** | None | 96-bit | L0/L1/L2 | Yes | Mid-range |
| **H7** | AES, HASH, RNG | 96-bit | L0/L1/L2 | Yes | Premium |
| **H7RS** | SAES, PKA, DHUK | 96-bit | L0/L1/L2 | Yes | High security |

### Protection Mechanisms

**RDP (Read-out Protection) Levels:**
- **Level 0**: Full debug access (development only)
- **Level 1**: Debug blocked, custom bootloader accessible, reversible (erases flash)
- **Level 2**: Permanently locked, no debug, no bootloader - **IRREVERSIBLE**

**Recommendation**: Use **RDP Level 1** for production
- Protects bootloader code from extraction
- Allows firmware updates via custom bootloader
- Reversible for RMA/warranty (with flash erase)

**UID Binding (Optional Premium Features):**
```c
// Read 96-bit unique device ID
uint32_t uid[3];
uid[0] = HAL_GetUIDw0();
uid[1] = HAL_GetUIDw1();
uid[2] = HAL_GetUIDw2();

// Hash for license verification
uint8_t device_hash[32];
sha256(uid, 12, device_hash);
```

### Comparison with Industry Approaches

| Company | Model | Protection Method | Revenue Source |
|---------|-------|-------------------|----------------|
| **Teensy/PJRC** | Bootloader chip | Closed-source bootloader IC | Hardware sales |
| **DJI** | Full stack | Hardware + encrypted firmware | Hardware + services |
| **Betaflight** | Open source | None (GPL) | Donations |
| **ArduPilot** | Open source | None (GPL) | Donations + services |

**Key Insight**: Successful commercial embedded products protect the bootloader/loader, not the application code.

---

## Target Market

### Primary: Hobbyist FPV Pilots

**Characteristics:**
- Price-sensitive but willing to pay for quality
- Value ease of use over advanced features
- Active in online communities (forums, Discord, YouTube)
- Prefer plug-and-play over configuration complexity
- Trust peer recommendations

**Pain Points with Current Solutions:**
- Betaflight/INAV require complex tuning
- PID configuration is intimidating
- Firmware updates require special tools (DFU drivers)
- Airframe-specific setups are trial-and-error

**Value Proposition:**
- Pre-configured mixers for common airframes
- Drag-drop firmware/mixer updates (no drivers)
- Community-validated configurations
- "It just works" experience

### Secondary: Small-Scale Manufacturers

**Characteristics:**
- Building RTF (ready-to-fly) drones
- Need reliable, supported firmware
- Want differentiation from commodity products
- May need custom mixer development

**Value Proposition:**
- Licensed bootloader for their hardware
- Custom mixer development services
- Technical support agreement
- White-label options

---

## Bootloader Strategy

### Bootloader Options Analysis

| Option | Drag-Drop | License | Size | SPI Flash | Closed-Source |
|--------|-----------|---------|------|-----------|---------------|
| **STM32 DFU** | No (needs tool) | ST Proprietary | 8KB (ROM) | No | Yes |
| **OpenBLT** | No (needs tool) | GPL (commercial $) | 4-20KB | Yes | Paid license |
| **TinyUF2** | Yes (UF2 format) | MIT | 12-20KB | No native | Yes |
| **Custom USB MSC** | Yes (.bin files) | Custom | 12-20KB | Yes | Yes |
| **Betaflight** | No (DFU only) | GPL | ~8KB | No | No |

### Recommendation: Custom USB MSC Bootloader

**Why Custom USB MSC:**

1. **True drag-drop with .bin files** - No format conversion needed
2. **Direct SPI Flash integration** - Leverages existing LittleFS code
3. **Full closed-source capability** - No GPL contamination
4. **Zero driver installation** - Works on Windows/Mac/Linux
5. **Small footprint** - 12-20KB achievable
6. **Complete control** - Custom features, signing, recovery

**Reference Implementations:**
- ST Hotspot STM32G0 MSC - Official ST example
- sfyip STM32F103 MSD Bootloader - Proven FAT32 emulation
- Blue Pill USB Bootloader - Composite USB device

**Alternative: TinyUF2**
- Faster development (~2 weeks vs 4-6 weeks)
- MIT license allows closed-source
- Requires UF2 format conversion (minor friction)
- Well-maintained by Adafruit

### Flight Controller Bootloader Comparison

| Firmware | Bootloader | Update Method | User Experience |
|----------|------------|---------------|-----------------|
| **Betaflight** | STM32 DFU | Configurator + drivers | Medium friction |
| **INAV** | STM32 DFU | Configurator + drivers | Medium friction |
| **ArduPilot** | Custom | Mission Planner or SD card | Low-medium friction |
| **RP2040** | UF2 | Drag-drop | Very low friction |
| **Proposed** | USB MSC | Drag-drop .bin | Very low friction |

---

## Architecture Overview

### System Components

```
┌─────────────────────────────────────────────────────────────────┐
│                        USER DEVICE                               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐ │
│  │ Mixer Files │  │ Firmware    │  │ Signing Tool            │ │
│  │ (.mix)      │  │ (.bin)      │  │ (generates signatures)  │ │
│  └──────┬──────┘  └──────┬──────┘  └─────────────────────────┘ │
│         │                │                                       │
│         └────────┬───────┘                                       │
│                  │ Drag-Drop                                     │
│                  ▼                                               │
│         ┌────────────────┐                                       │
│         │ USB Drive View │                                       │
│         │ (FAT32)        │                                       │
│         └────────┬───────┘                                       │
└──────────────────┼──────────────────────────────────────────────┘
                   │ USB
                   ▼
┌─────────────────────────────────────────────────────────────────┐
│                    FLIGHT CONTROLLER                             │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                 PROTECTED BOOTLOADER                      │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │  │
│  │  │ USB MSC     │  │ Ed25519     │  │ Flash Manager   │  │  │
│  │  │ Stack       │  │ Verify      │  │ (Int + SPI)     │  │  │
│  │  └─────────────┘  └─────────────┘  └─────────────────┘  │  │
│  │                                                          │  │
│  │  RDP Level 1 Protected | Closed Source | ~20KB          │  │
│  └──────────────────────────────────────────────────────────┘  │
│                              │                                   │
│                              ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                   APPLICATION FIRMWARE                    │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │  │
│  │  │ dRehmFlight │  │ Mixer       │  │ Sensor/Motor    │  │  │
│  │  │ Core        │  │ Loader      │  │ Drivers         │  │  │
│  │  └─────────────┘  └─────────────┘  └─────────────────┘  │  │
│  └──────────────────────────────────────────────────────────┘  │
│                              │                                   │
│                              ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │                     SPI FLASH                             │  │
│  │  ┌─────────────────────────────────────────────────────┐ │  │
│  │  │ LittleFS Filesystem                                 │ │  │
│  │  │  /mixers/quad_x.mix     (signed mixer binary)       │ │  │
│  │  │  /mixers/hex_y6.mix     (signed mixer binary)       │ │  │
│  │  │  /config/params.ini     (user configuration)        │ │  │
│  │  │  /logs/flight_001.bin   (flight logs)               │ │  │
│  │  └─────────────────────────────────────────────────────┘ │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### Memory Layout (STM32F411 Example)

```
Flash (512KB):
┌─────────────────────────────────────┐ 0x08000000
│ Bootloader (64KB)                   │
│ - USB MSC Stack                     │
│ - FAT32 Emulation                   │
│ - Ed25519 Signature Verification    │
│ - Flash Programming                 │
│ - Recovery Mode                     │
├─────────────────────────────────────┤ 0x08010000
│ Application Firmware (448KB)        │
│ - dRehmFlight Core                  │
│ - Mixer Loader                      │
│ - Sensor Drivers                    │
│ - Motor Control                     │
└─────────────────────────────────────┘ 0x08080000

External SPI Flash (8-16MB):
┌─────────────────────────────────────┐
│ LittleFS Filesystem                 │
│ - /mixers/*.mix (signed binaries)   │
│ - /config/*.ini (user settings)     │
│ - /logs/*.bin (flight data)         │
└─────────────────────────────────────┘
```

### Bootloader Flow

```c
void bootloader_main(void) {
    // 1. Initialize minimal hardware
    init_clocks();
    init_gpio();

    // 2. Check for forced bootloader mode
    if (button_held() || no_valid_app()) {
        // Enter USB Mass Storage mode
        usb_msc_mode();  // Blocks until file written or timeout
    }

    // 3. Validate application
    if (!verify_app_signature()) {
        error_blink();
        usb_msc_mode();  // Recovery
    }

    // 4. Jump to application
    jump_to_app(APP_START_ADDR);
}

void jump_to_app(uint32_t addr) {
    __disable_irq();
    SysTick->CTRL = 0;

    // Relocate vector table
    SCB->VTOR = addr;

    // Set stack pointer and jump
    uint32_t *vectors = (uint32_t *)addr;
    __set_MSP(vectors[0]);

    void (*app_reset)(void) = (void (*)(void))vectors[1];
    app_reset();
}
```

### Mixer File Format

```c
// Mixer header (128 bytes)
typedef struct __attribute__((packed)) {
    uint32_t magic;           // 0x4D495852 ("MIXR")
    uint32_t version;         // Format version
    uint32_t mixer_type;      // QUAD_X, HEX_Y6, etc.
    uint32_t code_size;       // Size of mixer code
    uint32_t data_size;       // Size of mixer data
    uint8_t  name[32];        // Human-readable name
    uint8_t  author[32];      // Author identifier
    uint8_t  signature[64];   // Ed25519 signature
    uint8_t  reserved[16];    // Future use
} MixerHeader;

// Signature covers: magic through reserved (header minus signature)
// Plus: all code and data bytes
```

### Signing Workflow

```
┌─────────────────────────────────────────────────────────────────┐
│                     MIXER DEVELOPMENT                            │
│                                                                  │
│  Developer creates mixer:                                        │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────────┐ │
│  │ Source Code │ -> │ Compile     │ -> │ Unsigned .mix       │ │
│  │ (C/C++)     │    │ (arm-gcc)   │    │ (header + binary)   │ │
│  └─────────────┘    └─────────────┘    └──────────┬──────────┘ │
│                                                    │             │
│                                                    ▼             │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │                    SIGNING TOOL (Free)                      ││
│  │  $ mixersign --key private.pem --input quad_x.mix           ││
│  │                                                             ││
│  │  1. Hash mixer header + code + data                         ││
│  │  2. Sign hash with Ed25519 private key                      ││
│  │  3. Embed signature in header                               ││
│  │  4. Output signed .mix file                                 ││
│  └─────────────────────────────────────────────────────────────┘│
│                                                    │             │
│                                                    ▼             │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │  Signed .mix file ready for distribution                    ││
│  │  - Can be shared freely                                     ││
│  │  - Bootloader verifies signature before loading             ││
│  │  - Invalid signatures rejected                              ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
```

---

## Implementation Roadmap

### Phase 1: Bootloader Development (4-6 weeks)

**Week 1-2: USB MSC Foundation**
- USB Device stack integration (STM32Cube HAL)
- FAT32 filesystem emulation (minimal implementation)
- Basic read/write to virtual drive
- Test on NUCLEO_F411RE

**Week 3-4: Flash Programming**
- Internal flash erase/write routines
- SPI flash integration (existing LittleFS)
- File validation and programming logic
- Recovery mode implementation

**Week 5-6: Security Integration**
- Ed25519 signature verification (monocypher or similar)
- RDP Level 1 configuration
- Bootloader protection (write-protect sectors)
- Production testing

### Phase 2: Mixer System (2-3 weeks)

**Mixer Format Definition**
- Header structure
- Code/data layout
- Signature scheme

**Mixer Loader**
- Runtime loading from SPI flash
- Symbol resolution
- Memory management

**Signing Tool**
- Command-line tool
- Key management
- Batch signing support

### Phase 3: Community Infrastructure (2-3 weeks)

**Mixer Repository**
- GitHub-based distribution
- CI/CD for validation
- Documentation templates

**Development SDK**
- Mixer API definition
- Example mixers
- Build system integration

### Phase 4: Hardware Production (Parallel)

**Initial Run**
- Partner with existing FC manufacturer, or
- Custom PCB design (based on NOXE V3 or similar)
- Pre-flash bootloader
- Quality control

---

## Risk Analysis

### Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| USB MSC compatibility issues | Medium | High | Test on multiple OS versions early |
| Bootloader too large for F4 | Low | Medium | Optimize aggressively, use F7/H7 if needed |
| Ed25519 performance on F4 | Low | Low | monocypher is fast, verify benchmarks |
| SPI Flash reliability | Low | Medium | Use proven LittleFS, wear leveling |

### Business Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Clone hardware flooding market | Medium | High | Bootloader binding, community trust |
| Community doesn't engage | Medium | High | Seed with quality mixers, good docs |
| Betaflight/INAV add drag-drop | Low | Medium | First-mover advantage, better UX |
| Supply chain issues | Medium | Medium | Multiple MCU family support |

### Legal Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| GPL contamination in bootloader | Medium | High | Audit all dependencies, use MIT/BSD only |
| Patent claims on USB MSC | Low | Medium | USB-IF membership, standard compliance |
| Trademark issues | Low | Low | Clear branding, avoid existing marks |

---

## Financial Projections (Illustrative)

### Hardware Revenue Model

**Assumptions:**
- Flight controller unit cost: $25
- Selling price: $60
- Gross margin: ~58%

| Year | Units | Revenue | Gross Profit |
|------|-------|---------|--------------|
| 1 | 500 | $30,000 | $17,500 |
| 2 | 2,000 | $120,000 | $70,000 |
| 3 | 5,000 | $300,000 | $175,000 |

### Optional: Support/Services Revenue

| Service | Price | Estimated Annual |
|---------|-------|------------------|
| Custom mixer development | $500-2000 | $10,000 |
| OEM licensing | $5-10/unit | $25,000 |
| Technical support tier | $99/year | $5,000 |

---

## Appendix A: STM32 UID Binding Code

```c
#include "stm32f4xx_hal.h"
#include <string.h>

// Get unique device ID
void get_device_uid(uint8_t uid[12]) {
    uint32_t *uid_base = (uint32_t *)UID_BASE;
    memcpy(uid, uid_base, 12);
}

// Simple hash for device binding (use proper crypto in production)
uint32_t hash_uid(const uint8_t uid[12]) {
    uint32_t hash = 0x811c9dc5;  // FNV-1a offset basis
    for (int i = 0; i < 12; i++) {
        hash ^= uid[i];
        hash *= 0x01000193;  // FNV-1a prime
    }
    return hash;
}

// Verify device is authorized
bool verify_device(uint32_t expected_hash) {
    uint8_t uid[12];
    get_device_uid(uid);
    return hash_uid(uid) == expected_hash;
}
```

## Appendix B: Ed25519 Signature Verification

```c
#include "monocypher.h"

// Public key embedded in bootloader (32 bytes)
static const uint8_t public_key[32] = { /* ... */ };

// Verify mixer signature
bool verify_mixer_signature(const MixerHeader *header,
                            const uint8_t *code,
                            const uint8_t *data) {
    // Build message to verify (header without signature + code + data)
    size_t msg_len = offsetof(MixerHeader, signature) +
                     header->code_size +
                     header->data_size;

    uint8_t *message = malloc(msg_len);
    if (!message) return false;

    // Copy header (excluding signature)
    memcpy(message, header, offsetof(MixerHeader, signature));
    size_t offset = offsetof(MixerHeader, signature);

    // Copy code and data
    memcpy(message + offset, code, header->code_size);
    offset += header->code_size;
    memcpy(message + offset, data, header->data_size);

    // Verify Ed25519 signature
    int result = crypto_ed25519_check(
        header->signature,
        public_key,
        message,
        msg_len
    );

    free(message);
    return result == 0;
}
```

## Appendix C: USB MSC Bootloader References

### Official ST Examples
- [STM32G0 USB MSC Bootloader](https://github.com/stm32-hotspot/BOOTLOADER_USB_MSC_STM32G0)
- STM32Cube USB Device Library (USBD_MSC class)

### Community Projects
- [STM32F103 MSD Bootloader](https://github.com/sfyip/STM32F103_MSD_BOOTLOADER)
- [Blue Pill USB Bootloader](https://lupyuen.github.io/articles/stm32-blue-pill-usb-bootloader-how-i-fixed-the-usb-storage-serial-dfu-and-webusb-interfaces/)
- [Kevin Cuzner's USB Bootloader Guide](https://kevincuzner.com/2018/06/28/building-a-usb-bootloader-for-an-stm32/)

### Libraries
- [TinyUSB](https://github.com/hathach/tinyusb) - MIT license, excellent MSC support
- [TinyUF2](https://github.com/adafruit/tinyuf2) - UF2 bootloader based on TinyUSB
- [monocypher](https://monocypher.org/) - Small crypto library, public domain

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2024-01 | Claude Code Session | Initial creation from planning discussion |

---

## Appendix D: PrecompLib Relationship

### Why PrecompLib Exists

PrecompLib was created to validate the `precompiled=true` Arduino library feature for potential proprietary mixer distribution. During business model analysis, the strategy evolved:

| Original Plan | Final Strategy |
|---------------|----------------|
| Mixers as precompiled `.a` files | Mixers as signed `.mix` binaries |
| Protection via binary-only distribution | Protection via bootloader + signatures |
| Linked at compile time | Loaded at runtime from SPI Flash |
| Revenue from mixer licensing | Revenue from hardware sales |

### PrecompLib's Current Role

PrecompLib remains valuable as an **educational reference**:

1. **Technical Demonstration**: Shows Arduino `precompiled=true` workflow
2. **Multi-Architecture Validation**: Proves cortex-m4/m7 binary distribution works
3. **Future Reference**: Pattern for any closed-source Arduino libraries
4. **Bootloader Distribution**: Could distribute bootloader via same pattern (header + binary)

### Not Part of Core Business Model

PrecompLib does **not** provide meaningful IP protection for the mixer ecosystem because:
- Precompiled `.a` files can be freely copied (no hardware binding)
- Compile-time linking doesn't prevent redistribution
- No signature verification or runtime protection

The **protected bootloader** approach solves these issues by:
- Hardware binding via STM32 UID
- Ed25519 signature verification
- RDP Level 1 protecting bootloader code
- Runtime validation before execution

---

## Related Documents

- [PRECOMPILED_LIB.md](../libraries/PrecompLib/PRECOMPILED_LIB.md) - Precompiled library implementation details (educational reference)
- [PrecompLib README](../libraries/PrecompLib/README.md) - PrecompLib usage documentation
- [TIMERS.md](TIMERS.md) - Timer/PWM documentation for motor control
- [SERIAL.md](SERIAL.md) - Serial/UART documentation for RC receivers
