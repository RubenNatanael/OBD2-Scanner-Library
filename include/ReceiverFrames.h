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
            if (frame.data[1] == 1) {
                currentMode = new Mode1();
            } else if (frame.data[1] == 3) {
                currentMode = new Mode3();
            }
            return currentMode;
        }
};

class IObd2Modes {

    public:

        virtual std::vector<DecodedItem> Decodify(can_frame &frame);
};

class Mode1: public IObd2Modes {
    public:

        std::vector<DecodedItem> Decodify(can_frame &frame) override;

};

class Mode3: public IObd2Modes {
    public:

        std::vector<DecodedItem> Decodify(can_frame &frame) override;

};

#endif