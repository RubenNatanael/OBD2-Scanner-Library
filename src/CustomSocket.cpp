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

void SocketCAN::setTimeout(int timeoutMs) {
    struct timeval tv;
    tv.tv_sec  = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

bool SocketCAN::send(const can_frame& frame) {
    return ::write(sock, &frame, sizeof(frame)) == sizeof(frame);
}

bool SocketCAN::receive(can_frame& frame) {

    while(true) {
        if (::read(sock, &frame, sizeof(frame)) != sizeof(frame)) return false;

        // Filter frames
        if (isOBD2(frame)) break;
    }

    LOG_INFO("Frame: " + std::to_string(frame.data[0]) + " "
                       + std::to_string(frame.data[1]) + " "
                       + std::to_string(frame.data[2]) + " "
                       + std::to_string(frame.data[3]) + " "
                       + std::to_string(frame.data[4]) + " "
                       + std::to_string(frame.data[5]) + " "
                       + std::to_string(frame.data[6]) + " "
                       + std::to_string(frame.data[7]));
    return true;
}
SocketCAN::~SocketCAN() {
    close(sock);
}

bool SocketCAN::isOBD2(can_frame& frame) {
    LOG_INFO(std::to_string(frame.data[1]));
    if (frame.data[0] != 0x10 && frame.data[1] < 0x4B && frame.data[1] > 0x40) {
        return true;
    } else if( frame.data[0] == 0x10 && frame.data[2] < 0x4B && frame.data[2] > 0x40) {
        return true;
    } else if (frame.data[0] > 0x19 && frame.data[0] < 0x30) {
        return true;
    }
    return false;
}

ELM327Transport::ELM327Transport() : fd(-1), timeoutMs(1000) {}

ELM327Transport::~ELM327Transport() { closePort(); }

void ELM327Transport::setTimeout(int t) { timeoutMs = t; }

bool ELM327Transport::set_raw_mode(int fd, speed_t baud) {
    struct termios tio;
    if (tcgetattr(fd, &tio) != 0) {
        LOG_ERR("Error getting tty");
        return false;
    }
    /*
        I use cfmakeraw()to set automatically the flags insted of doing:

        tio.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR | IGNCR);
        tio.c_oflag &= ~(OPOST | ONLCR);
        tio.c_lflag &= ~(ECHO | ECHOE | ICANON | ISIG);          
    */
    cfmakeraw(&tio);
    cfsetispeed(&tio, baud);
    cfsetospeed(&tio, baud);

    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 5;

    // Saving settings
    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        LOG_ERR("Error setting tty");
        return false;
    }
    return true;
}

void ELM327Transport::sendRaw(const std::string& cmd) {
    write(fd, cmd.c_str(), cmd.size());
}

std::string read_until_prompt(int fd, int timeoutSeconds) {
    std::string out;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSeconds);

    char buf[256];
    while (std::chrono::steady_clock::now() < deadline) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            out.append(buf, buf + n);
            if (out.find('>') != std::string::npos) break;
        }
    }
    return out;
}

bool ELM327Transport::init(const std::string& serialPort) {

    fd = open(serialPort.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        LOG_ERR(("open " + serialPort).c_str());
        return false;
    }

    if (!set_raw_mode(fd, B38400)) {
        close(fd);
        fd = -1;
        return false;
    }

    usleep(300);

    sendRaw("ATZ\r");   // reset
    std::string r = read_until_prompt(2);
    LOG_INFO("ATZ -> " + r);

    sendRaw("ATE0\r");  // disable echo (prevents duplicates)
    r = read_until_prompt(1);

    sendRaw("ATSP0\r"); // automatic protocol
    r = read_until_prompt(1);

    sendRaw("AT SH " + to_upper_hex(broadcast_id) +"\r"); // set id
    r = read_until_prompt(1);

    sendRaw("AT CAF0\r");   // set raw mode once
    r = read_until_prompt(1);

    std::string leftover = read_until_prompt(1);
    (void)leftover;

    return true;
}

void ELM327Transport::closePort() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

bool ELM327Transport::send(const can_frame &frame) {
    if (fd < 0) return false;


    std::string r;
    if (frame.can_id != broadcast_id) {
        std::string sh = "AT SH " + to_upper_hex(frame.can_id) + "\r";
        sendRaw(sh);
        r = read_until_prompt(1);
    }

    std::ostringstream payload;
    payload << std::uppercase << std::hex << std::setfill('0');
    for (int i = 0; i < frame.can_dlc; ++i) {
        if (i) payload << ' ';
        payload << std::setw(2) << (static_cast<int>(frame.data[i]) & 0xFF);
    }
    payload << "\r";
    sendRaw(payload.str());

    r = read_until_prompt(2);
    (void)r;

    return true;
}

