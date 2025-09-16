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
        perror("tcgetattr");
        return false;
    }
    cfmakeraw(&tio);                    // disables canonical, echo, signals
    cfsetispeed(&tio, baud);
    cfsetospeed(&tio, baud);

    // Disable software flow control and other mappings
    tio.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR | IGNCR);
    // Disable output post processing
    tio.c_oflag &= ~(OPOST | ONLCR);
    // Raw: no echo, no signs
    tio.c_lflag &= ~(ECHO | ECHOE | ICANON | ISIG);
    // Blocking read behavior: return as soon as data available or timeout
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 5; // 0.5s read timeout

    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        perror("tcsetattr");
        return false;
    }
    return true;
}

ssize_t ELM327Transport::write_all(const std::string &s) {
    if (fd < 0) return -1;
    const char *p = s.c_str();
    size_t left = s.size();
    while (left) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("write");
            return -1;
        }
        left -= n;
        p += n;
    }
    // flush output driver (drain)
    tcdrain(fd);
    return (ssize_t)s.size();
}

void ELM327Transport::sendRaw(const std::string& cmd) {
    write_all(cmd);
}

// Read until '>' prompt or timeout (timeoutSeconds)
std::string ELM327Transport::read_until_prompt(int timeoutSeconds) {
    std::string out;
    if (fd < 0) return out;

    fd_set rf;
    struct timeval tv;
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSeconds);

    while (std::chrono::steady_clock::now() < deadline) {
        FD_ZERO(&rf);
        FD_SET(fd, &rf);
        tv.tv_sec = 0;
        tv.tv_usec = 200000; // 200ms
        int r = select(fd + 1, &rf, nullptr, nullptr, &tv);
        if (r > 0 && FD_ISSET(fd, &rf)) {
            char buf[256];
            ssize_t n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                out.append(buf, buf + n);
                // break if prompt char appears
                if (out.find('>') != std::string::npos) break;
            } else if (n == 0) {
                // no data; continue polling
                continue;
            } else if (errno != EINTR && errno != EAGAIN) {
                break;
            }
        }
    }
    return out;
}

