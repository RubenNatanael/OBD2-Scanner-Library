#include <iostream>

#include "../include/OBD2Scanner.h"

int main() {
    
    OBD2Scanner obd("vcan0");

    auto res = obd.getDTCs();
    for(auto& r: res) {
        std::cout<<r.label<<"\n  *"<<r.value<<"\n";
    }
}