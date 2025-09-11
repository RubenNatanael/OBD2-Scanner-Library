#ifndef RECEIVER_FRAMES
#define RECEIVER_FRAMES

#include "CustomSocket.h"
#include "MultiLogger.h"
#include "Mode1Pids.h"
#include "TroubleCodes.h"

#ifdef ARDUINO

#include "ArduinoSTL.h"

#endif

#include <vector>
#include <utility>

class IObd2Modes {
    protected:
        uint8_t* responseBuffer;
        uint8_t receivedBytes = 0;
    public:
        virtual std::vector<DecodedItem> Decodify() = 0;
        void setResponseBuffer(uint8_t *r);
        void setReceivedBytes(uint8_t r);
        bool ContainsPid(uint8_t pid);
};

class Mode1: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify() override;

};

/* Mode 2 differs from Mode 1 only by the mode byte.
 To avoid duplicated code, it’s best to use class inheritance */
class Mode2: public Mode1 {
};

class Mode3: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify() override;
    private:
        std::string DecodifyDTC(uint8_t *dtc);

};

/* Mode 7 and 0A differs from Mode 3 only by the mode byte.
 To avoid duplicated code, it’s best to use class inheritance */
class Mode7: public Mode3 {
};

class ModeA: public Mode3 {
};

class Mode4: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify() override;

};

class ModeDefault: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify() override;

};

class ReceiverFrames {
public:
    ICANInterface &r;
    Mode1 mode1;
    Mode2 mode2;
    Mode3 mode3;
    Mode7 mode7;
    ModeA modeA;
    Mode4 mode4;
    ModeDefault modeDefault;
private:
    IObd2Modes* currentMode = nullptr;
    __u8 responseBuffer[64];
    uint8_t receivedBytes = 0;
    uint8_t totalLength = 0;

public:
    ReceiverFrames(ICANInterface &receiver);

    IObd2Modes* ReceiveFrames();
private:
    bool readAndAssembleFrames();
};

#endif