#include "IMU.h"
#include "stm32yyxx_ll_system.h"  // For DWT cycle counter

// External symbols needed by TDK driver
extern "C" {
    void inv_disable_irq(void);
    void inv_enable_irq(void);
    uint64_t inv_timer_get_counter(unsigned timer_num);
    void inv_delay_us(uint32_t us);
}

// IRQ nesting counter
static uint32_t sDisableIntCount = 0;

void inv_disable_irq(void)
{
    if (sDisableIntCount == 0) {
        __disable_irq();
    }
    sDisableIntCount++;
}

void inv_enable_irq(void)
{
    sDisableIntCount--;
    if (sDisableIntCount == 0) {
        __enable_irq();
    }
}

uint64_t inv_timer_get_counter(unsigned timer_num)
{
    (void)timer_num;
    return micros();
}

void inv_delay_us(uint32_t us)
{
    delayMicroseconds(us);
}

// ============================================================================
// IMU Class Implementation
// ============================================================================

IMU::IMU()
{
    // Initialize DWT ticks per microsecond
    us_ticks_ = SystemCoreClock / 1000000;
}

IMU::Result IMU::Init(SPIClass& spi, uint32_t cs_pin, uint32_t spi_freq_hz)
{
    // Store SPI reference and configuration
    p_spi_ = &spi;
    cs_pin_ = cs_pin;
    spi_freq_hz_ = spi_freq_hz;
    cs_pin_name_ = digitalPinToPinName(cs_pin);

    // Initialize CS pin
    pinMode(cs_pin_, OUTPUT);
    digitalWrite(cs_pin_, HIGH);

    // Initialize SPI
    p_spi_->begin();

    // Set up the TDK driver transport layer
    driver_.transport.context = this;  // Store this pointer for callbacks
    driver_.transport.read_reg = spiReadRegs;
    driver_.transport.write_reg = spiWriteRegs;
    driver_.transport.configure = spiConfigure;
    driver_.transport.serif.context = this;
    driver_.transport.serif.serif_type = ICM426XX_UI_SPI4;
    driver_.transport.serif.is_spi = 1;

    // Initialize the TDK high-level driver
    int rc = inv_icm426xx_init(&driver_, &driver_.transport.serif, DriverEventCb);
    if (rc != 0) {
        return Result::ERR;
    }

    // Check WHO_AM_I
    uint8_t who_am_i = 0;
    rc = inv_icm426xx_get_who_am_i(&driver_, &who_am_i);
    if (rc != 0 || who_am_i != ICM_WHOAMI) {
        return Result::ERR;
    }

    initialized_ = true;
    return Result::OK;
}

IMU::Result IMU::ConfigureInvDevice(AccelFS acc_fsr_g, GyroFS gyr_fsr_dps,
                                     AccelODR acc_freq, GyroODR gyr_freq)
{
    if (!initialized_) {
        return Result::ERR;
    }

    int rc = 0;

    // Set FSR
    rc |= inv_icm426xx_set_accel_fsr(&driver_, static_cast<ICM426XX_ACCEL_CONFIG0_FS_SEL_t>(acc_fsr_g));
    rc |= inv_icm426xx_set_gyro_fsr(&driver_, static_cast<ICM426XX_GYRO_CONFIG0_FS_SEL_t>(gyr_fsr_dps));

    // Set ODR
    rc |= inv_icm426xx_set_accel_frequency(&driver_, static_cast<ICM426XX_ACCEL_CONFIG0_ODR_t>(acc_freq));
    rc |= inv_icm426xx_set_gyro_frequency(&driver_, static_cast<ICM426XX_GYRO_CONFIG0_ODR_t>(gyr_freq));

    // Update sensitivity values
    rc |= inv_icm426xx_get_accel_fsr(&driver_, reinterpret_cast<ICM426XX_ACCEL_CONFIG0_FS_SEL_t*>(&acc_fsr_g));
    rc |= inv_icm426xx_get_gyro_fsr(&driver_, reinterpret_cast<ICM426XX_GYRO_CONFIG0_FS_SEL_t*>(&gyr_fsr_dps));

    // Calculate sensitivity from FSR
    switch (acc_fsr_g) {
        case gpm2:  accel_sensitivity_ = 16384.0f; break;
        case gpm4:  accel_sensitivity_ = 8192.0f; break;
        case gpm8:  accel_sensitivity_ = 4096.0f; break;
        case gpm16: accel_sensitivity_ = 2048.0f; break;
    }

    switch (gyr_fsr_dps) {
        case dps250:  gyro_sensitivity_ = 1311.0f; break;
        case dps500:  gyro_sensitivity_ = 655.0f; break;
        case dps1000: gyro_sensitivity_ = 328.0f; break;
        case dps2000: gyro_sensitivity_ = 164.0f; break;
    }

    return (rc == 0) ? Result::OK : Result::ERR;
}

