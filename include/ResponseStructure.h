#ifndef RESPONSE_STRUCTURE
#define RESPONSE_STRUCTURE

#include <string>
#include <stdint.h>

struct DecodedItem {
    uint8_t mode = 0x01;
    uint8_t pid;
    std::string label;
    std::string value;
    bool valid = true;
};

using DecoderFunc = std::vector<DecodedItem> (*)(const uint8_t* data, uint8_t len);

struct PIDEntry {
    uint8_t pid;
    DecoderFunc decoder;
};

#endif