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
    #include <queue>
    #include <fstream>
    #include <sstream>
    #include <iomanip>

    #include <termios.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <cerrno>
    #include <cstring>
    #include <cctype>
    #include <chrono>
    #include <iostream>
    #include <vector>
    #include <string>
    #include <algorithm>

    class ICANInterface {
    protected:
        int broadcast_id = 0x07DF;
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

    
    class ELM327Transport: public ICANInterface {
    public:
        ELM327Transport();
        ~ELM327Transport();

        bool init(const std::string& serialPort) override;
        void setTimeout(int t) override;

        // Only sends a frame
        bool send(const can_frame &frame) override;

        // Only receives a frame
        bool receive(can_frame& frame) override;

        void closePort();

    private:
        int fd;
        int timeoutMs;
        std::queue<can_frame> frameQueue;

        bool set_raw_mode(int fd, speed_t baud);
        ssize_t write_all(const std::string &s);
        std::string read_until_prompt(int timeoutSeconds = 3);
        void sendRaw(const std::string& cmd);
        bool readFullResponse(std::vector<uint8_t>& outPayload);
        void splitPayloadIntoFrames(const std::vector<uint8_t>& payload);

        // helper: uppercase hex string
        static std::string to_upper_hex(unsigned int x) {
            std::ostringstream ss; ss << std::hex << std::uppercase << x;
            return ss.str();
        }
    };
#endif

#endif
