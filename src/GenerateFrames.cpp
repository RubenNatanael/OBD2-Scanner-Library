#include "../include/GenerateFrame.h"

GenerateFrame::GenerateFrame(ICANInterface &sender): s(&sender){}

void GenerateFrame::ShowCurrentData(uint8_t pid){
    LOG_INFO("Client requested ShowCurrentData");
    SendFrame(broadcast_id, MShowCurrentData, pid);
}

void GenerateFrame::ShowFreezFrameData(uint8_t pid) {
    LOG_INFO("Client requested ShowFreezFrameData");
    SendFrame(broadcast_id, MShowFreezFrameData, pid);

}
void GenerateFrame::ShowDTCs() {
    LOG_INFO("Client requested ShowDTCs");
    SendFrame(broadcast_id, MShowDTCs);
}
void GenerateFrame::ClearDTCs() {
    LOG_INFO("Client requested ClearDTCs");
    SendFrame(broadcast_id, MClearDTCs);
}
void GenerateFrame::OxygenSensorMonitorSensorResult(uint8_t pid) {
    LOG_INFO("Client requested OxygenSensorMonitorSensorResult");
    SendFrame(broadcast_id, MOxygenSensorMonitorSensorResult, pid);
}
void GenerateFrame::ShowOnBoardMonitoringTestResult() {
    LOG_INFO("Client requested ShowOnBoardMonitoringTestResult");
    SendFrame(broadcast_id, MShowOnBoardMonitoringTestResult);
}
void GenerateFrame::RequestVehicleInformation(uint8_t pid) {
    LOG_INFO("Client requested RequestVehicleInformation");
    SendFrame(broadcast_id, MRequestVehicleInformation, pid);
}
void GenerateFrame::PendingDTCs() {
    LOG_INFO("Client requested Pending DTCs");
    SendFrame(broadcast_id, MPendingDTCs);
}
void GenerateFrame::PermanentDTCs() {
    LOG_INFO("Client requested PermanentDTCs");
    SendFrame(broadcast_id, MPermanentDTCs);
}

void logCanFrame(const can_frame &frame) {
    char buf[128];
    int offset = 0;

    offset += snprintf(buf + offset, sizeof(buf) - offset,
                       "CAN ID=0x%X DLC=%d Data=",
                       frame.can_id, frame.can_dlc);

    for (int i = 0; i < frame.can_dlc; i++) {
        offset += snprintf(buf + offset, sizeof(buf) - offset,
                           " %02X", frame.data[i]);
    }

    LOG_INFO(std::string(buf));
}


void GenerateFrame::SendFrame(uint32_t id, MOD mode, int pid) {

    can_frame frame;

    frame.can_id = id;
    frame.can_dlc = 8;
    frame.data[0] = (pid == -1)? 1: 2;
    frame.data[1] = mode;
    frame.data[2] = (pid == -1)? 0: pid;

    for (int i = 3; i < 8; i++) {
        frame.data[i] = 0x0;
    }

    if (s->send(frame)) {
        LOG_INFO("Frame sanded");
        logCanFrame(frame);
        return;
    }
    LOG_ERR("Error while sending frame");
}
