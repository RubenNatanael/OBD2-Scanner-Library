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
#include "../pids/Mode1Pids.h"
#include "../pids/TroubleCodes.h"

#ifdef ARDUINO

#include "ArduinoSTL.h"

#endif

#include <vector>
#include <utility>

class IObd2Modes {
    protected:
        uint8_t *responseBuffer;
        uint8_t receivedBytes = 0;
    public:
        virtual std::vector<DecodedItem> Decodify();
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
    uint8_t* responseBuffer;
    uint8_t receivedBytes = 0;
    uint8_t totalLength = 0;

public:
    ReceiverFrames(ICANInterface &receiver);

    IObd2Modes* ReceiveFrames();
private:

    bool readAndAssembleFrames() {
        receivedBytes = 0;
        totalLength = 0;
        can_frame flowControl;
        flowControl.can_id = 0x777;
        flowControl.can_dlc = 4;
        flowControl.data[0] = 0x30;
        flowControl.data[1] = 0x00;
        flowControl.data[2] = 0x00;
        flowControl.data[3] = 0x00;


        while (true) {
            // if (!r.available()) {
            //     // Optional: small delay to avoid busy-wait
            //     sleep(1); 
            //     continue;
            // }

            can_frame frame;
            r.receive(frame);

            uint8_t pci = frame.data[0];
            uint8_t frameType = pci >> 4;

            // Single Frame
            if (frameType == 0x0) {
                totalLength = pci & 0x0F;
                memcpy(responseBuffer, &frame.data[1], totalLength);
                receivedBytes = totalLength;
                return true;
            }
            // First Frame
            else if (frameType == 0x1) {
                totalLength = ((pci & 0x0F) << 8) | frame.data[1];
                memcpy(responseBuffer, &frame.data[2], 6);
                receivedBytes = 6;

                r.send(flowControl);
            }
            // Consecutive Frame
            else if (frameType == 0x2) {
                uint8_t length = pci & 0x0F;
                memcpy(responseBuffer + receivedBytes, &frame.data[1], length);
                receivedBytes += length;

                if (receivedBytes >= totalLength) {
                    return true;
                }
            }
        }

        // Never reached, but safe
        return false;
    }
    uint8_t* getResponseBuffer() { return responseBuffer; }
    uint8_t getResponseLength() const { return totalLength; }
};

#endif