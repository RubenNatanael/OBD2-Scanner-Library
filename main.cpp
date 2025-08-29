#include <iostream>
#include "OBD2Scanner.h"

int main() {
    
    SocketCAN com;
    GenerateFrame g(com);
    ReceiverFrames r(com);

    com.init("vcan0");
    g.ShowDTCs();
    auto res = r.ReceiveFrames()->Decodify();
    for(auto& r: res) {
        std::cout<<r.label<<"\n  *"<<r.value<<"\n";
    }
}