int IMU::Reset()
{
    if (!initialized_) return -1;
    return inv_icm426xx_soft_reset(&driver_);
}

int IMU::SetPwrState(PwrState state)
{
    if (!initialized_) return -1;

    if (state == POWER_ON) {
        return inv_icm426xx_enable_accel_low_noise_mode(&driver_) |
               inv_icm426xx_enable_gyro_low_noise_mode(&driver_);
    } else {
        return inv_icm426xx_disable_accel(&driver_) |
               inv_icm426xx_disable_gyro(&driver_);
    }
}

int IMU::EnableAccelLNMode()
{
    if (!initialized_) return -1;
    return inv_icm426xx_enable_accel_low_noise_mode(&driver_);
}

int IMU::DisableAccel()
{
    if (!initialized_) return -1;
    return inv_icm426xx_disable_accel(&driver_);
}

int IMU::EnableGyroLNMode()
{
    if (!initialized_) return -1;
    return inv_icm426xx_enable_gyro_low_noise_mode(&driver_);
}

int IMU::DisableGyro()
{
    if (!initialized_) return -1;
    return inv_icm426xx_disable_gyro(&driver_);
}

int IMU::SetAccelODR(AccelODR frequency)
{
    if (!initialized_) return -1;
    return inv_icm426xx_set_accel_frequency(&driver_, static_cast<ICM426XX_ACCEL_CONFIG0_ODR_t>(frequency));
}

int IMU::SetGyroODR(GyroODR frequency)
{
    if (!initialized_) return -1;
    return inv_icm426xx_set_gyro_frequency(&driver_, static_cast<ICM426XX_GYRO_CONFIG0_ODR_t>(frequency));
}

int IMU::SetAccelFSR(AccelFS fsr)
{
    if (!initialized_) return -1;
    int rc = inv_icm426xx_set_accel_fsr(&driver_, static_cast<ICM426XX_ACCEL_CONFIG0_FS_SEL_t>(fsr));

    // Update sensitivity
    if (rc == 0) {
        switch (fsr) {
            case gpm2:  accel_sensitivity_ = 16384.0f; break;
            case gpm4:  accel_sensitivity_ = 8192.0f; break;
            case gpm8:  accel_sensitivity_ = 4096.0f; break;
            case gpm16: accel_sensitivity_ = 2048.0f; break;
        }
    }
    return rc;
}

int IMU::SetGyroFSR(GyroFS fsr)
{
    if (!initialized_) return -1;
    int rc = inv_icm426xx_set_gyro_fsr(&driver_, static_cast<ICM426XX_GYRO_CONFIG0_FS_SEL_t>(fsr));

    // Update sensitivity
    if (rc == 0) {
        switch (fsr) {
            case dps250:  gyro_sensitivity_ = 1311.0f; break;
            case dps500:  gyro_sensitivity_ = 655.0f; break;
            case dps1000: gyro_sensitivity_ = 328.0f; break;
            case dps2000: gyro_sensitivity_ = 164.0f; break;
        }
    }
    return rc;
}

int IMU::EnableDataReadyInt1()
{
    if (!initialized_) return -1;
    return inv_icm426xx_enable_accel_gyro_data_ready_int1(&driver_);
}

int IMU::DisableDataReadyInt1()
{
    if (!initialized_) return -1;
    return inv_icm426xx_disable_accel_gyro_data_ready_int1(&driver_);
}

int IMU::RunSelfTest(int* result, std::array<int, 6>* bias)
{
    if (!initialized_) return -1;

    if (bias != nullptr) {
        return inv_icm426xx_run_selftest(&driver_, result, bias->data());
    } else {
        return inv_icm426xx_run_selftest(&driver_, result, nullptr);
    }
}

int IMU::ReadDataFromRegisters()
{
    if (!initialized_) return -1;
    return inv_icm426xx_get_data_from_registers(&driver_);
}

