/* --------------------------------------------------------------------------------------
 *  Extern functions definition - Invensense to UVOS adapters
 * -------------------------------------------------------------------------------------- */

/* Default SPI frequency is 1 Mhz */
static uint32_t spi_freq_hz_ = IMU_SPI_SPEED;
SPIClass* spi_bus_;

#ifdef __cplusplus
extern "C" {
#endif

void inv_board_hal_init(void);
void inv_spi_chip_select_setup_delay(void);
void inv_spi_chip_select_hold_time(void);
void inv_spi_bus_select_device(void);
void inv_spi_bus_deselect_device(void);
uint8_t inv_spi_transfer_byte(uint8_t txByte);
int inv_spi_bus_read_registers(uint8_t addr, uint8_t count, uint8_t* data);
int inv_spi_bus_write_register(uint8_t reg, const uint8_t* data);

/******************************************************/
/* Low-level serial interface function implementation */
/******************************************************/

void inv_io_hal_board_init(void)
{
    // Initialize LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void inv_io_hal_configure_spi_speed(uint8_t spi_freq_mhz)
{
    spi_freq_hz_ = spi_freq_mhz * 1000000;
}

int inv_io_hal_init(struct inv_icm426xx_serif* serif)
{

#if 0 // gls
    uint8_t spi_freq_mhz = spi_default_freq_mhz_;

    // Initialize chip select (CS) pin
    csPin_.Init(CS_PIN, GPIO::Mode::OUTPUT, GPIO::Pull::PULLUP);

    // Configure the Uart Peripheral to print out results
    UartHandler::Config uart_conf;
    uart_conf.periph        = UART_NUM;
    uart_conf.mode          = UartHandler::Config::Mode::TX;
    uart_conf.pin_config.tx = TX_PIN;
    uart_conf.pin_config.rx = RX_PIN;

    // Initialize the uart peripheral and start the DMA transmit
    uart.Init(uart_conf);

    // Configure the ICM-42688P IMU SPI interface (match for Matek_H743 WLITE)
    spi_conf.periph = SpiHandle::Config::Peripheral::SPI_1;
    spi_conf.mode = SpiHandle::Config::Mode::MASTER;
    spi_conf.direction = SpiHandle::Config::Direction::TWO_LINES;
    spi_conf.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
    spi_conf.clock_phase = SpiHandle::Config::ClockPhase::TWO_EDGE;

#ifdef USE_SOFT_NSS
    spi_conf.nss = SpiHandle::Config::NSS::SOFT;
#else
    spi_conf.nss = SpiHandle::Config::NSS::HARD_OUTPUT;
#endif /* USE_SOFT_NSS */

    spi_conf.pin_config.nss = CS_PIN;
    spi_conf.pin_config.sclk = SCLK_PIN;
    spi_conf.pin_config.miso = MISO_PIN;
    spi_conf.pin_config.mosi = MOSI_PIN;

    // spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_32;
    spi_handle.GetBaudHz(spi_conf.periph, (spi_freq_mhz * 1'000'000), spi_conf.baud_prescaler);

    // Initialize the IMU SPI instance
    spi_handle.Init(spi_conf);

#endif // gls

    // Initialize CS pin
    pinMode(IMU_CS_PIN, OUTPUT);
    digitalWrite(IMU_CS_PIN, HIGH);

    // Cast the context back to SPIClass pointer and call begin()
    spi_bus_ = static_cast<SPIClass*>(serif->context);
    spi_bus_->begin(SPISettings(spi_freq_hz_, MSBFIRST, SPI_MODE0));

    return 0;
}

int inv_io_hal_configure(struct inv_icm426xx_serif *serif)
{
    switch (serif->serif_type) {
    default:
        return -1;
    }
}

int inv_io_hal_read_reg(struct inv_icm426xx_serif *serif, uint8_t reg, uint8_t *rbuffer, uint32_t rlen)
{
    return inv_spi_bus_read_registers(reg, (uint8_t)rlen, rbuffer);
}

int inv_io_hal_write_reg(struct inv_icm426xx_serif * serif, uint8_t reg, const uint8_t* wbuffer, uint32_t wlen)
{
    int rc;

    for (uint32_t i = 0; i < wlen; i++) {
        rc = inv_spi_bus_write_register(reg + i, &wbuffer[i]);
        if (rc) {
            return rc;
        }
    }
    return 0;
}

void inv_spi_chip_select_setup_delay(void)
{
    // CS->CLK delay, MPU6000 - 8ns
    // CS->CLK delay, ICM42688P - 39ns
    // System::DelayNs(39);
    delayMicroseconds(1);
}

void inv_spi_chip_select_hold_time(void)
{
    // CLK->CS delay, MPU6000 - 500ns
    // CS->CLK delay, ICM42688P - 18ns
    delayMicroseconds(1);
}

void inv_spi_bus_select_device(void)
{
    // csPin_.Write(GPIO_PIN_RESET);
    digitalWrite(IMU_CS_PIN, LOW);
    inv_spi_chip_select_setup_delay();
}

void inv_spi_bus_deselect_device(void)
{
    inv_spi_chip_select_hold_time();
    // csPin_.Write(GPIO_PIN_SET);
    digitalWrite(IMU_CS_PIN, HIGH);
}

uint8_t inv_spi_transfer_byte(uint8_t txByte)
{
    uint8_t value = 0xFF;
    spi_bus_->transfer(&txByte, &value, 1);
    return value;
}

int inv_spi_bus_read_registers(uint8_t addr, uint8_t count, uint8_t* data)
{
    inv_spi_bus_select_device();

    inv_spi_transfer_byte((addr | 0x80));
    for (uint8_t i = 0; i < count; i++) {
        spi_bus_->transfer(NULL, &data[i], 1);
    }

    inv_spi_bus_deselect_device();

    return 0;
}

int inv_spi_bus_write_register(uint8_t reg, const uint8_t* data)
{
    inv_spi_bus_select_device();

    inv_spi_transfer_byte(reg);
    inv_spi_transfer_byte(*data);

    inv_spi_bus_deselect_device();

    return 0;
}

#ifdef __cplusplus
}
#endif