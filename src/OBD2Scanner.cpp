#include "../include/OBD2Scanner.h"

OBD2Scanner::OBD2Scanner(ICANInterface *transporter) {
    transport = transporter;
    generator = new GenerateFrame(*transport);
    receiver = new ReceiverFrames(*transport);
}

std::vector<DecodedItem> OBD2Scanner::getPid(uint8_t pid) {
    
    generator->ShowCurrentData(pid);
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(2000);
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

void OBD2Scanner::getPid(uint8_t pid, std::function<void(const std::vector<DecodedItem>&)> func) {
    
    generator->ShowCurrentData(pid);
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(2000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode1) {
            if (mode->ContainsPid(pid)) {
                func(mode->Decodify());
                return;
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
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

void OBD2Scanner::getDTCs(std::function<void(const std::vector<DecodedItem>&)> callback) {
    generator->ShowDTCs();
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode3) {
                callback(mode->Decodify());
                return;
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
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

void OBD2Scanner::getFreezFrame(uint8_t pid, std::function<void(const std::vector<DecodedItem>&)> callback) {
    generator->ShowFreezFrameData(pid);
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode2) {
            if (mode->ContainsPid(pid)) {
                callback(mode->Decodify());
                return;
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
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

void OBD2Scanner::getPendingDTCs(std::function<void(const std::vector<DecodedItem>&)> callback) {
    generator->PendingDTCs();
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode7) {
                callback(mode->Decodify());
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
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

void OBD2Scanner::getPermanentDTCs(std::function<void(const std::vector<DecodedItem>&)> callback) {
    generator->PermanentDTCs();
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->modeA) {
                callback(mode->Decodify());
                return;
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
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

void OBD2Scanner::ClearDTCs(std::function<void(const std::vector<DecodedItem>&)> callback) {
    generator->ClearDTCs();
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    while (true)
    {
        IObd2Modes* mode = receiver->ReceiveFrames();
        if (mode == &receiver->mode4) {
                callback(mode->Decodify());
                return;
        }

        auto now = std::chrono::steady_clock::now();
        if (now - startTime >= timeout) {
            LOG_ERR("Timeout waiting for PID response");
            break;
        }
    }
}

OBD2Scanner::~OBD2Scanner() {
    delete receiver;
    delete generator;
    delete transport;
}
