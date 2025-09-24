#include "icm42688p.h"
#include "../../../../ci_log.h"
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"
#include <SPI.h>

#include "inv_main.h"
#include "inv_uart.h"

#include <cstring>

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

int main(void)
{
    // Initialize the UVOS board hardware
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("Initializing Example SelfTest...\n");
    CI_BUILD_INFO();

    // Create Invn serial spi interface
    inv_icm426xx_serif spi_if = {
        .context = &spi_bus,
    };

    // Give ICM-42688P some time to stabilize
    delay(5);

    // Call the main function of the Invensense example
    inv_main();

    // Infinite loop
    while(1) {
        // Do nothing
    }
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

int inv_uart_mngr_puts(inv_uart_num_t uart_num, const char* s, unsigned short l)
{
    Serial.printf("%.*s", static_cast<int>(l), s);
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
