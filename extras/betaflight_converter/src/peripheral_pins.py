"""
Parser for STM32 Arduino Core PeripheralPins.c files.

Extracts pin-to-peripheral mappings for validation.
"""

import re
from dataclasses import dataclass
from typing import Dict, List, Tuple, Optional
from pathlib import Path


class Patterns:
    """Compiled regex patterns for PeripheralPins.c parsing."""
    # Section patterns - find PinMap_XXX arrays
    TIM_SECTION = re.compile(
        r'WEAK const PinMap PinMap_TIM\[\] = \{(.+?)\{NC,',
        re.DOTALL
    )

    # Entry patterns - parse individual pin entries
    TIMER_ENTRY = re.compile(
        r'\{(\w+),\s+(\w+),\s+STM_PIN_DATA_EXT\([^,]+,[^,]+,\s+GPIO_AF(\d+)_\w+,\s+(\d+),\s+(\d+)\)\}'
    )
    PERIPHERAL_ENTRY = re.compile(
        r'\{(\w+),\s+(\w+),\s+STM_PIN_DATA'
    )

    @staticmethod
    def get_section_pattern(map_name: str) -> re.Pattern:
        """Build regex pattern for finding a specific PinMap array."""
        return re.compile(
            rf'WEAK const PinMap {map_name}\[\] = \{{(.+?)\{{NC,',
            re.DOTALL
        )


@dataclass
class TimerPin:
    """Timer pin configuration."""
    pin: str        # e.g., "PB_4"
    timer: str      # e.g., "TIM3"
    af: int         # Alternate function number (1-15)
    channel: int    # Timer channel (1-4)
    is_complementary: bool  # True for CHxN pins


@dataclass
class SPIPin:
    """SPI pin configuration."""
    pin: str        # e.g., "PA_7"
    bus: str        # e.g., "SPI1"
    signal: str     # "MOSI", "MISO", or "SCLK"


@dataclass
class I2CPin:
    """I2C pin configuration."""
    pin: str        # e.g., "PB_8"
    bus: str        # e.g., "I2C1"
    signal: str     # "SCL" or "SDA"


@dataclass
class UARTPin:
    """UART pin configuration."""
    pin: str        # e.g., "PB_6"
    uart: str       # e.g., "USART1"
    signal: str     # "TX" or "RX"