int IMU::ReadIMU6(std::array<int16_t, 6>& buf)
{
    if (!initialized_) return -1;

    uint8_t raw_data[NUM_DATA_BYTES];
    int rc = 0;

    // Read all 12 bytes (6 accel + 6 gyro) at once
    SelectDevice();

    // Send read command for ACCEL_DATA_X1 register
    uint8_t reg_addr = MPUREG_ACCEL_DATA_X1_UI | 0x80;  // Set read bit
    p_spi_->beginTransaction(SPISettings(spi_freq_hz_, MSBFIRST, SPI_MODE0));
    p_spi_->transfer(reg_addr);

    // Read data
    for (uint32_t i = 0; i < NUM_DATA_BYTES; i++) {
        raw_data[i] = p_spi_->transfer(0xFF);
    }

    p_spi_->endTransaction();
    DeselectDevice();

    // Convert to int16_t
    buf[0] = (int16_t)((raw_data[0] << 8) | raw_data[1]);   // Accel X
    buf[1] = (int16_t)((raw_data[2] << 8) | raw_data[3]);   // Accel Y
    buf[2] = (int16_t)((raw_data[4] << 8) | raw_data[5]);   // Accel Z
    buf[3] = (int16_t)((raw_data[6] << 8) | raw_data[7]);   // Gyro X
    buf[4] = (int16_t)((raw_data[8] << 8) | raw_data[9]);   // Gyro Y
    buf[5] = (int16_t)((raw_data[10] << 8) | raw_data[11]); // Gyro Z

    return rc;
}

int IMU::ReadDataFromFifo()
{
    if (!initialized_) return -1;
    return inv_icm426xx_get_data_from_fifo(&driver_);
}

void IMU::SetSensorEventCallback(void (*userCb)(inv_icm426xx_sensor_event_t *event))
{
    user_event_cb_ = userCb;
}

// ============================================================================
// Private Methods
// ============================================================================

void IMU::DelayNs(uint32_t delay_ns)
{
    const uint32_t start = DWT->CYCCNT;
    const uint32_t ticks = (delay_ns * us_ticks_) / 1000;
    while ((DWT->CYCCNT - start) < ticks) {}
}

void IMU::SelectDevice()
{
    digitalWriteFast(cs_pin_name_, LOW);
    DelayNs(SETUP_TIME_NS);
}

void IMU::DeselectDevice()
{
    DelayNs(HOLD_TIME_NS);
    digitalWriteFast(cs_pin_name_, HIGH);
}

void IMU::DriverEventCb(inv_icm426xx_sensor_event_t *event)
{
    // Get IMU instance from context
    IMU* imu = static_cast<IMU*>(event->sensor_mask);  // TDK driver stores context here

    if (imu && imu->user_event_cb_) {
        imu->user_event_cb_(event);
    }
}

// ============================================================================
// TDK Transport Layer Callbacks
// ============================================================================

int IMU::spiReadRegs(struct inv_icm426xx_serif *serif,
                     uint8_t reg,
                     uint8_t *buf,
                     uint32_t len)
{
    IMU* imu = static_cast<IMU*>(serif->context);
    if (!imu || !imu->p_spi_) return -1;

    imu->SelectDevice();

    imu->p_spi_->beginTransaction(SPISettings(imu->spi_freq_hz_, MSBFIRST, SPI_MODE0));
    imu->p_spi_->transfer(reg | 0x80);  // Set read bit

    for (uint32_t i = 0; i < len; i++) {
        buf[i] = imu->p_spi_->transfer(0xFF);
    }

    imu->p_spi_->endTransaction();
    imu->DeselectDevice();

    return 0;
}

int IMU::spiWriteRegs(struct inv_icm426xx_serif *serif,
                      uint8_t reg,
                      const uint8_t *buf,
                      uint32_t len)
{
    IMU* imu = static_cast<IMU*>(serif->context);
    if (!imu || !imu->p_spi_) return -1;

    for (uint32_t i = 0; i < len; i++) {
        imu->SelectDevice();

        imu->p_spi_->beginTransaction(SPISettings(imu->spi_freq_hz_, MSBFIRST, SPI_MODE0));
        imu->p_spi_->transfer(reg + i);  // Write bit is 0
        imu->p_spi_->transfer(buf[i]);
        imu->p_spi_->endTransaction();

        imu->DeselectDevice();
    }

    return 0;
}

int IMU::spiConfigure(struct inv_icm426xx_serif *serif)
{
    // No-op for Arduino - SPI already configured
    (void)serif;
    return 0;
}
