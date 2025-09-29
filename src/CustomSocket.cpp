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

    setTimeout(500);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }
    struct timeval tv;
    tv.tv_sec  = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return true;
}

void SocketCAN::setTimeout(int timeoutMs) {
    this->timeoutMs = timeoutMs;
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
    if (frame.data[1] == 0x7F) {
        return true;
    } else if (frame.data[0] != 0x10 && frame.data[1] < 0x4B && frame.data[1] > 0x40) {
        return true;
    } else if( frame.data[0] == 0x10 && frame.data[2] < 0x4B && frame.data[2] > 0x40) {
        return true;
    } else if (frame.data[0] > 0x19 && frame.data[0] < 0x30) {
        return true;
    }
    return false;
}

char SocketCAN::getProtocol() {
    // TODO
    return '6';
}

ELM327Transport::ELM327Transport(speed_t baudRate, char protocol) : baudRate(baudRate), protocol(protocol) {}

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
    tio.c_cc[VTIME] = timeoutMs / 100;

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

std::string ELM327Transport::read_until_prompt(int timeoutMiliSeconds) {
    std::string out;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMiliSeconds);
    char buf[128];

    while (std::chrono::steady_clock::now() < deadline) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            out.append(buf, buf + n);
            deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMiliSeconds);
            if (out.find('>') != std::string::npos) break;
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    return out;
}

bool ELM327Transport::initChip() {

    sendRaw("ATZ\r");   // reset
    std::string r = read_until_prompt(5000);
    LOG_INFO("ATZ -> " + r);
    usleep(1000000);


    sendRaw("ATE0\r");  // disable echo (prevents duplicates)
    r = read_until_prompt(5000);
    LOG_INFO("ATE0 -> " + r);
    usleep(1000000);


    sendRaw("ATL0\r");  // disable new line
    r = read_until_prompt(5000);
    LOG_INFO("ATL0 -> " + r);
    usleep(1000000);

    std::string command = "ATSP" + std::string(1, protocol);
    sendRaw(command + "\r"); // set protocol
    r = read_until_prompt(5000);
    LOG_INFO(command + " -> " + r);
    usleep(1000000);

    sendRaw("ATSWAA\r");  // set wakeup
    r = read_until_prompt(5000);
    LOG_INFO("ATSWAA -> " + r);
    usleep(1000000);

    /* 
        TODO: Need to activate raw mode and   AT CFC0 to manage manually multi frame.
        Also, change receive frame, pci and id will be received
    */
    // sendRaw("AT SH " + to_upper_hex(broadcast_id) +"\r"); // set id for broadcast
    // r = read_until_prompt(1);

    // sendRaw("AT CAF0\r");   // set raw mode once
    // r = read_until_prompt(1);

    // sendRaw("AT CFC0\r");   // set flow controll on
    // r = read_until_prompt(1);

    bool protocolFound = false;
    for (int i = 0; i < 7; i++) {
        usleep(2000000);
        sendRaw("0100\r");
        std::string resp = read_until_prompt(3000);
        LOG_INFO("Test PID -> " + resp);

        if (resp.find("41 00") != std::string::npos) {
            LOG_INFO("Protocol detected and working");
            protocolFound = true;
            break;
        }
    }

    if (!protocolFound) {
        LOG_ERR("No protocol detected");
        return false;
    }
    return true;
}


bool ELM327Transport::init(const std::string& serialPort) {

    fd = open(serialPort.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        LOG_ERR(("open " + serialPort).c_str());
        return false;
    }

    if (!set_raw_mode(fd, baudRate)) {
        close(fd);
        fd = -1;
        return false;
    }

    usleep(2000000);
    tcflush(fd, TCIFLUSH);

    bool initialized = false;

    // Change here max value to set max no. of attemps
    for (int i = 0; i < 2; i++) {
        if (initChip()) {
            initialized = true;
            break;
        }
    }

    return initialized;
}

