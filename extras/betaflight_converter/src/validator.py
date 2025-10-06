"""
Validator that combines Betaflight config with PeripheralPins.c data.
"""

from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple
from pathlib import Path

from betaflight_config import BetaflightConfig
from peripheral_pins import PeripheralPinMap


@dataclass
class ValidationError:
    """Validation error information."""
    severity: str  # "error" or "warning"
    message: str
    resource: Optional[str] = None
    pin: Optional[str] = None


@dataclass
class ValidatedMotor:
    """Validated motor configuration."""
    index: int
    pin_bf: str      # Betaflight format: "B04"
    pin_arduino: str # Arduino format: "PB_4"
    timer: str       # e.g., "TIM3"
    channel: int     # Timer channel (1-4)
    af: int          # Alternate function


@dataclass
class ValidatedSPIBus:
    """Validated SPI bus configuration."""
    bus_num: int
    bus_name: str    # e.g., "SPI1"
    mosi: str        # Arduino format
    miso: str
    sclk: str


@dataclass
class ValidatedI2CBus:
    """Validated I2C bus configuration."""
    bus_num: int
    bus_name: str    # e.g., "I2C1"
    scl: str         # Arduino format
    sda: str


@dataclass
class ValidatedUART:
    """Validated UART configuration."""
    uart_num: int
    uart_name: str   # e.g., "USART1"
    tx: str          # Arduino format
    rx: str


