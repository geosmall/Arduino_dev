/*
 * IBus_Basic - Basic IBus RC receiver example
 *
 * Demonstrates reading RC channels from a FlySky IBus receiver
 * using the SerialRx library.
 *
 * Hardware connections:
 *   RC Receiver IBus → USART1 RX (PA10 on NUCLEO_F411RE)
 *   RC Receiver GND  → GND
 *   RC Receiver VCC  → 5V (if receiver needs 5V power)
 *
 * Protocol: IBus @ 115200 baud
 * Expected: 10 RC channels (1000-2000 us typical range)
 */

#include <SerialRx.h>
#include <ci_log.h>

// Create HardwareSerial instance for USART1 (RX=PA10, TX=PA9)
HardwareSerial SerialRC(PA10, PA9);

// Create SerialRx instance (protocol configured in setup)
SerialRx rc;

void setup() {
  // Initialize Serial for debug output (not needed with RTT)
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("IBus RC Receiver - Basic Example\n");
  CI_LOG("===================================\n");

  // Configure RC receiver on USART1
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.protocol = SerialRx::IBUS;  // IBus protocol
  config.baudrate = 115200;          // IBus standard baudrate
  config.timeout_ms = 1000;          // 1 second failsafe timeout

  if (rc.begin(config)) {
    CI_LOG("RC Receiver initialized on USART1 (PA10/PA9)\n");
    CI_LOG("Waiting for IBus frames...\n\n");
  } else {
    CI_LOG("ERROR: Failed to initialize RC receiver!\n");
    while (1);  // Halt on error
  }

  CI_BUILD_INFO();
  CI_READY_TOKEN();
}

// Track signal state for failsafe reporting
static bool signal_lost = false;

void loop() {
  // Update RC receiver (polls Serial.available())
  rc.update();

  // Check for new messages
  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      // Print first 4 channels
      CI_LOGF("Ch1: %4d  Ch2: %4d  Ch3: %4d  Ch4: %4d  ",
              msg.channels[0], msg.channels[1],
              msg.channels[2], msg.channels[3]);

      // Print all 10 channels on second line
      CI_LOGF("All: ");
      for (int i = 0; i < RC_NUM_CHANNELS; i++) {
        CI_LOGF("%d:%4d ", i + 1, msg.channels[i]);
      }
      CI_LOG("\n");
    }
  }

  // Check for timeout (failsafe)
  if (rc.timeout(1000)) {
    if (!signal_lost) {
      uint32_t time_since = rc.timeSinceLastMessage();
      CI_LOGF("WARNING: RC signal timeout (%lu ms since last message)\n", time_since);
      signal_lost = true;
    }
  } else {
    if (signal_lost) {
      CI_LOG("RC signal recovered\n");
      signal_lost = false;
    }
  }

  delay(100);  // Print at 10 Hz
}
