
#include "../include/ReceiverFrames.h"

std::vector<DecodedItem> Mode1::Decodify(can_frame &frame) {
    uint8_t pid = frame.data[1];
    uint8_t* data = frame.data + 2;
    uint8_t len = frame.len - 2;

    for (size_t i = 0; i < pidTableSize; ++i) {
        if (pidTable[i].pid == pid) {
            return pidTable[i].decoder(data, len);
        }
    }

    // Unknown PID
    std::vector<DecodedItem> r;
    r[0].label = "Erro: Not found";
    r[0].value = "404";
    return r;
}

