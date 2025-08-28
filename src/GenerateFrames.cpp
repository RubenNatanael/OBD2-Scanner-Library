#include "../include/GenerateFrame.h"

GenerateFrame::GenerateFrame(ICANInterface &sender): s(&sender){}

void GenerateFrame::ShowCurrentData(uint8_t pid){
    LOG_INFO("Client requested ShowCurrentData");
    SendFrame(0x728, MShowCurrentData, pid);
}

void GenerateFrame::ShowFreezFrameData(uint8_t pid) {
    LOG_INFO("Client requested ShowFreezFrameData");
    SendFrame(0x728, MShowFreezFrameData, pid);

}
void GenerateFrame::ShowDTCs() {
    LOG_INFO("Client requested ShowDTCs");
    SendFrame(0x728, MShowDTCs);
}
void GenerateFrame::ClearDTCs() {
    LOG_INFO("Client requested ClearDTCs");
    SendFrame(0x728, MClearDTCs);
}
void GenerateFrame::OxygenSensorMonitorSensorResult(uint8_t pid) {
    LOG_INFO("Client requested OxygenSensorMonitorSensorResult");
    SendFrame(0x728, MOxygenSensorMonitorSensorResult, pid);
}
void GenerateFrame::ShowOnBoardMonitoringTestResult(uint8_t pid) {
    LOG_INFO("Client requested ShowOnBoardMonitoringTestResult");
    SendFrame(0x728, MShowOnBoardMonitoringTestResult);
}
void GenerateFrame::RequestVehicleInformation(uint8_t pid) {
    LOG_INFO("Client requested RequestVehicleInformation");
    SendFrame(0x728, MRequestVehicleInformation);
}
void GenerateFrame::PermanentDTCs() {
    LOG_INFO("Client requested PermanentDTCs");
    SendFrame(0x728, MPermanentDTCs);
}

void GenerateFrame::SendFrame(uint32_t id, MOD mode, uint8_t pid) {

    can_frame frame;

    frame.can_id = id;
    frame.can_dlc = 8;
    frame.data[0] = (pid == -1)? 1: 2;
    frame.data[1] = mode;
    frame.data[2] = (pid == -1)? 0: pid;

    for (int i = 3; i < 8; i++) {
        frame.data[i] = 0x0;
    }

    s->send(frame);
}
