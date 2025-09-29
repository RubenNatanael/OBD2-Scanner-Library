#include "../include/Mode9Pids.h"

const PIDEntry Mode9Pid::pidTable[] = {
    {0x00, &Mode9Pid::DecodePID00},
    {0x02, &Mode9Pid::DecodePID02},
};

const PIDEntry* Mode9Pid::getTable() {
    return pidTable;
}

// PID 0x00: Supported PIDs 01–20
std::vector<DecodedItem> Mode9Pid::DecodePID00(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 4) return result;

    for (int byteIndex = 0; byteIndex < len; ++byteIndex) {
        for (int bit = 7; bit >= 0; --bit) {
            if (data[byteIndex] & (1 << bit)) {
                result.push_back({
                    0x09,
                    0x00,
                    "Supported PID",
                    std::to_string(byteIndex*8 + (8-bit))
                });
            }
        }
    }
    return result;
}


// PID 0x02: DTC that causes freez frame
std::vector<DecodedItem> Mode9Pid::DecodePID02(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 17) return result;

    std::string vin = "";

    for (int i = 1; i < len; i++) {
        vin += static_cast<char>(data[i]);
    }
    result.push_back({0x09,0x02,"VIN", vin});
    return result;
}

const size_t Mode9Pid::pidTableSize = sizeof(pidTable)/sizeof(pidTable[0]);

