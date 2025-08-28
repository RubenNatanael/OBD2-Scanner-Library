#include "../include/CustomSocket.h"

#ifdef EMBEDDED

bool ArduinoCAN::init() {
    mcp2515.reset();
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();
    return true;
}

bool ArduinoCAN::send(const can_frame& frame) {
    return mcp2515.sendMessage(&frame) == MCP2515::ERROR_OK;
}

bool ArduinoCAN::receive(can_frame& frame) {
    struct can_frame cf;
    if (mcp2515.readMessage(&frame) == MCP2515::ERROR_OK) {
        return true;
    }
    return false;
}

#else   // PC / Desktop

bool SocketCAN::init(const std::string& iface) {
    struct ifreq ifr;
    struct sockaddr_can addr;
    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) return false;

    strcpy(ifr.ifr_name, iface.c_str());
    ioctl(sock, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    return bind(sock, (struct sockaddr*)&addr, sizeof(addr)) >= 0;
}

bool SocketCAN::send(const can_frame& frame) {
    return ::write(sock, &frame, sizeof(frame)) == sizeof(frame);
}

bool SocketCAN::receive(can_frame& frame) {
    if (::read(sock, &frame, sizeof(frame)) != sizeof(frame)) return false;
    return true;
}
SocketCAN::~SocketCAN() {
    close(sock);
}

#endif
