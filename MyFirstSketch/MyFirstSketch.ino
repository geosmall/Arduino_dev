#include <stdio.h>
#include "SEGGER_RTT.h"

volatile int _Cnt;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Initialize RTT
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    
    // Phase 1 Build Workflow: J-Link RTT "hello" loop
    SEGGER_RTT_WriteString(0, "=== STM32 Arduino RTT Test ===\r\n");
    SEGGER_RTT_printf(0, "Board: NUCLEO_F411RE\r\n");
    SEGGER_RTT_printf(0, "Core: STM32 Arduino v2.7.1\r\n"); 
    SEGGER_RTT_printf(0, "RTT: v8.62 (Full Implementation)\r\n");
    SEGGER_RTT_printf(0, "Build: %s %s\r\n", __DATE__, __TIME__);
    SEGGER_RTT_WriteString(0, "================================\r\n");
    
    // Phase 1 Goal: Emit READY token for automated testing
    SEGGER_RTT_printf(0, "READY F411RE %s\r\n", __DATE__);
    
    _Cnt = 0;
}

void loop() {
    // Keep LED blinking for visual confirmation
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    
    // RTT loop counter
    _Cnt++;
    SEGGER_RTT_printf(0, "[%05d] RTT Loop Counter: %d\r\n", millis(), _Cnt);
    
    // Optional: test various RTT printf formats periodically
    if (_Cnt % 10 == 0) {
        SEGGER_RTT_printf(0, "  --> Hex test: 0x%08X, Decimal: %d, String: \"%s\"\r\n", 
                         _Cnt, _Cnt, "RTT Working!");
    }
}