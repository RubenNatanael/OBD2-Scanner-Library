
#include "../include/ReceiverFrames.h"

ReceiverFrames::ReceiverFrames(ICANInterface &receiver) : r(receiver) {}

IObd2Modes* ReceiverFrames::ReceiveFrames() {
    if (!this->readAndAssembleFrames()) {
        return nullptr;
    }

    uint8_t byteMode = responseBuffer[0];
    switch (byteMode) {
        case 1: case 2: currentMode = &mode1; break;
        case 3: case 7: case 0x0A: currentMode = &mode3; break;
        case 4: currentMode = &mode4; break;
        default: currentMode = nullptr; break;
    }
    if (currentMode) {
        currentMode->setReceivedBytes(receivedBytes);
        currentMode->setResponseBuffer(responseBuffer);
    }
    return currentMode;
}

bool ReceiverFrames::readAndAssembleFrames() {
    receivedBytes = 0;
    totalLength = 0;
    can_frame flowControl;
    flowControl.can_id = 0x777;
    flowControl.can_dlc = 4;
    flowControl.data[0] = 0x30;
    flowControl.data[1] = 0x00;
    flowControl.data[2] = 0x00;
    flowControl.data[3] = 0x00;

    r.setTimeout(2000);
    while (true) {

        can_frame frame;
        if (!r.receive(frame)) {
            return false;
        }

        uint8_t pci = frame.data[0];
        uint8_t frameType = pci >> 4;

        if (frameType == 0x0) {
            totalLength = pci & 0x0F;
            memcpy(responseBuffer, &frame.data[1], totalLength);
            receivedBytes = totalLength;
            return true;
        }

        else if (frameType == 0x1) {
            totalLength = ((pci & 0x0F) << 8) | frame.data[1];
            memcpy(responseBuffer, &frame.data[2], 6);
            receivedBytes = 6;

            r.send(flowControl);
        }

        else if (frameType == 0x2) {
            uint8_t length = pci & 0x0F;
            memcpy(responseBuffer + receivedBytes, &frame.data[1], length);
            receivedBytes += length;

            if (receivedBytes >= totalLength) {
                return true;
            }
        }
    }

    // Never reached, but safe
    return false;
}

/*  ______________________________________
    |_Byte_|_____0__1__2__3__4__5__6__7__|
    | Frame| 7E8#03 41 0D 12 AA AA AA AA |
*/
std::vector<DecodedItem> Mode1::Decodify() {
    uint8_t pid = responseBuffer[2];
    uint8_t* new_data = responseBuffer + 3;
    uint8_t len = len - 3;

    for (size_t i = 0; i < pidTableSize; ++i) {
        if (pidTable[i].pid == pid) {
            return pidTable[i].decoder(new_data, len);
        }
    }

    // Unknown PID
    std::vector<DecodedItem> r;
    r[0].label = "Erro: Not found";
    r[0].value = "1";
    return r;
}

/*  ______________________________________
    |_Byte_|_____0__1__2__3__4__5__6__7__|
    | Frame| 7E8#03 43 C1 58 AA AA AA AA |
*/
std::vector<DecodedItem> Mode3::Decodify() {
    
    uint8_t* encodedDtc;
    std::vector<DecodedItem> r;

    if (receivedBytes == 2) {
        r[0].label = "No DTCs founded";
        r[0].value = "0";
        return r;
    }

    uint8_t *dtcs = responseBuffer + 2;
    uint8_t newLength = receivedBytes- 2;

    for (int i = 0; i < receivedBytes / 2; i++) {
        encodedDtc[0] = dtcs[i * 2];
        encodedDtc[1] = dtcs[(i * 2) + 1];
        std::string dtc = DecodifyDTC(encodedDtc);
        r.push_back({"DTC: ",Parse(dtc)});
    }
    
    if (r.empty()) {
        r[0].label = "Erro: Not found";
        r[0].value = "1";
    }

    return r;
}

std::string Mode3::DecodifyDTC(uint8_t *data) {

    std::string DTC = "";
    uint8_t highBits = (data[0] >> 6) & 0x03;
    char letter;
    switch(highBits) {
        case 0: letter = 'P'; break;
        case 1: letter = 'C'; break;
        case 2: letter = 'B'; break;
        case 3: letter = 'U'; break;
        default: letter = '?';
    }

    uint8_t number1 = data[0] & 0x3f;
    uint8_t number2 = data[1];
    int numericValue = number1 << 8 | number2;

    char buf[5];
    snprintf(buf, sizeof(buf),"%04d", numericValue);
    DTC = letter + std::string(buf);
    return DTC;
}

/*  ______________________________________
    |_Byte_|_____0__1__2__3__4__5__6__7__|
    | Frame| 7E8#01 44 00 00 AA AA AA AA |
*/
std::vector<DecodedItem> Mode4::Decodify() {
    std::vector<DecodedItem> r;
    
    if (responseBuffer[1] == 0x44) {
        r[0].label = "All DTCs deleted";
        r[0].value = "0";
    } else {
        r[0].label = "Error";
        r[0].value = "1";
    }
    
    return r;
}
