#ifndef MULTI_LOGGER
#define MULTI_LOGGER

#if defined(ARDUINO)        // Arduino boards
    #define EMBEDDED
    #define LOG_INFO(msg)  do { Serial.print("[INFO] ");  Serial.println(msg); } while(0)
    #define LOG_ERR(msg)   do { Serial.print("[ERROR] "); Serial.println(msg); } while(0)

#elif defined(PICO_SDK_VERSION_MAJOR)  // Raspberry Pi Pico SDK
    #define EMBEDDED
    #include <stdio.h>
    #define LOG_INFO(msg)  do { printf("[INFO] %s\n", msg); } while(0)
    #define LOG_ERR(msg)   do { printf("[ERROR] %s\n", msg); } while(0)
    #define LOG_WARN(msg)   do { printf("[WARN] %s\n", msg); } while(0)

#else   // PC / Desktop
    #include <spdlog/spdlog.h>
    
    #define LOG_INFO(msg)  spdlog::info(msg)
    #define LOG_ERR(msg)   spdlog::error(msg)
    #define LOG_WARN(msg)   spdlog::warn(msg)
#endif

#endif