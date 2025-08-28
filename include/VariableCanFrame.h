// #ifndef VARIABLE_CAN_FRAME
// #define VARIABLE_CAN_FRAME

// #define CAN_MAX_DLEN 8

// #include <stdint.h>

// struct CANFrame {
//     uint32_t id;
//     uint8_t  dlc;
//     uint8_t  data[8];
// };

// class ICANInterface {
// public:
//     virtual bool send(const CANFrame& frame) = 0;
//     virtual bool receive(CANFrame& frame) = 0;
//     virtual ~ICANInterface() = default;
// };

// // Detect environment
// #if defined(ARDUINO) || defined(PICO_SDK_VERSION_MAJOR)

//     #define EMBEDDED

// #else   // PC / Desktop
//     #include <linux/can.h>
//     #include <linux/can/raw.h>
//     #include <sys/socket.h>
//     #include <net/if.h>
//     #include <unistd.h>

//     class SocketCAN : public ICANInterface {
//         int sock;
//     public:
//         bool init(const std::string& iface) {
//             struct ifreq ifr;
//             struct sockaddr_can addr;
//             sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
//             if (sock < 0) return false;

//             strcpy(ifr.ifr_name, iface.c_str());
//             ioctl(sock, SIOCGIFINDEX, &ifr);
//             addr.can_family = AF_CAN;
//             addr.can_ifindex = ifr.ifr_ifindex;
//             return bind(sock, (struct sockaddr*)&addr, sizeof(addr)) >= 0;
//         }

//         bool send(const CANFrame& frame) override {
//             struct can_frame cf;
//             cf.can_id  = frame.id;
//             cf.can_dlc = frame.dlc;
//             memcpy(cf.data, frame.data, frame.dlc);
//             return ::write(sock, &cf, sizeof(cf)) == sizeof(cf);
//         }

//         bool receive(CANFrame& frame) override {
//             struct can_frame cf;
//             if (::read(sock, &cf, sizeof(cf)) != sizeof(cf)) return false;
//             frame.id  = cf.can_id;
//             frame.dlc = cf.can_dlc;
//             memcpy(frame.data, cf.data, cf.can_dlc);
//             return true;
//         }
//     };
// #endif

// #endif
