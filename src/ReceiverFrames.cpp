
#include "../include/ReceiverFrames.h"

/*  ______________________________________
    |_Byte_|_____0__1__2__3__4__5__6__7__|
    | Frame| 7E8#03 41 0D 12 AA AA AA AA |
*/
std::vector<DecodedItem> Mode1::Decodify(uint8_t* data, uint8_t len) {
    uint8_t pid = data[2];
    uint8_t* new_data = data + 3;
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
std::vector<DecodedItem> Mode3::Decodify(uint8_t* data, uint8_t len) {
    
    uint8_t* encodedDtc;
    std::vector<DecodedItem> r;

    if (len == 2) {
        r[0].label = "No DTCs founded";
        r[0].value = "0";
        return r;
    }

    uint8_t *dtcs = data + 2;
    uint8_t newLength = len - 2;

    for (int i = 0; i < len / 2; i++) {
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
std::vector<DecodedItem> Mode4::Decodify(uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> r;
    
    if (data[1] == 0x44) {
        r[0].label = "All DTCs deleted";
        r[0].value = "0";
    } else {
        r[0].label = "Error";
        r[0].value = "1";
    }
    
    return r;
}
