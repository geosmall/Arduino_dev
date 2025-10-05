#!/usr/bin/env python3
"""
Betaflight unified target to BoardConfig converter.

Usage: python3 convert.py <config_file> <output_file>
"""

import sys
from pathlib import Path

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / 'src'))

from betaflight_config import BetaflightConfig
from peripheral_pins import PeripheralPinMap
from validator import ConfigValidator
from code_generator import BoardConfigGenerator


def main():
    if len(sys.argv) < 3:
        print("Usage: ./convert.py <config_file> <output_file>")
        print("Example: ./convert.py data/JHEF-JHEF411.config output/NOXE_V3.h")
        sys.exit(1)

    config_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])

    if not config_path.exists():
        print(f"Error: Config file not found: {config_path}")
        sys.exit(1)

    # Load Betaflight config
    print(f"Loading Betaflight config: {config_path}")
    bf_config = BetaflightConfig(config_path)
    print(f"  Board: {bf_config.board_name}")
    print(f"  Manufacturer: {bf_config.manufacturer_id}")
    print(f"  MCU: {bf_config.mcu_type}")

    # Detect Arduino variant path from MCU type
    arduino_root = Path(__file__).parents[2]

    # Map MCU type to variant path
    mcu_to_variant = {
        'STM32F411': 'STM32F4xx/F411C(C-E)(U-Y)',
        'STM32F405': 'STM32F4xx/F405RG',
        'STM32F745': 'STM32F7xx/F74xZ(G-I)',
        'STM32H743': 'STM32H7xx/H743Z(G-I)',
    }

    variant_subpath = mcu_to_variant.get(bf_config.mcu_type)
    if not variant_subpath:
        print(f"Error: Unsupported MCU type: {bf_config.mcu_type}")
        print(f"Supported MCUs: {', '.join(mcu_to_variant.keys())}")
        sys.exit(1)

    pinmap_path = arduino_root / f"Arduino_Core_STM32/variants/{variant_subpath}/PeripheralPins.c"

    if not pinmap_path.exists():
        print(f"Error: PeripheralPins.c not found at: {pinmap_path}")
        sys.exit(1)

    print(f"Loading PeripheralPins.c: {pinmap_path}")
    pinmap = PeripheralPinMap(pinmap_path)

    # Validate configuration
    print("\nValidating configuration...")
    validator = ConfigValidator(bf_config, pinmap)
    if not validator.validate_all():
        print("\n❌ Validation failed:")
        print(validator.get_validation_summary())
        sys.exit(1)

    print("✅ Validation passed")
    print(validator.get_validation_summary())

    # Generate code
    print(f"\nGenerating BoardConfig: {output_path}")
    generator = BoardConfigGenerator(bf_config, validator)

    # Create output directory if needed
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Save generated code
    generator.save(output_path)

    print(f"✅ Successfully generated: {output_path}")


if __name__ == '__main__':
    main()
