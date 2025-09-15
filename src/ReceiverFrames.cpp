
#include "../include/ReceiverFrames.h"

ReceiverFrames::ReceiverFrames(ICANInterface &receiver) : r(receiver) {}

IObd2Modes* ReceiverFrames::ReceiveFrames() {
    if (!this->readAndAssembleFrames()) {
        return nullptr;
    }

    uint8_t byteMode = responseBuffer[0];
    switch (byteMode) {
        case 0x41: currentMode = &mode1; LOG_INFO("Mode1 recognized"); break;
        case 0x42: currentMode = &mode2; LOG_INFO("Mode2 recognized"); break;
        case 0x43: currentMode = &mode3; LOG_INFO("Mode3 recognized"); break;
        case 0x47: currentMode = &mode7; LOG_INFO("Mode7 recognized"); break;
        case 0x4A: currentMode = &modeA; LOG_INFO("Mode0A recognized"); break;
        case 0x44: currentMode = &mode4; LOG_INFO("Mode4 recognized"); break;
        default: currentMode = &modeDefault; LOG_WARN("Can frame is not an OBD2 message"); break;
    }
    if (currentMode != &modeDefault) {
        currentMode->setReceivedBytes(totalLength);
        currentMode->setResponseBuffer(responseBuffer);
    }
    return currentMode;
}

void IObd2Modes::setReceivedBytes(uint8_t receivedBytes) {
    this->receivedBytes = receivedBytes;
}

void IObd2Modes::setResponseBuffer(uint8_t* responseBuffer) {
    this->responseBuffer = responseBuffer;
}

bool ReceiverFrames::readAndAssembleFrames() {
    receivedBytes = 0;
    totalLength = 0;
    can_frame flowControl;
    flowControl.can_dlc = 8;
    memset(flowControl.data, 0, 8);
    flowControl.data[0] = 0x30;

    r.setTimeout(500);

    bool isMultipleFrame = false;

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);

    __u8* temp_responseBuffer = responseBuffer + 1;
    uint8_t mode = 0;

    while (std::chrono::steady_clock::now() < deadline) {

        can_frame frame;
        if (!r.receive(frame)) {
            if (!mode) LOG_WARN("Timeout, frame no received");
            break;
        }

        uint8_t pci = frame.data[0];
        uint8_t frameType = (pci >> 4) & 0x0F;

        if (frameType == 0x0 && (mode == 0 || mode == frame.data[1])) {
            totalLength += pci - 1;
            memcpy(temp_responseBuffer + receivedBytes, &frame.data[2], totalLength);
            responseBuffer[0] = frame.data[1];
            receivedBytes += totalLength;
            LOG_INFO("Frame received");
            mode = frame.data[1];
        }

        else if (frameType == 0x1 && (mode == 0 || mode == frame.data[2])) {
            totalLength += (((pci & 0x0F) << 8) | frame.data[1]) -1;
            memcpy(temp_responseBuffer + receivedBytes, &frame.data[3], 5);
            responseBuffer[0] = frame.data[2];
            receivedBytes = 5;
            flowControl.can_id = frame.can_id;
            isMultipleFrame = true;
            mode = frame.data[2];
            r.send(flowControl);
        }

        else if (frameType == 0x2 && isMultipleFrame) {
            memcpy(temp_responseBuffer + receivedBytes, &frame.data[1], 7);
            receivedBytes += 7;

            if (receivedBytes >= totalLength) {
                LOG_INFO("All frames received");
                isMultipleFrame = false;
            }
        }
    }

    totalLength += 1;
    if (mode) return true;

    return false;
}

/*  ______________________________________
    |_Byte_|_____0__1__2__3__4__5__6__7__|
    | Frame| 7E8#03 41 0D 12 AA AA AA AA |
*/
std::vector<DecodedItem> Mode1::Decodify() {
    uint8_t pid = responseBuffer[1];
    uint8_t* new_data = responseBuffer + 2;
    uint8_t len = receivedBytes - 2;

    const PIDEntry* pidTable = Mode1Pid().getTable();
    uint8_t pidTableSize = Mode1Pid().pidTableSize;

    for (size_t i = 0; i < pidTableSize; ++i) {
        if (pidTable[i].pid == pid) {
            return pidTable[i].decoder(new_data, len);
        }
    }

    // Unknown PID
    std::vector<DecodedItem> r;
    r.push_back({"Erro: Not found", "1"});
    return r;
}

/*  ______________________________________
    |_Byte_|_____0__1__2__3__4__5__6__7__|
    | Frame| 7E8#03 43 C1 58 AA AA AA AA |
*/
std::vector<DecodedItem> Mode3::Decodify() {
    
    std::vector<DecodedItem> r;

    if (receivedBytes == 1) {
        r.push_back({"No DTCs founded","0"});
        return r;
    }

    uint8_t *dtcs = responseBuffer + 1;
    uint8_t newLength = receivedBytes - 1;

    uint8_t encodedDtc[2];


    for (int i = 0; i < newLength / 2; i++) {
        encodedDtc[0] = dtcs[i * 2];
        encodedDtc[1] = dtcs[(i * 2) + 1];
        std::string dtc = DecodifyDTC(encodedDtc);
        r.push_back({"DTC: ",Parse(dtc)});
    }
    
    if (r.empty()) {
        r.push_back({"Erro: Not found","1"});
    }

    return r;
}

std::string Mode3::DecodifyDTC(uint8_t *data) {

    uint8_t byte0 = data[0];
    uint8_t byte1 = data[1];

    // Letter
    char letter;
    switch ((byte0 >> 6) & 0x03) {
        case 0: letter = 'P'; break;
        case 1: letter = 'C'; break;
        case 2: letter = 'B'; break;
        case 3: letter = 'U'; break;
    }

    // Digits
    uint8_t digit1 = (byte0 >> 4) & 0x03;
    uint8_t digit2 = byte0 & 0x0F;
    uint8_t digit3 = (byte1 >> 4) & 0x0F;
    uint8_t digit4 = byte1 & 0x0F;

    char buf[6];
    snprintf(buf, sizeof(buf), "%1X%1X%1X%1X", digit1, digit2, digit3, digit4);
    std::string DTC = letter + std::string(buf);
    return DTC;

}

/*  ______________________________________
    |_Byte_|_____0__1__2__3__4__5__6__7__|
    | Frame| 7E8#01 44 00 00 AA AA AA AA |
*/
std::vector<DecodedItem> Mode4::Decodify() {
    std::vector<DecodedItem> r;
    
    if (responseBuffer[0] == 0x44) {
        r.push_back({"All DTCs deleted", "0"});
    } else {
        r.push_back({"Error", "-1"});
    }
    
    return r;
}

std::vector<DecodedItem> ModeDefault::Decodify() {
    std::vector<DecodedItem> r;
    
    r.push_back({"", ""});
    
    return r;
}

bool IObd2Modes::ContainsPid(uint8_t pid) {
    return responseBuffer[2] == pid;
}

