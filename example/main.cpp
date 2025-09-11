#include <iostream>

#include "../include/OBD2Scanner.h"

int main() {
    
    OBD2Scanner obd("vcan0");

    auto res = obd.getDTCs();
    for(auto& r: res) {
        std::cout<<r.label<<"\n  *"<<r.value<<"\n";
    }
    auto res2 = obd.getFreezFrame(0x00);
    for(auto& r: res2) {
        std::cout<<r.label<<"\n  *"<<r.value<<"\n";
    }
    auto res3 = obd.ClearDTCs();
    for(auto& r: res3) {
        std::cout<<r.label<<"\n  *"<<r.value<<"\n";
    }
}
