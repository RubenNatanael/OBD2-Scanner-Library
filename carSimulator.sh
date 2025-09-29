#!/bin/bash

candump vcan0 | while read -r line; do
    # Extract everything after the ] (the data bytes)
    bytes=$(echo "$line" | awk -F']' '{print $2}')

    # Trim leading spaces
    bytes=$(echo "$bytes" | xargs)

    # Extract 2nd and 3rd bytes
    second_byte=$(echo "$bytes" | awk '{print $2}')
    third_byte=$(echo "$bytes" | awk '{print $3}')

    case "$second_byte" in
        "01")
            cansend vcan0 7E1#0641${third_byte}983BA013
            ;;
        "02")
            cansend vcan0 7E1#0742${third_byte}00983BA013
            ;;
        "03")
            cansend vcan0 7E1#06430207450744
            cansend vcan0 7E1#06430207450744
            ;;
        "07")
            cansend vcan0 7E1#0747074507440746
            ;;
        "0A")
            cansend vcan0 7E1#074A074507440746
            ;;
        "04")
            cansend vcan0 7E1#024400
            ;;
    esac
done