class PeripheralPinMap:
    """Parser for PeripheralPins.c files."""

    def __init__(self, filepath: Path):
        """Initialize parser with path to PeripheralPins.c file."""
        self.filepath = filepath
        self.timer_pins: List[TimerPin] = []
        self.spi_pins: List[SPIPin] = []
        self.i2c_pins: List[I2CPin] = []
        self.uart_pins: List[UARTPin] = []

        if filepath.exists():
            self._parse()

    def _parse(self):
        """Parse PeripheralPins.c file."""
        with open(self.filepath, 'r') as f:
            content = f.read()

        self._parse_timers(content)
        self._parse_spi(content)
        self._parse_i2c(content)
        self._parse_uart(content)

    def _parse_timers(self, content: str):
        """Parse PinMap_TIM array."""
        # Find PinMap_TIM array
        tim_section = Patterns.TIM_SECTION.search(content)
        if not tim_section:
            return

        # Parse each line: {PB_4, TIM3, STM_PIN_DATA_EXT(..., GPIO_AF2_TIM3, 1, 0)}, // TIM3_CH1
        for match in Patterns.TIMER_ENTRY.finditer(tim_section.group(1)):
            pin = match.group(1)
            timer = match.group(2)
            af = int(match.group(3))
            channel = int(match.group(4))
            is_complementary = int(match.group(5)) == 1

            self.timer_pins.append(TimerPin(
                pin=pin,
                timer=timer,
                af=af,
                channel=channel,
                is_complementary=is_complementary
            ))

    def _parse_spi(self, content: str):
        """Parse PinMap_SPI_* arrays."""
        signal_types = {
            'PinMap_SPI_MOSI': 'MOSI',
            'PinMap_SPI_MISO': 'MISO',
            'PinMap_SPI_SCLK': 'SCLK'
        }

        for map_name, signal in signal_types.items():
            section = Patterns.get_section_pattern(map_name).search(content)
            if not section:
                continue

            # Parse: {PA_7, SPI1, STM_PIN_DATA(...)}
            for match in Patterns.PERIPHERAL_ENTRY.finditer(section.group(1)):
                pin = match.group(1)
                bus = match.group(2)

                self.spi_pins.append(SPIPin(
                    pin=pin,
                    bus=bus,
                    signal=signal
                ))

    def _parse_i2c(self, content: str):
        """Parse PinMap_I2C_* arrays."""
        signal_types = {
            'PinMap_I2C_SDA': 'SDA',
            'PinMap_I2C_SCL': 'SCL'
        }

        for map_name, signal in signal_types.items():
            section = Patterns.get_section_pattern(map_name).search(content)
            if not section:
                continue

            # Parse: {PB_9, I2C1, STM_PIN_DATA(...)}
            for match in Patterns.PERIPHERAL_ENTRY.finditer(section.group(1)):
                pin = match.group(1)
                bus = match.group(2)

                self.i2c_pins.append(I2CPin(
                    pin=pin,
                    bus=bus,
                    signal=signal
                ))

    def _parse_uart(self, content: str):
        """Parse PinMap_UART_* arrays."""
        signal_types = {
            'PinMap_UART_TX': 'TX',
            'PinMap_UART_RX': 'RX'
        }

        for map_name, signal in signal_types.items():
            section = Patterns.get_section_pattern(map_name).search(content)
            if not section:
                continue

            # Parse: {PB_6, USART1, STM_PIN_DATA(...)}
            for match in Patterns.PERIPHERAL_ENTRY.finditer(section.group(1)):
                pin = match.group(1)
                uart = match.group(2)

                self.uart_pins.append(UARTPin(
                    pin=pin,
                    uart=uart,
                    signal=signal
                ))

    def validate_timer(self, pin: str, timer: str, af: int) -> Optional[int]:
        """
        Validate timer assignment and return channel number.

        Args:
            pin: Pin name (e.g., "PB_4")
            timer: Timer name (e.g., "TIM3")
            af: Alternate function number

        Returns:
            Timer channel number if valid, None otherwise
        """
        # Try exact pin match first
        for tp in self.timer_pins:
            if tp.pin == pin and tp.timer == timer and tp.af == af:
                return tp.channel

        # Try ALT variants (PB_0_ALT1, etc.)
        pin_base = pin.split('_')[0] + '_' + pin.split('_')[1]  # "PB_0" -> "PB_0"
        for tp in self.timer_pins:
            tp_base = tp.pin.split('_ALT')[0]  # "PB_0_ALT1" -> "PB_0"
            if tp_base == pin_base and tp.timer == timer and tp.af == af:
                return tp.channel

        return None

    def validate_spi_bus(self, pins: Dict[str, str], bus: str) -> bool:
        """
        Validate SPI bus pin assignments.

        Args:
            pins: Dict with keys 'MOSI', 'MISO', 'SCLK' and pin names as values
            bus: Expected SPI bus (e.g., "SPI1")

        Returns:
            True if all pins belong to specified bus, False otherwise
        """
        for signal, pin in pins.items():
            found = False
            for sp in self.spi_pins:
                if sp.pin == pin and sp.bus == bus and sp.signal == signal:
                    found = True
                    break
            if not found:
                return False
        return True

    def validate_i2c_bus(self, pins: Dict[str, str], bus: str) -> bool:
        """
        Validate I2C bus pin assignments.

        Args:
            pins: Dict with keys 'SCL', 'SDA' and pin names as values
            bus: Expected I2C bus (e.g., "I2C1")

        Returns:
            True if both pins belong to specified bus, False otherwise
        """
        for signal, pin in pins.items():
            found = False
            for ip in self.i2c_pins:
                if ip.pin == pin and ip.bus == bus and ip.signal == signal:
                    found = True
                    break
            if not found:
                return False
        return True

    def validate_uart(self, pins: Dict[str, str], uart: str) -> bool:
        """
        Validate UART pin assignments.

        Args:
            pins: Dict with keys 'TX', 'RX' and pin names as values
            uart: Expected UART (e.g., "USART1")

        Returns:
            True if both pins belong to specified UART, False otherwise
        """
        for signal, pin in pins.items():
            found = False
            for up in self.uart_pins:
                if up.pin == pin and up.uart == uart and up.signal == signal:
                    found = True
                    break
            if not found:
                return False
        return True

    def get_spi_bus(self, pin: str, signal: str) -> Optional[str]:
        """Get SPI bus for a given pin and signal type."""
        for sp in self.spi_pins:
            if sp.pin == pin and sp.signal == signal:
                return sp.bus
        return None

    def get_i2c_bus(self, pin: str, signal: str) -> Optional[str]:
        """Get I2C bus for a given pin and signal type."""
        for ip in self.i2c_pins:
            if ip.pin == pin and ip.signal == signal:
                return ip.bus
        return None

    def get_uart(self, pin: str, signal: str) -> Optional[str]:
        """Get UART for a given pin and signal type."""
        for up in self.uart_pins:
            if up.pin == pin and up.signal == signal:
                return up.uart
        return None