void ELM327Transport::closePort() {
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

bool ELM327Transport::send(const can_frame &frame) {

    std::lock_guard<std::mutex> lock(ioMutex);

    if (fd < 0) return false;

    std::ostringstream payload;
    payload << std::uppercase << std::hex << std::setfill('0');
    for (int i = 1; i <= frame.data[0]; ++i) {
        payload << std::setw(2) << (static_cast<int>(frame.data[i]) & 0xFF);
    }
    
    payload << "\r";
    LOG_INFO(payload.str());
    sendRaw(payload.str());

    return true;
}

bool ELM327Transport::readFullResponse(std::vector<uint8_t>& outPayload) {
    outPayload.clear();
    if (fd < 0) return false;

    std::string raw = read_until_prompt(timeoutMs);
    if (raw.empty()) return false;

    raw.erase(std::remove(raw.begin(), raw.end(), '>'), raw.end());

    for (char &c : raw) {
        if (c == '\r' || c == '\n') c = ' ';
    }

    std::istringstream iss(raw);
    std::string tok;
    while (iss >> tok) {

        size_t start = 0;
        // Skip tokens that are just frame numbers like "0:" or "1:"
        if (tok.size() == 2 && isdigit(tok[0]) && tok[1] == ':') {
            continue;
        }
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
        // Single Frame
        can_frame frame{};
        frame.can_id = 0x7E0; // send to ECU
        frame.can_dlc = static_cast<uint8_t>(totalLength + 1);
        frame.data[0] = static_cast<uint8_t>(0x00 | (totalLength & 0x0F)); // SF PCI
        std::memcpy(frame.data + 1, payload.data(), totalLength);
        frameQueue.push(frame);
    } else {
        // First Frame
        can_frame firstFrame{};
        firstFrame.can_id = 0x7E0; // send to ECU
        firstFrame.can_dlc = 8;
        firstFrame.data[0] = static_cast<uint8_t>(0x10 | ((totalLength >> 8) & 0x0F)); // FF PCI
        firstFrame.data[1] = static_cast<uint8_t>(totalLength & 0xFF);
        size_t firstChunk = 6;
        std::memcpy(firstFrame.data + 2, payload.data(), firstChunk);
        frameQueue.push(firstFrame);
        pos = firstChunk;

        // Consecutive frames
        while (pos < totalLength) {
            can_frame cf{};
            cf.can_id = 0x7E0; // send to ECU
            size_t chunkSize = std::min<size_t>(7, totalLength - pos);
            cf.data[0] = static_cast<uint8_t>(0x20 | (seq & 0x0F)); // CF PCI
            std::memcpy(cf.data + 1, payload.data() + pos, chunkSize);
            cf.can_dlc = static_cast<uint8_t>(chunkSize + 1);
            if (chunkSize < 7) std::memset(cf.data + 1 + chunkSize, 0x00, 7 - chunkSize);
            frameQueue.push(cf);
            pos += chunkSize;
            seq = (seq + 1) & 0x0F;
            if (seq == 0) seq = 1;
        }
    }
}


bool ELM327Transport::receive(can_frame& frame) {

    std::lock_guard<std::mutex> lock(ioMutex);

    if (!frameQueue.empty()) {
        frame = frameQueue.front();
        frameQueue.pop();
        return true;
    }

    std::vector<uint8_t> payload;
    if (!readFullResponse(payload)) return false;

    std::string data = "";
    for (auto hex: payload) {
        data += std::to_string(hex) + " ";
    }

    splitPayloadIntoFrames(payload);

    if (!frameQueue.empty()) {
        frame = frameQueue.front();
        frameQueue.pop();
        LOG_INFO("Frame: " + data);
        return true;
    }
    return false;
}

char ELM327Transport::getProtocol() {
    sendRaw("ATDPN");
    char res = read_until_prompt(timeoutMs)[0];
    return res;
}


#endif
