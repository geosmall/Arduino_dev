/*
 * ICM42688P Manufacturer Self-Test Integration
 *
 * This example integrates TDK InvenSense factory self-test code with STM32 Arduino Core
 * and CI/HIL testing framework. It demonstrates complete manufacturer driver integration
 * while preserving factory code integrity.
 *
 * HARDWARE VERIFICATION:
 * - Target: NUCLEO-F411RE development board
 * - IMU: ICM42688P 6-axis IMU sensor
 * - Connection: SPI with jumper wires (1MHz for reliability)
 * - Pin mapping: CS=PA4, MOSI=PA7, MISO=PA6, SCLK=PA5
 *
 * VERIFICATION RESULTS (2025-09-27):
 * - WHO_AM_I: 0x47 (confirmed ICM42688P device ID)
 * - Gyro Self-Test: PASS
 * - Accel Self-Test: PASS
 * - Gyro Bias (dps): x=0.358582, y=-0.778198, z=0.251770
 * - Accel Bias (g): x=-0.010132, y=0.044250, z=0.039490
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Deterministic exit with "*STOP*" wildcard
 * - Build traceability with git SHA and timestamp
 */

#include "icm42688p.h"
#include <ci_log.h>
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"
#include <SPI.h>

#include "inv_main.h"
#include "inv_uart.h"

#include <cstring>

// libPrintf integration - automatic aliasing enabled
#include <libPrintf.h>
// Uncomment to use software driven NSS
#define USE_SOFT_NSS
#define DESIRED_SPI_FREQ 1000000

constexpr bool off = 0;
constexpr bool on = 1;

// Pin definitions for NUCLEO F411RE (from BoardConfig::imu)
#define IMU_CS_PIN    PA4
#define IMU_MOSI_PIN  PA7
#define IMU_MISO_PIN  PA6
#define IMU_SCLK_PIN  PA5
#define IMU_SPI_SPEED 1000000  // 1MHz for reliable jumper connections

// Create SPI instance with specific pins (software CS control)
SPIClass spi_bus(IMU_MOSI_PIN, IMU_MISO_PIN, IMU_SCLK_PIN);  // No SSEL = software CS

// Global print buffer
char buf[128];

void setup() {
    // Initialize the UVOS board hardware
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("Initializing Example SelfTest...\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Give ICM-42688P some time to stabilize
    delay(5);

    // Call the main function of the Invensense example
    inv_main();

    // Add completion message for HIL testing
    CI_LOG("*STOP*\n");
}

// Infinite loop
void loop() {
}

/* --------------------------------------------------------------------------------------
 *  Extern functions definition - Invensense to UVOS adapters
 * -------------------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* This variable contains the number of nested calls to disable_irq */
static uint32_t sDisableIntCount = 0;

void inv_disable_irq(void)
{
    if(sDisableIntCount == 0) {
        __disable_irq();
    }
    sDisableIntCount ++;
}

void inv_enable_irq(void)
{
    sDisableIntCount --;
    if(sDisableIntCount == 0) {
        __enable_irq();
    }
}

// Embedded printf putchar implementation for output routing
void putchar_(char c) {
#ifdef USE_RTT
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);
#else
    Serial.print(c);
#endif
}

int inv_uart_mngr_puts(inv_uart_num_t uart_num, const char* s, unsigned short l)
{
    // The string 's' is already formatted by embedded printf with proper float support!
    // Just output it directly through our CI_LOG system
    static char temp_buf[256];
    int copy_len = (l < sizeof(temp_buf) - 1) ? l : sizeof(temp_buf) - 1;
    memcpy(temp_buf, s, copy_len);
    temp_buf[copy_len] = '\0';

    // Use CI_LOG for both RTT and Serial output
    CI_LOG(temp_buf);
    return 1;
}

uint64_t inv_timer_get_counter(unsigned timer_num)
{
    return micros();
}

void inv_delay_us(uint32_t us)
{
    delayMicroseconds(us);
}

#ifdef __cplusplus
}
#endif
