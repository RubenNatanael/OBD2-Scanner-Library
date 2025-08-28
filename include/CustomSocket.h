#ifndef CUSTOM_SOCKET
#define CUSTOM_SOCKET

#include "MultiLogger.h"
#include <stdint.h>

// namespace custom {
//     struct can_frame {
//         uint32_t id;
//         uint8_t  dlc;
//         uint8_t  data[8];
//     };
// }


class ICANInterface {
public:
    virtual bool send(const can_frame& frame) = 0;
    virtual bool receive(can_frame& frame) = 0;
    virtual void setTimeout(int timeoutMs);
    virtual ~ICANInterface() = default;
};

// Detect environment
#if defined(ARDUINO) || defined(PICO_SDK_VERSION_MAJOR)

    #define EMBEDDED
    // Use can library
    #define EMBEDDED
    #include <SPI.h>
    #include <mcp2515.h>

    class ArduinoCAN : public ICANInterface {
        struct can_frame cf;
    public:
        MCP2515 mcp2515;

        bool init();

        bool send(const can_frame& frame) override;

        bool receive(can_frame& frame) override;
    };

#else   // PC / Desktop
    //Use socket
    #include <linux/can.h>
    #include <linux/can/raw.h>
    #include <sys/socket.h>
    #include <net/if.h>
    #include <unistd.h>
    #include <sys/ioctl.h>

    class SocketCAN : public ICANInterface {
        int sock;
    public:
        bool init(const std::string& iface);

        bool send(const can_frame& frame) override;

        bool receive(can_frame& frame) override;

        void setTimeout(int timeoutMs);

        ~SocketCAN();
    };
#endif

#endif
