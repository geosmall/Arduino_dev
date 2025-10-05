"""
Tests for PeripheralPins.c parser.
"""

import unittest
from pathlib import Path
import sys

# Add src to path
sys.path.insert(0, str(Path(__file__).parent.parent / 'src'))

from peripheral_pins import PeripheralPinMap, TimerPin, SPIPin, I2CPin, UARTPin


class TestPeripheralPinMap(unittest.TestCase):
    """Test PeripheralPins.c parser with real STM32F411 data."""

    @classmethod
    def setUpClass(cls):
        """Load real F411CE PeripheralPins.c file."""
        # Path to Arduino Core STM32 variant
        arduino_root = Path(__file__).parents[3]
        variant_path = arduino_root / "Arduino_Core_STM32/variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c"

        if not variant_path.exists():
            raise FileNotFoundError(f"PeripheralPins.c not found at {variant_path}")

        cls.pinmap = PeripheralPinMap(variant_path)

    def test_timer_parsing(self):
        """Test timer pin parsing."""
        # Should have parsed multiple timer pins
        self.assertGreater(len(self.pinmap.timer_pins), 0)

        # Verify known entry: PB4 = TIM3_CH1 on AF2
        found = False
        for tp in self.pinmap.timer_pins:
            if (tp.pin == "PB_4" and tp.timer == "TIM3" and
                tp.af == 2 and tp.channel == 1 and not tp.is_complementary):
                found = True
                break
        self.assertTrue(found, "PB_4 TIM3_CH1 AF2 not found")

    def test_spi_parsing(self):
        """Test SPI pin parsing."""
        # Should have parsed SPI pins
        self.assertGreater(len(self.pinmap.spi_pins), 0)

        # Verify known entry: PA7 = SPI1 MOSI
        found = False
        for sp in self.pinmap.spi_pins:
            if sp.pin == "PA_7" and sp.bus == "SPI1" and sp.signal == "MOSI":
                found = True
                break
        self.assertTrue(found, "PA_7 SPI1 MOSI not found")

    def test_i2c_parsing(self):
        """Test I2C pin parsing."""
        # Should have parsed I2C pins
        self.assertGreater(len(self.pinmap.i2c_pins), 0)

        # Verify known entry: PB8 = I2C1 SCL
        found = False
        for ip in self.pinmap.i2c_pins:
            if ip.pin == "PB_8" and ip.bus == "I2C1" and ip.signal == "SCL":
                found = True
                break
        self.assertTrue(found, "PB_8 I2C1 SCL not found")

    def test_uart_parsing(self):
        """Test UART pin parsing."""
        # Should have parsed UART pins
        self.assertGreater(len(self.pinmap.uart_pins), 0)

        # Verify known entry: PB6 = USART1 TX
        found = False
        for up in self.pinmap.uart_pins:
            if up.pin == "PB_6" and up.uart == "USART1" and up.signal == "TX":
                found = True
                break
        self.assertTrue(found, "PB_6 USART1 TX not found")

    def test_validate_timer(self):
        """Test timer validation."""
        # Valid: PB4 = TIM3 on AF2
        channel = self.pinmap.validate_timer("PB_4", "TIM3", 2)
        self.assertEqual(channel, 1, "PB_4 TIM3 AF2 should return channel 1")

        # Invalid: Wrong AF
        channel = self.pinmap.validate_timer("PB_4", "TIM3", 99)
        self.assertIsNone(channel, "Invalid AF should return None")

        # Invalid: Wrong pin
        channel = self.pinmap.validate_timer("PX_99", "TIM3", 2)
        self.assertIsNone(channel, "Invalid pin should return None")

    def test_validate_spi_bus(self):
        """Test SPI bus validation."""
        # Valid: SPI1 = PA7 (MOSI), PA6 (MISO), PA5 (SCLK)
        pins = {'MOSI': 'PA_7', 'MISO': 'PA_6', 'SCLK': 'PA_5'}
        result = self.pinmap.validate_spi_bus(pins, "SPI1")
        self.assertTrue(result, "SPI1 pins should validate")

        # Invalid: Wrong bus
        result = self.pinmap.validate_spi_bus(pins, "SPI99")
        self.assertFalse(result, "Invalid SPI bus should fail")

        # Invalid: Mixed buses
        pins = {'MOSI': 'PA_7', 'MISO': 'PB_14', 'SCLK': 'PA_5'}  # MISO on SPI2
        result = self.pinmap.validate_spi_bus(pins, "SPI1")
        self.assertFalse(result, "Mixed SPI buses should fail")

    def test_validate_i2c_bus(self):
        """Test I2C bus validation."""
        # Valid: I2C1 = PB8 (SCL), PB9 (SDA)
        pins = {'SCL': 'PB_8', 'SDA': 'PB_9'}
        result = self.pinmap.validate_i2c_bus(pins, "I2C1")
        self.assertTrue(result, "I2C1 pins should validate")

        # Invalid: Wrong bus
        result = self.pinmap.validate_i2c_bus(pins, "I2C99")
        self.assertFalse(result, "Invalid I2C bus should fail")

    def test_validate_uart(self):
        """Test UART validation."""
        # Valid: USART1 = PB6 (TX), PB7 (RX)
        pins = {'TX': 'PB_6', 'RX': 'PB_7'}
        result = self.pinmap.validate_uart(pins, "USART1")
        self.assertTrue(result, "USART1 pins should validate")

        # Invalid: Wrong UART
        result = self.pinmap.validate_uart(pins, "USART99")
        self.assertFalse(result, "Invalid UART should fail")

    def test_get_spi_bus(self):
        """Test SPI bus lookup."""
        bus = self.pinmap.get_spi_bus("PA_7", "MOSI")
        self.assertEqual(bus, "SPI1", "PA_7 MOSI should be SPI1")

        bus = self.pinmap.get_spi_bus("PB_15", "MOSI")
        self.assertEqual(bus, "SPI2", "PB_15 MOSI should be SPI2")

        bus = self.pinmap.get_spi_bus("PX_99", "MOSI")
        self.assertIsNone(bus, "Invalid pin should return None")

    def test_get_i2c_bus(self):
        """Test I2C bus lookup."""
        bus = self.pinmap.get_i2c_bus("PB_8", "SCL")
        self.assertEqual(bus, "I2C1", "PB_8 SCL should be I2C1")

        bus = self.pinmap.get_i2c_bus("PX_99", "SCL")
        self.assertIsNone(bus, "Invalid pin should return None")

    def test_get_uart(self):
        """Test UART lookup."""
        uart = self.pinmap.get_uart("PB_6", "TX")
        self.assertEqual(uart, "USART1", "PB_6 TX should be USART1")

        uart = self.pinmap.get_uart("PA_2", "TX")
        self.assertEqual(uart, "USART2", "PA_2 TX should be USART2")

        uart = self.pinmap.get_uart("PX_99", "TX")
        self.assertIsNone(uart, "Invalid pin should return None")

    def test_jhef411_motor_validation(self):
        """Test motor pins from JHEF411 config."""
        # MOTOR 1: PA8 = TIM1_CH1 on AF1
        channel = self.pinmap.validate_timer("PA_8", "TIM1", 1)
        self.assertEqual(channel, 1, "Motor 1 (PA8) should be TIM1_CH1")

        # MOTOR 5: PB4 = TIM3_CH1 on AF2
        channel = self.pinmap.validate_timer("PB_4", "TIM3", 2)
        self.assertEqual(channel, 1, "Motor 5 (PB4) should be TIM3_CH1")

    def test_jhef411_spi_validation(self):
        """Test SPI buses from JHEF411 config."""
        # SPI1 (IMU): PA5/PA6/PA7
        spi1_pins = {'MOSI': 'PA_7', 'MISO': 'PA_6', 'SCLK': 'PA_5'}
        self.assertTrue(self.pinmap.validate_spi_bus(spi1_pins, "SPI1"),
                        "JHEF411 SPI1 (IMU) should validate")

        # SPI2 (Flash/OSD): PB13/PB14/PB15
        spi2_pins = {'MOSI': 'PB_15', 'MISO': 'PB_14', 'SCLK': 'PB_13'}
        self.assertTrue(self.pinmap.validate_spi_bus(spi2_pins, "SPI2"),
                        "JHEF411 SPI2 (Flash/OSD) should validate")

    def test_jhef411_i2c_validation(self):
        """Test I2C bus from JHEF411 config."""
        # I2C1 (Sensors): PB8/PB9
        i2c1_pins = {'SCL': 'PB_8', 'SDA': 'PB_9'}
        self.assertTrue(self.pinmap.validate_i2c_bus(i2c1_pins, "I2C1"),
                        "JHEF411 I2C1 (sensors) should validate")

    def test_jhef411_uart_validation(self):
        """Test UARTs from JHEF411 config."""
        # UART1: PB6/PB7
        uart1_pins = {'TX': 'PB_6', 'RX': 'PB_7'}
        self.assertTrue(self.pinmap.validate_uart(uart1_pins, "USART1"),
                        "JHEF411 UART1 should validate")

        # UART2: PA2/PA3
        uart2_pins = {'TX': 'PA_2', 'RX': 'PA_3'}
        self.assertTrue(self.pinmap.validate_uart(uart2_pins, "USART2"),
                        "JHEF411 UART2 should validate")


if __name__ == '__main__':
    unittest.main()