class ConfigValidator:
    """Validates Betaflight config against PeripheralPins.c data."""

    def __init__(self, bf_config: BetaflightConfig, pinmap: PeripheralPinMap):
        """Initialize validator with parsed configs."""
        self.bf_config = bf_config
        self.pinmap = pinmap
        self.errors: List[ValidationError] = []
        self.warnings: List[ValidationError] = []

    def validate_all(self) -> bool:
        """
        Run all validations.

        Returns:
            True if no errors, False if errors found (warnings are OK)
        """
        self.errors.clear()
        self.warnings.clear()

        self.validate_motors()
        self.validate_spi_buses()
        self.validate_i2c_buses()
        self.validate_uarts()

        return len(self.errors) == 0

    def validate_motors(self) -> List[ValidatedMotor]:
        """Validate motor timer assignments."""
        validated_motors = []

        for motor in self.bf_config.get_motors():
            pin_bf = motor.pin
            pin_arduino = self.bf_config.convert_pin_format(pin_bf)

            # Get timer assignment
            if pin_bf not in self.bf_config.timers:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"Motor {motor.index} pin {pin_bf} has no timer assignment",
                    resource=f"MOTOR_{motor.index}",
                    pin=pin_bf
                ))
                continue

            timer_assignment = self.bf_config.timers[pin_bf]

            # Validate against PeripheralPins.c
            channel = self.pinmap.validate_timer(
                pin=pin_arduino,
                timer=timer_assignment.timer,
                af=timer_assignment.af
            )

            if channel is None:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"Motor {motor.index}: {pin_arduino} does not support {timer_assignment.timer} on AF{timer_assignment.af}",
                    resource=f"MOTOR_{motor.index}",
                    pin=pin_bf
                ))
                continue

            # Verify channel matches
            if timer_assignment.channel and channel != timer_assignment.channel:
                self.warnings.append(ValidationError(
                    severity="warning",
                    message=f"Motor {motor.index}: Expected CH{timer_assignment.channel}, found CH{channel}",
                    resource=f"MOTOR_{motor.index}",
                    pin=pin_bf
                ))

            validated_motors.append(ValidatedMotor(
                index=motor.index,
                pin_bf=pin_bf,
                pin_arduino=pin_arduino,
                timer=timer_assignment.timer,
                channel=channel,
                af=timer_assignment.af
            ))

        return validated_motors

    def validate_servos(self) -> List[ValidatedMotor]:
        """Validate servo timer assignments (reuses ValidatedMotor dataclass)."""
        validated_servos = []

        for servo in self.bf_config.get_servos():
            pin_bf = servo.pin
            pin_arduino = self.bf_config.convert_pin_format(pin_bf)

            # Get timer assignment
            if pin_bf not in self.bf_config.timers:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"Servo {servo.index} pin {pin_bf} has no timer assignment",
                    resource=f"SERVO_{servo.index}",
                    pin=pin_bf
                ))
                continue

            timer_assignment = self.bf_config.timers[pin_bf]

            # Validate against PeripheralPins.c
            channel = self.pinmap.validate_timer(
                pin=pin_arduino,
                timer=timer_assignment.timer,
                af=timer_assignment.af
            )

            if channel is None:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"Servo {servo.index}: {pin_arduino} does not support {timer_assignment.timer} on AF{timer_assignment.af}",
                    resource=f"SERVO_{servo.index}",
                    pin=pin_bf
                ))
                continue

            # Verify channel matches
            if timer_assignment.channel and channel != timer_assignment.channel:
                self.warnings.append(ValidationError(
                    severity="warning",
                    message=f"Servo {servo.index}: Expected CH{timer_assignment.channel}, found CH{channel}",
                    resource=f"SERVO_{servo.index}",
                    pin=pin_bf
                ))

            validated_servos.append(ValidatedMotor(
                index=servo.index,
                pin_bf=pin_bf,
                pin_arduino=pin_arduino,
                timer=timer_assignment.timer,
                channel=channel,
                af=timer_assignment.af
            ))

        return validated_servos

    def validate_spi_buses(self) -> List[ValidatedSPIBus]:
        """Validate SPI bus pin assignments."""
        validated_buses = []

        # Find all SPI buses in config
        spi_resources = self.bf_config.get_resources('SPI_SCK')
        bus_numbers = [res.index for res in spi_resources]

        for bus_num in bus_numbers:
            pins_bf = self.bf_config.get_spi_pins(bus_num)
            if not pins_bf:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"SPI{bus_num} has incomplete pin assignments",
                    resource=f"SPI{bus_num}"
                ))
                continue

            # Convert to Arduino format
            pins_arduino = {
                signal: self.bf_config.convert_pin_format(pin)
                for signal, pin in pins_bf.items()
            }

            # Detect bus name from pins
            bus_name = self.pinmap.get_spi_bus(pins_arduino['MOSI'], 'MOSI')
            if not bus_name:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"SPI{bus_num} MOSI pin {pins_arduino['MOSI']} not found in pinmap",
                    resource=f"SPI{bus_num}"
                ))
                continue

            # Validate all pins belong to same bus
            if not self.pinmap.validate_spi_bus(pins_arduino, bus_name):
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"SPI{bus_num} pins don't all belong to {bus_name}",
                    resource=f"SPI{bus_num}"
                ))
                continue

            validated_buses.append(ValidatedSPIBus(
                bus_num=bus_num,
                bus_name=bus_name,
                mosi=pins_arduino['MOSI'],
                miso=pins_arduino['MISO'],
                sclk=pins_arduino['SCLK']
            ))

        return validated_buses

    def validate_i2c_buses(self) -> List[ValidatedI2CBus]:
        """Validate I2C bus pin assignments."""
        validated_buses = []

        # Find all I2C buses in config
        i2c_resources = self.bf_config.get_resources('I2C_SCL')
        bus_numbers = [res.index for res in i2c_resources]

        for bus_num in bus_numbers:
            pins_bf = self.bf_config.get_i2c_pins(bus_num)
            if not pins_bf:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"I2C{bus_num} has incomplete pin assignments",
                    resource=f"I2C{bus_num}"
                ))
                continue

            # Convert to Arduino format
            pins_arduino = {
                signal: self.bf_config.convert_pin_format(pin)
                for signal, pin in pins_bf.items()
            }

            # Detect bus name from pins
            bus_name = self.pinmap.get_i2c_bus(pins_arduino['SCL'], 'SCL')
            if not bus_name:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"I2C{bus_num} SCL pin {pins_arduino['SCL']} not found in pinmap",
                    resource=f"I2C{bus_num}"
                ))
                continue

            # Validate both pins belong to same bus
            if not self.pinmap.validate_i2c_bus(pins_arduino, bus_name):
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"I2C{bus_num} pins don't both belong to {bus_name}",
                    resource=f"I2C{bus_num}"
                ))
                continue

            validated_buses.append(ValidatedI2CBus(
                bus_num=bus_num,
                bus_name=bus_name,
                scl=pins_arduino['SCL'],
                sda=pins_arduino['SDA']
            ))

        return validated_buses

    def validate_uarts(self) -> List[ValidatedUART]:
        """Validate UART pin assignments."""
        validated_uarts = []

        # Find all UARTs in config
        uart_resources = self.bf_config.get_resources('SERIAL_TX')
        uart_numbers = [res.index for res in uart_resources]

        for uart_num in uart_numbers:
            pins_bf = self.bf_config.get_uart_pins(uart_num)
            if not pins_bf:
                self.warnings.append(ValidationError(
                    severity="warning",
                    message=f"UART{uart_num} has incomplete pin assignments (TX only?)",
                    resource=f"UART{uart_num}"
                ))
                continue

            # Convert to Arduino format
            pins_arduino = {
                signal: self.bf_config.convert_pin_format(pin)
                for signal, pin in pins_bf.items()
            }

            # Detect UART name from pins
            uart_name = self.pinmap.get_uart(pins_arduino['TX'], 'TX')
            if not uart_name:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"UART{uart_num} TX pin {pins_arduino['TX']} not found in pinmap",
                    resource=f"UART{uart_num}"
                ))
                continue

            # Validate both pins belong to same UART
            if not self.pinmap.validate_uart(pins_arduino, uart_name):
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"UART{uart_num} pins don't both belong to {uart_name}",
                    resource=f"UART{uart_num}"
                ))
                continue

            validated_uarts.append(ValidatedUART(
                uart_num=uart_num,
                uart_name=uart_name,
                tx=pins_arduino['TX'],
                rx=pins_arduino['RX']
            ))

        return validated_uarts

    def group_motors_by_timer(self, motors: List[ValidatedMotor]) -> Dict[str, List[ValidatedMotor]]:
        """Group motors by timer for bank generation."""
        timer_banks = {}

        for motor in motors:
            if motor.timer not in timer_banks:
                timer_banks[motor.timer] = []
            timer_banks[motor.timer].append(motor)

        return timer_banks

    def get_validation_summary(self) -> str:
        """Get human-readable validation summary."""
        lines = []
        lines.append(f"Validation Summary:")
        lines.append(f"  Errors: {len(self.errors)}")
        lines.append(f"  Warnings: {len(self.warnings)}")

        if self.errors:
            lines.append("\nErrors:")
            for err in self.errors:
                lines.append(f"  ❌ {err.message}")

        if self.warnings:
            lines.append("\nWarnings:")
            for warn in self.warnings:
                lines.append(f"  ⚠️  {warn.message}")

        return "\n".join(lines)
