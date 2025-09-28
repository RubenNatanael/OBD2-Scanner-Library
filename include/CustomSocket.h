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

    #include <queue>
    #include <iomanip>
    #include <cstring>
    #include <chrono>
    #include <vector>
    #include <string>
    #include <mutex>

    #include <linux/can.h>
    #include <linux/can/raw.h>
    #include <sys/socket.h>
    #include <net/if.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <termios.h>

    class ICANInterface {
    protected:
        const uint32_t broadcast_id = 0x07DF;
        uint32_t last_id = broadcast_id;
    public:
        virtual bool init(const std::string& iface) = 0;
        virtual bool send(const can_frame& frame) = 0;
        virtual bool receive(can_frame& frame) = 0;
        virtual void setTimeout(int timeoutMs) = 0;
        virtual char getProtocol() = 0;
        virtual ~ICANInterface() = default;
    };

    class SocketCAN : public ICANInterface {
        int sock;
        int timeoutMs = 500;
    public:
        bool init(const std::string& iface) override;

        bool send(const can_frame& frame) override;

        bool receive(can_frame& frame) override;

        void setTimeout(int timeoutMs) override;

        bool isOBD2(can_frame& frame);

        char getProtocol();

        ~SocketCAN() override;
    };

    
    class ELM327Transport: public ICANInterface {
    public:
        ELM327Transport();
        
        /**
         * 
         * @param baudRete B38400(default), B115200, B9600
         * @param protocol
         * '0 for "ATSP0" - Auto (default)
         * '1' for "ATSP1" - SAE J1850 PWM
         * '2' for "ATSP2" - SAE J1850 VPW
         * '3' for "ATSP3" - ISO 9141-2
         * '4' for "ATSP4" - ISO 14230-4 KWP
         * '5' for "ATSP5" - ISO 14230-4SAE  KWP (fast init)
         * '6' for "ATSP6" - ISO 15765-4 CAN (11 bit, 500 kbps)
         * '7' for "ATSP7" - ISO 15765-4 CAN (29 bit, 500 kbps)
         * '8' for "ATSP8" - ISO 15765-4 CAN (11 bit, 250 kbps)
         * '9' for "ATSP9" - ISO 15765-4 CAN (29 bit, 250 kbps)
         */
        ELM327Transport(speed_t baudRate, char protocol);

        ~ELM327Transport();

        bool init(const std::string& serialPort) override;
        void setTimeout(int t) override;

        // Only sends a frame
        bool send(const can_frame &frame) override;

        // Only receives a frame
        bool receive(can_frame& frame) override;

        char getProtocol();

        void closePort();

    private:
        int fd = -1;
        int timeoutMs = 500;
        speed_t baudRate = B38400;
        char protocol = '0';
        std::queue<can_frame> frameQueue;
        std::mutex ioMutex;

        bool set_raw_mode(int fd, speed_t baud);
        ssize_t write_all(const std::string &s);
        std::string read_until_prompt(int timeoutSeconds = 3);
        void sendRaw(const std::string& cmd);
        bool readFullResponse(std::vector<uint8_t>& outPayload);
        void splitPayloadIntoFrames(const std::vector<uint8_t>& payload);
        bool initChip();

        static std::string to_upper_hex(unsigned int x) {
            std::ostringstream ss; ss << std::hex << std::uppercase << x;
            return ss.str();
        }
    };
#endif

#endif
