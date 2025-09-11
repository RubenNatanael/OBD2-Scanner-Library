#ifndef GENARETE_FRAME
#define GENERATE_FRAME

#include <stdio.h>

#include "MultiLogger.h"
#include "CustomSocket.h"

enum MOD {
    MShowCurrentData = 0x01,
    MShowFreezFrameData = 0x02,
    MShowDTCs = 0x03,
    MClearDTCs = 0x04,
    MOxygenSensorMonitorSensorResult = 0x05,
    MShowOnBoardMonitoringTestResult = 0x06,
    MPendingDTCs = 0x07,
    MRequestVehicleInformation = 0x09,
    MPermanentDTCs = 0x0A
};

class GenerateFrame {

    private:
        ICANInterface* s;
        struct can_frame frame;
    
    public:
        GenerateFrame(ICANInterface &sender);
        void ShowCurrentData(uint8_t pid);
        void ShowFreezFrameData(uint8_t pid);
        void ShowDTCs();
        void ClearDTCs();
        void OxygenSensorMonitorSensorResult(uint8_t pid);
        void ShowOnBoardMonitoringTestResult();
        void PendingDTCs();
        void RequestVehicleInformation(uint8_t pid);
        void PermanentDTCs();

    
    private:
        void SendFrame(uint32_t id, MOD mode, int pid = -1);

};

#endif