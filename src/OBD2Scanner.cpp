#include "../include/OBD2Scanner.h"

OBD2Scanner::OBD2Scanner(std::string interfaceName) {
    transport = new SocketCAN();
    transport->init(interfaceName);
    generator = new GenerateFrame(*transport);
    receiver = new ReceiverFrames(*transport);
}

std::vector<DecodedItem> OBD2Scanner::getPid(uint8_t pid) {
    
    generator->ShowCurrentData(pid);
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode1) {
            if (mode->ContainsPid(pid)) {
                return mode->Decodify();
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
    return {};
}

std::vector<DecodedItem> OBD2Scanner::getDTCs() {
    generator->ShowDTCs();
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode3) {
                return mode->Decodify();
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
    return {};
}

std::vector<DecodedItem> OBD2Scanner::getFreezFrame(uint8_t pid) {
    generator->ShowFreezFrameData(pid);
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode2) {
            if (mode->ContainsPid(pid)) {
                return mode->Decodify();
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
    return {};
}

std::vector<DecodedItem> OBD2Scanner::getPendingDTCs() {
    generator->PendingDTCs();
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode7) {
                return mode->Decodify();
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
    return {};
}

std::vector<DecodedItem> OBD2Scanner::getPermanentDTCs() {
    generator->PermanentDTCs();
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->modeA) {
                return mode->Decodify();
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
    return {};
}

std::vector<DecodedItem> OBD2Scanner::ClearDTCs() {
    generator->ClearDTCs();
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode4) {
                return mode->Decodify();
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
    return {};
}

OBD2Scanner::~OBD2Scanner() {
    delete receiver;
    delete generator;
    delete transport;
}
