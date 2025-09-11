#!/bin/bash

make qt

if ! ip link show vcan0 &>/dev/null; then
    echo "Creating vcan0..."
    sudo ip link add dev vcan0 type vcan
    sudo ip link set up vcan0
else
    echo "vcan0 already exists"
fi

./carSimulator.sh &
sleep 1
./qt_example

pkill -f carSimulator.sh
