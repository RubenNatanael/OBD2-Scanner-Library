/*
int main() {
    SocketCAN sender;
    sender.init("vcan0");
    ReceiverFrames r(sender);
    auto rsp = r.ReceiveFrames()->Decodify();
}
*/
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
};

class Mode1: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify() override;

};

class Mode3: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify() override;
    private:
        std::string DecodifyDTC(uint8_t *dtc);

};

class Mode4: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify() override;

};

class ReceiverFrames {
private:
    ICANInterface &r;
    Mode1 mode1;
    Mode3 mode3;
    Mode4 mode4;
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