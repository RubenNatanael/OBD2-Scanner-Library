#ifndef OBD2_SCANNER
#define OBD2_SCANNER

#include <chrono>

#include "MultiLogger.h"
#include "CustomSocket.h"
#include "GenerateFrame.h"
#include "ReceiverFrames.h"

#define PID_SUPPORTED 0x00

class OBD2Scanner {

    ICANInterface* transport;
    GenerateFrame* generator;
    ReceiverFrames* receiver;

    public:
        OBD2Scanner(std::string interfaceName);
        std::vector<DecodedItem> getPid(uint8_t pid);
        std::vector<DecodedItem> getDTCs();
        std::vector<DecodedItem> getFreezFrame(uint8_t pid);
        std::vector<DecodedItem> getPermanentDTCs();
        std::vector<DecodedItem> ClearDTCs();
        
        ~OBD2Scanner();
};

#endif