bool ELM327Transport::readFullResponse(std::vector<uint8_t>& outPayload) {
    outPayload.clear();
    if (fd < 0) return false;

    std::string raw = read_until_prompt((timeoutMs + 999)/1000); // timeout in seconds
    if (raw.empty()) return false;

    raw.erase(std::remove(raw.begin(), raw.end(), '>'), raw.end());

    for (char &c : raw) {
        if (c == '\r' || c == '\n') c = ' ';
    }

    std::istringstream iss(raw);
    std::string tok;
    while (iss >> tok) {

        size_t start = 0;
        while (start < tok.size() && !isxdigit((unsigned char)tok[start])) ++start;
        size_t end = tok.size();
        while (end > start && !isxdigit((unsigned char)tok[end-1])) --end;
        if (start >= end) continue;
        std::string hexs = tok.substr(start, end - start);

        bool allhex = true;
        for (char c : hexs) if (!isxdigit((unsigned char)c)) { allhex = false; break; }
        if (!allhex) continue;

        if (hexs.size() == 2) {
            try {
                int v = std::stoi(hexs, nullptr, 16);
                outPayload.push_back(static_cast<uint8_t>(v & 0xFF));
            } catch (...) { continue; }
        } else if (hexs.size() > 2) {
            if (hexs.size() == 3) continue;
            if ((hexs.size() % 2) == 0) {
                for (size_t i = 0; i < hexs.size(); i += 2) {
                    std::string pair = hexs.substr(i, 2);
                    try {
                        int v = std::stoi(pair, nullptr, 16);
                        outPayload.push_back(static_cast<uint8_t>(v & 0xFF));
                    } catch (...) {}
                }
            } else {
                continue;
            }
        } else if (hexs.size() == 1) {
            try {
                int v = std::stoi(hexs, nullptr, 16);
                outPayload.push_back(static_cast<uint8_t>(v & 0xFF));
            } catch (...) { continue; }
        }
    }

    return !outPayload.empty();
}

void ELM327Transport::splitPayloadIntoFrames(const std::vector<uint8_t>& payload) {

    frameQueue = {};

    size_t totalLength = payload.size();
    size_t pos = 0;
    uint8_t seq = 1;

    if (totalLength == 0) return;

    if (totalLength <= 7) {
        can_frame frame{};
        frame.can_id = 0x7E8;
        frame.can_dlc = static_cast<__u8>(totalLength + 1);
        frame.data[0] = static_cast<uint8_t>(totalLength & 0xFF);
        std::memcpy(frame.data + 1, payload.data(), totalLength);
        frameQueue.push(frame);
    } else {
        // First frame 
        can_frame firstFrame{};
        firstFrame.can_id = 0x7E8;
        firstFrame.can_dlc = 8;
        firstFrame.data[0] = static_cast<uint8_t>(0x10 | ((totalLength >> 8) & 0x0F));
        firstFrame.data[1] = static_cast<uint8_t>(totalLength & 0xFF);
        size_t firstChunk = std::min<size_t>(6, totalLength);
        std::memcpy(firstFrame.data + 2, payload.data(), firstChunk);
        if (firstChunk < 6) {
            std::memset(firstFrame.data + 2 + firstChunk, 0x00, 6 - firstChunk);
        }
        frameQueue.push(firstFrame);
        pos = firstChunk;

        // Consecutive frames
        while (pos < totalLength) {
            can_frame cf{};
            cf.can_id = 0x7E8;
            size_t chunkSize = std::min<size_t>(7, totalLength - pos);
            cf.data[0] = static_cast<uint8_t>(0x20 | (seq & 0x0F));
            std::memcpy(cf.data + 1, payload.data() + pos, chunkSize);
            cf.can_dlc = static_cast<__u8>(chunkSize + 1);
            if (chunkSize < 7) std::memset(cf.data + 1 + chunkSize, 0x00, 7 - chunkSize);
            frameQueue.push(cf);
            pos += chunkSize;
            seq = (seq + 1) & 0x0F;
            if (seq == 0) seq = 1;
        }
    }
}

bool ELM327Transport::receive(can_frame& frame) {

    if (!frameQueue.empty()) {
        frame = frameQueue.front();
        frameQueue.pop();
        return true;
    }

    std::vector<uint8_t> payload;
    if (!readFullResponse(payload)) return false;

    splitPayloadIntoFrames(payload);

    if (!frameQueue.empty()) {
        frame = frameQueue.front();
        frameQueue.pop();
        LOG_INFO("Frame: " + std::to_string(frame.data[0]) + " "
                       + std::to_string(frame.data[1]) + " "
                       + std::to_string(frame.data[2]) + " "
                       + std::to_string(frame.data[3]) + " "
                       + std::to_string(frame.data[4]) + " "
                       + std::to_string(frame.data[5]) + " "
                       + std::to_string(frame.data[6]) + " "
                       + std::to_string(frame.data[7]));
        return true;
    }
    return false;
}


#endif
