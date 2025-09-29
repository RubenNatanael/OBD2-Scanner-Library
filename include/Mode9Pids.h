#ifndef MODE9_PIDS
#define MODE9_PIDS

#include <stdint.h>
#include <vector>
#include <string>
#include "TroubleCodes.h"
#include "ResponseStructure.h"

class Mode9Pid {
    private:
        uint8_t mode;

    public:
        // PID 0x00: Supported PIDs 01–20
        static std::vector<DecodedItem> DecodePID00(const uint8_t* data, uint8_t len);
        // PID 0x02: VIN
        static std::vector<DecodedItem> DecodePID02(const uint8_t* data, uint8_t len);
        
        static const PIDEntry* getTable();

        static const PIDEntry pidTable[];

        static const size_t pidTableSize;

};

#endif
