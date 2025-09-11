#ifndef CUSTOM_SOCKET
#define CUSTOM_SOCKET

#include "MultiLogger.h"
#include <stdint.h>
#include <string>
#include <cstring>

// Detect environment
#if defined(ARDUINO) || defined(PICO_SDK_VERSION_MAJOR)

    #define EMBEDDED
    #include <SPI.h>
    #include <mcp2515.h>

    class ICANInterface {
    public:
        virtual bool send(const can_frame& frame) = 0;
        virtual bool receive(can_frame& frame) = 0;
        //virtual void setTimeout(int timeoutMs);
        virtual ~ICANInterface() = default;
    };

    class ArduinoCAN : public ICANInterface {
        struct can_frame cf;
    public:
        MCP2515 mcp2515;

        bool init();

        bool send(const can_frame& frame) override;

        bool receive(can_frame& frame) override;
    };

#else   // PC / Desktop
    #include <linux/can.h>
    #include <linux/can/raw.h>
    #include <sys/socket.h>
    #include <net/if.h>
    #include <unistd.h>
    #include <sys/ioctl.h>

    class ICANInterface {
    public:
        virtual bool init(const std::string& iface) = 0;
        virtual bool send(const can_frame& frame) = 0;
        virtual bool receive(can_frame& frame) = 0;
        virtual void setTimeout(int timeoutMs) = 0;
        virtual ~ICANInterface() = default;
    };

    class SocketCAN : public ICANInterface {
        int sock;
    public:
        bool init(const std::string& iface) override;

        bool send(const can_frame& frame) override;

        bool receive(can_frame& frame) override;

        void setTimeout(int timeoutMs) override;

        bool isOBD2(can_frame& frame);

        ~SocketCAN() override;
    };
#endif

#endif
