#include <iostream>

#include "../include/OBD2Scanner.h"

int main() {
    
    // Using socket CAN
    ICANInterface* transport = new SocketCAN();
    transport->init("vcan0");

    // Using ELM327 protocol
    /* ICANInterface* transport = new ELM327Transport();
    if (!transport->init("/dev/pts/3")) {
        exit(1);
    } */
   
    OBD2Scanner obd(transport);

    auto res = obd.getDTCs();
    for(auto& r: res) {
        std::cout<<r.label<<"\n  *"<<r.value<<"\n";
    }
    auto res2 = obd.getFreezFrame(OBD2::Pid::Supported0);
    for(auto& r: res2) {
        std::cout<<r.label<<"\n  *"<<r.value<<"\n";
    }
    auto res3 = obd.ClearDTCs();
    for(auto& r: res3) {
        std::cout<<r.label<<"\n  *"<<r.value<<"\n";
    }

    
}
