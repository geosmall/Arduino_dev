// ci_log.h — drop into your sketch
#pragma once

#if defined(USE_RTT)
  #include "SEGGER_RTT.h"
  
  // Build traceability handling
  #ifdef __has_include
    #if __has_include("build_id.h")
      #include "build_id.h"
      #define HAS_BUILD_ID
    #endif
  #endif
  #ifndef HAS_BUILD_ID
    #define BUILD_GIT_SHA "unknown"
    #define BUILD_UTC_TIME "unknown"
  #endif
  
  #define CI_LOGF(...)   SEGGER_RTT_printf(0, __VA_ARGS__)
  #define CI_LOG(s)      SEGGER_RTT_WriteString(0, s)
  #define CI_BUILD_INFO() CI_LOGF("Build: %s (%s)\n", BUILD_GIT_SHA, BUILD_UTC_TIME)
  #define CI_READY_TOKEN() CI_LOGF("READY NUCLEO_F411RE %s %s\n", BUILD_GIT_SHA, BUILD_UTC_TIME)
#else
  #define CI_LOGF(...)   Serial.printf(__VA_ARGS__)
  #define CI_LOG(s)      Serial.print(s)
  #define CI_BUILD_INFO() do { } while(0) // No-op in Serial mode
  #define CI_READY_TOKEN() do { } while(0) // No-op in Serial mode
#endif