bool ELM327Transport::init(const std::string& serialPort) {
    // open serial device (non-blocking not needed; we'll use select/timeouts)
    fd = open(serialPort.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror(("open " + serialPort).c_str());
        return false;
    }

    // default baud 38400 (as your original code suggested)
    if (!set_raw_mode(fd, B38400)) {
        close(fd);
        fd = -1;
        return false;
    }

    // small delay to let device come up
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Reset and basic setup (we'll read responses to consume prompt)
    sendRaw("ATZ\r");                      // reset
    std::string r = read_until_prompt(2);
    LOG_INFO("ATZ -> " + r);

    sendRaw("ATE0\r");                     // disable echo (prevents duplicates)
    r = read_until_prompt(1);

    sendRaw("ATSP0\r");                    // automatic protocol
    r = read_until_prompt(1);

    // Clear any buffered output until prompt
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

// Prepare and send CAN frame via ELM327: set header then send raw bytes
bool ELM327Transport::send(const can_frame &frame) {
    if (fd < 0) return false;

    // 1) set header to the target ID (ELM expects hex, uppercase)
    unsigned int id = (frame.can_id & CAN_EFF_FLAG) ? (frame.can_id & CAN_EFF_MASK) : (frame.can_id & CAN_SFF_MASK);
    std::string sh = "AT SH " + to_upper_hex(id) + "\r";
    //sendRaw(sh);
    //std::string r ;//= read_until_prompt(1);

    // 2) optionally ensure CAF0 (no auto-format) - do once, but safe to repeat
    //sendRaw("AT CAF0\r");
    //r = read_until_prompt(1);

    // 3) send data bytes as space-separated hex + CR
    std::ostringstream payload;
    payload << std::uppercase << std::hex << std::setfill('0');
    for (int i = 0; i < frame.can_dlc; ++i) {
        if (i) payload << ' ';
        payload << std::setw(2) << (static_cast<int>(frame.data[i]) & 0xFF);
    }
    payload << "\r";
    sendRaw(payload.str());

    // read immediate response (ELM327 will respond with OK or the bus reply)
    //r = read_until_prompt(2);
    //(void)r; // you can log or parse if needed

    return true;
}

bool ELM327Transport::readFullResponse(std::vector<uint8_t>& outPayload) {
    outPayload.clear();
    if (fd < 0) return false;

    // Read until '>' prompt (ELM end marker). We'll normalize CR/LF -> spaces,
    // then extract hex byte tokens while skipping obvious non-byte tokens (IDs, words).
    std::string raw = read_until_prompt((timeoutMs + 999)/1000); // timeout in seconds
    if (raw.empty()) return false;

    // Remove the '>' prompt from raw if present
    raw.erase(std::remove(raw.begin(), raw.end(), '>'), raw.end());

    // Replace CR/LF with space to normalize tokens
    for (char &c : raw) {
        if (c == '\r' || c == '\n') c = ' ';
    }

    // Tokenize by whitespace and try to extract bytes.
    // Heuristics:
    //  - tokens of length 2 and hex => byte
    //  - tokens of even length >2 and all-hex => split into byte pairs
    //  - tokens of length 3 are likely IDs (e.g., "7E8") -> skip
    //  - tokens with non-hex chars -> skip
    std::istringstream iss(raw);
    std::string tok;
    while (iss >> tok) {
        // strip any non-hex prefix/suffix (defensive)
        size_t start = 0;
        while (start < tok.size() && !isxdigit((unsigned char)tok[start])) ++start;
        size_t end = tok.size();
        while (end > start && !isxdigit((unsigned char)tok[end-1])) --end;
        if (start >= end) continue;
        std::string hexs = tok.substr(start, end - start);

        // if token contains any non-hex, skip
        bool allhex = true;
        for (char c : hexs) if (!isxdigit((unsigned char)c)) { allhex = false; break; }
        if (!allhex) continue;

        if (hexs.size() == 2) {
            // standard single-byte token
            try {
                int v = std::stoi(hexs, nullptr, 16);
                outPayload.push_back(static_cast<uint8_t>(v & 0xFF));
            } catch (...) { continue; }
        } else if (hexs.size() > 2) {
            // If it's 3 chars - likely an ID (skip)
            if (hexs.size() == 3) continue;
            // if even length, split into byte pairs
            if ((hexs.size() % 2) == 0) {
                for (size_t i = 0; i < hexs.size(); i += 2) {
                    std::string pair = hexs.substr(i, 2);
                    try {
                        int v = std::stoi(pair, nullptr, 16);
                        outPayload.push_back(static_cast<uint8_t>(v & 0xFF));
                    } catch (...) { /* ignore */ }
                }
            } else {
                // odd-length hex token that's >2 (rare). Skip it.
                continue;
            }
        } else if (hexs.size() == 1) {
            // single hex nibble alone — not expected as isolated token; treat as byte value
            try {
                int v = std::stoi(hexs, nullptr, 16);
                outPayload.push_back(static_cast<uint8_t>(v & 0xFF));
            } catch (...) { continue; }
        }
    }

    return !outPayload.empty();
}

void ELM327Transport::splitPayloadIntoFrames(const std::vector<uint8_t>& payload) {
    // Create ISO-TP style frames (virtual) from payload bytes and queue them
    frameQueue = {}; // clear

    size_t totalLength = payload.size();
    size_t pos = 0;
    uint8_t seq = 1;

    if (totalLength == 0) return;

    if (totalLength <= 7) {
        // Single frame: PCI = length
        can_frame frame{};
        frame.can_id = 0x7E8; // response ID (you may want to parametrize)
        frame.can_dlc = static_cast<__u8>(totalLength + 1);
        frame.data[0] = static_cast<uint8_t>(totalLength & 0xFF); // PCI for single frame
        std::memcpy(frame.data + 1, payload.data(), totalLength);
        frameQueue.push(frame);
    } else {
        // First frame (PCI 0x10, two-byte length)
        can_frame firstFrame{};
        firstFrame.can_id = 0x7E8;
        firstFrame.can_dlc = 8;
        firstFrame.data[0] = static_cast<uint8_t>(0x10 | ((totalLength >> 8) & 0x0F));
        firstFrame.data[1] = static_cast<uint8_t>(totalLength & 0xFF);
        // copy up to 6 bytes into first frame
        size_t firstChunk = std::min<size_t>(6, totalLength);
        std::memcpy(firstFrame.data + 2, payload.data(), firstChunk);
        // if less than 6 copied, pad remaining bytes (not strictly necessary)
        if (firstChunk < 6) {
            std::memset(firstFrame.data + 2 + firstChunk, 0x00, 6 - firstChunk);
        }
        frameQueue.push(firstFrame);
        pos = firstChunk;

        // Consecutive frames: PCI 0x2n and up to 7 bytes each
        while (pos < totalLength) {
            can_frame cf{};
            cf.can_id = 0x7E8;
            size_t chunkSize = std::min<size_t>(7, totalLength - pos);
            cf.data[0] = static_cast<uint8_t>(0x20 | (seq & 0x0F));
            std::memcpy(cf.data + 1, payload.data() + pos, chunkSize);
            cf.can_dlc = static_cast<__u8>(chunkSize + 1);
            // pad leftover bytes to zero for clarity (not required)
            if (chunkSize < 7) std::memset(cf.data + 1 + chunkSize, 0x00, 7 - chunkSize);
            frameQueue.push(cf);
            pos += chunkSize;
            seq = (seq + 1) & 0x0F;
            if (seq == 0) seq = 1; // avoid zero sequence if you prefer
        }
    }
}

bool ELM327Transport::receive(can_frame& frame) {
    // If queued frames exist, return next
    if (!frameQueue.empty()) {
        frame = frameQueue.front();
        frameQueue.pop();
        return true;
    }

    // Otherwise, read a full response and split into frames
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
