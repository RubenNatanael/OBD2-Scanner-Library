/*
int main() {
    SocketCAN sender;
    sender.init("vcan0");
    can_frame frame;
    ReceiverFrames r(sender);
    auto rsp = r.ReceiveFrames(frame)->Decodify(frame);
}
*/
#ifndef RECEIVER_FRAMES
#define RECEIVER_FRAMES

#include "CustomSocket.h"
#include "MultiLogger.h"
#include "../pids/Mode1Pids.h"
#include "../pids/TroubleCodes.h"

#ifdef ARDUINO

#include "ArduinoSTL.h"

#endif

#include <vector>
#include <utility>

class ReceiverFrames {

    private:
        ICANInterface &s;
        IObd2Modes* currentMode;

    public:
        ReceiverFrames(ICANInterface &sender) : s(sender) {}

        IObd2Modes* ReceiveFrames(can_frame &frame) {
            s.receive(frame);

            currentMode = nullptr;
            uint8_t byteMode = frame.data[1];
            if (byteMode == 1 || byteMode == 2) {
                currentMode = new Mode1();
            } else if (byteMode == 3 || byteMode == 7 || byteMode == 0x0A) {
                currentMode = new Mode3();
            } else if (byteMode == 4) {
                currentMode = new Mode4();
            }
            return currentMode;
        }
};

class IObd2Modes {

    public:
        virtual std::vector<DecodedItem> Decodify(uint8_t* data, uint8_t len);
};

class Mode1: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify(uint8_t* data, uint8_t len) override;

};

class Mode3: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify(uint8_t* data, uint8_t len) override;
    private:
        std::string DecodifyDTC(uint8_t *dtc);

};

class Mode4: public IObd2Modes {
    public:
        std::vector<DecodedItem> Decodify(uint8_t* data, uint8_t len) override;

};

#endif