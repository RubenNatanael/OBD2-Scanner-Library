#include "Mode1Pids.h"

const PIDEntry Mode1Pid::pidTable[] = {
    {0x00, &Mode1Pid::DecodePID00},
    {0x01, &Mode1Pid::DecodePID01},
    {0x02, &Mode1Pid::DecodePID02},
    {0x03, &Mode1Pid::DecodePID03},
    {0x04, &Mode1Pid::DecodePID04},
    {0x05, &Mode1Pid::DecodePID05},
    {0x06, &Mode1Pid::DecodePID06},
    {0x07, &Mode1Pid::DecodePID07},
    {0x08, &Mode1Pid::DecodePID08},
    {0x09, &Mode1Pid::DecodePID09},
    {0x0A, &Mode1Pid::DecodePID0A},
    {0x0B, &Mode1Pid::DecodePID0B},
    {0x0C, &Mode1Pid::DecodePID0C},
    {0x0D, &Mode1Pid::DecodePID0D},
    {0x0E, &Mode1Pid::DecodePID0E},
    {0x0F, &Mode1Pid::DecodePID0F},
    {0x10, &Mode1Pid::DecodePID10},
    {0x11, &Mode1Pid::DecodePID11},
    {0x12, &Mode1Pid::DecodePID12},
    {0x13, &Mode1Pid::DecodePID13},
    {0x1F, &Mode1Pid::DecodePID1F},
    {0x20, &Mode1Pid::DecodePID20},
    {0x21, &Mode1Pid::DecodePID21},
    {0x22, &Mode1Pid::DecodePID22},
    {0x23, &Mode1Pid::DecodePID23},
    {0x2C, &Mode1Pid::DecodePID2C},
    {0x2D, &Mode1Pid::DecodePID2D},
    {0x2E, &Mode1Pid::DecodePID2E},
    {0x2F, &Mode1Pid::DecodePID2F},
    {0x30, &Mode1Pid::DecodePID30},
    {0x31, &Mode1Pid::DecodePID31},
    {0x32, &Mode1Pid::DecodePID32},
    {0x33, &Mode1Pid::DecodePID33},
    {0x40, &Mode1Pid::DecodePID40},
    {0x41, &Mode1Pid::DecodePID41},
    {0x42, &Mode1Pid::DecodePID42},
    {0x43, &Mode1Pid::DecodePID43},
    {0x44, &Mode1Pid::DecodePID44},
    {0x45, &Mode1Pid::DecodePID45},
    {0x46, &Mode1Pid::DecodePID46},
    {0x47, &Mode1Pid::DecodePID47},
    {0x48, &Mode1Pid::DecodePID48},
    {0x49, &Mode1Pid::DecodePID49},
    {0x4A, &Mode1Pid::DecodePID4A},
    {0x4B, &Mode1Pid::DecodePID4B},
    {0x4C, &Mode1Pid::DecodePID4C},
    {0x4D, &Mode1Pid::DecodePID4D},
    {0x4E, &Mode1Pid::DecodePID4E},
    // {0x2F, &Mode1Pid::DecodePID4F},
    // {0x20, &Mode1Pid::DecodePID40},
    // {0x21, &Mode1Pid::DecodePID41},
    // {0x22, &Mode1Pid::DecodePID42},
    // {0x23, &Mode1Pid::DecodePID43},
    // {0x24, &Mode1Pid::DecodePID44},
    // {0x25, &Mode1Pid::DecodePID45},
    // {0x26, &Mode1Pid::DecodePID46},
    // {0x27, &Mode1Pid::DecodePID47},
    // {0x28, &Mode1Pid::DecodePID48},
    // {0x29, &Mode1Pid::DecodePID49},
    // {0x2A, &Mode1Pid::DecodePID4A},
    // {0x2B, &Mode1Pid::DecodePID4B},
    // {0x2C, &Mode1Pid::DecodePID4C},
    // {0x2D, &Mode1Pid::DecodePID4D},
    // {0x2E, &Mode1Pid::DecodePID4E},
    // {0x2F, &Mode1Pid::DecodePID4F},
};

const PIDEntry* Mode1Pid::getTable() {
    return pidTable;
}

// PID 0x00: Supported PIDs 01–20
std::vector<DecodedItem> Mode1Pid::DecodePID00(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 4) return result;

    for (int byteIndex = 0; byteIndex < len; ++byteIndex) {
        for (int bit = 7; bit >= 0; --bit) {
            if (data[byteIndex] & (1 << bit)) {
                result.push_back({
                    0x01,
                    0x00,
                    "Supported PID",
                    std::to_string(byteIndex*8 + (8-bit))
                });
            }
        }
    }
    return result;
}

// PID 0x01: Monitor status since DTCs cleared
std::vector<DecodedItem> Mode1Pid::DecodePID01(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 4) return result;

    uint8_t A = data[0], B = data[1], C = data[2], D = data[3];

    bool milOn = (A & 0x80) != 0;
    int dtcCount = A & 0x7F;
    bool isDiesel = (B & 0x08) != 0;
    bool commonComplete = ((B >> 4) & 0x07) == (B & 0x07);
    bool engineComplete = C == D;

    result.push_back({0x01,0x1, "MIL", milOn ? "ON" : "OFF"});
    result.push_back({0x01,0x1, "DTC Count", std::to_string(dtcCount)});
    result.push_back({0x01,0x1, "Engine Type", isDiesel ? "Diesel" : "Gasoline"});
    result.push_back({0x01,0x1, "Common Tests Completed", commonComplete ? "Yes" : "No"});
    result.push_back({0x01,0x1, "Engine Tests Completed", engineComplete ? "Yes" : "No"});

    return result;
}

// PID 0x02: DTC that causes freez frame
std::vector<DecodedItem> Mode1Pid::DecodePID02(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 2) return result;

    uint8_t encodedDtc[2];

     for (int i = 0; i < len / 2; i++) {
        encodedDtc[0] = data[i * 2];
        encodedDtc[1] = data[(i * 2) + 1];

        // Letter
        char letter;
        switch ((encodedDtc[0] >> 6) & 0x03) {
            case 0: letter = 'P'; break;
            case 1: letter = 'C'; break;
            case 2: letter = 'B'; break;
            case 3: letter = 'U'; break;
        }

        // Digits
        uint8_t digit1 = (encodedDtc[0] >> 4) & 0x03;
        uint8_t digit2 = encodedDtc[0] & 0x0F;
        uint8_t digit3 = (encodedDtc[1] >> 4) & 0x0F;
        uint8_t digit4 = encodedDtc[1] & 0x0F;

        char buf[6];
        snprintf(buf, sizeof(buf), "%1X%1X%1X%1X", digit1, digit2, digit3, digit4);
        std::string DTC = letter + std::string(buf);
        result.push_back({0x01,0x2, "DTC: ",Parse(DTC)});
        }
    
    if (result.empty()) {
        result.push_back({0x01,0x2, "Erro: Not found","1"});
    }
    return result;
}

//TODO - verify
// PID 0x03: Fuel System Status
std::vector<DecodedItem> Mode1Pid::DecodePID03(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    std::string text = "";
    switch (data[0])
    {
    case 0:
        text = "0 	The motor is off";
        break;
    case 1:
        text = "1 	Open loop due to insufficient engine temperature";
        break;
    case 2:
        text = "2 	Closed loop, using oxygen sensor feedback to determine fuel mix";
        break;
    case 4:
        text = "4 	Open loop due to engine load OR fuel cut due to decelerationf";
        break;
    case 8:
        text = "8 	Open loop due to system failure";
        break;
    case 16:
        text = "16 	Closed loop, using at least one oxygen sensor but there is a fault in the feedback system ";
        break;
    
    default:
        text = data[0] + "Invalide response";
        break;
    }
    result.push_back({0x01,0x3, "Fuel System Status", text});
    return result;
}

// PID 0x04: Calculate Engine Load
std::vector<DecodedItem> Mode1Pid::DecodePID04(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    float procentage = (float)data[0] * 2.55;
    result.push_back({0x01,0x4, "Engine Load", std::to_string(procentage) + " %"});
    return result;
}

// PID 0x05: Engine Coolant Temp
std::vector<DecodedItem> Mode1Pid::DecodePID05(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    result.push_back({0x01,0x5, "Coolant Temp", std::to_string((int)data[0]-40) + " °C"});
    return result;
}

// PID 0x06: Short term fuel trim (STFT)—Bank 1 
std::vector<DecodedItem> Mode1Pid::DecodePID06(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    float procentage = ((float)data[0] / 1.28) - 100;
    result.push_back({0x01,0x6, "STFT-Bank 1", std::to_string(procentage) + " %"});
    return result;
}

// PID 0x07: Long term fuel trim (LTFT)—Bank 1 
std::vector<DecodedItem> Mode1Pid::DecodePID07(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    float procentage = ((float)data[0] / 1.28) - 100;
    result.push_back({0x01,0x7, "LTFT-Bank 1", std::to_string(procentage) + " %"});
    return result;
}

// PID 0x08: Short term fuel trim (STFT)—Bank 2
std::vector<DecodedItem> Mode1Pid::DecodePID08(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    float procentage = ((float)data[0] / 1.28) - 100;
    result.push_back({0x01,0x8, "STFT-Bank 1", std::to_string(procentage) + " %"});
    return result;
}

// PID 0x08: Long term fuel trim (LTFT)—Bank 2
std::vector<DecodedItem> Mode1Pid::DecodePID09(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    float procentage = ((float)data[0] / 1.28) - 100;
    result.push_back({0x01,0x9, "LTFT-Bank 1", std::to_string(procentage) + " %"});
    return result;
}

// PID 0x0A: Fuel Presure
std::vector<DecodedItem> Mode1Pid::DecodePID0A(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    int presure = data[0] * 3;
    result.push_back({0x01,0xA, "Fuel Presure", std::to_string(presure) + " kPa"});
    return result;
}

// PID 0x0B: Intake manifold absolute pressure 
std::vector<DecodedItem> Mode1Pid::DecodePID0B(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    result.push_back({0x01,0xB, "Intake manifold absolute pressure", std::to_string(data[0]) + " kPa"});
    return result;
}

// PID 0x0C: Engine RPM
std::vector<DecodedItem> Mode1Pid::DecodePID0C(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 2) return result;
    int raw = (data[0] << 8) | data[1];
    float rpm = raw / 4.0f;
    result.push_back({0x01,0xC, "RPM", std::to_string(rpm) + " rpm"});
    return result;
}

// PID 0x0D: Vehicle Speed
std::vector<DecodedItem> Mode1Pid::DecodePID0D(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 1) return result;
    result.push_back({0x01,0xD, "Speed", std::to_string(data[0]) + " km/h"});
    return result;
}

// PID 0x0E: Timing Advance
std::vector<DecodedItem> Mode1Pid::DecodePID0E(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        DecodedItem r;
        r.mode = 1;
        r.pid = 0x0E;
        r.label = "Timing Advance";
        r.value = std::to_string(data[0]/2.0f - 64) + " °BTDC"; // formula per SAE J1979
        result.push_back(r);
    }
    return result;
}

// PID 0x0F: Intake Air Temp
std::vector<DecodedItem> Mode1Pid::DecodePID0F(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        result.push_back({0x01,0xF, "Intake Air Temp", std::to_string(data[0] - 40) + " °C"});
    }
    return result;
}

// PID 0x10: Mass Air Flow Rate
std::vector<DecodedItem> Mode1Pid::DecodePID10(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        float maf = ((data[0] << 8) | data[1]) / 100.0f;
        result.push_back({0x01,0x10, "Mass Air Flow Rate", std::to_string(maf) + " g/s"});
    }
    return result;
}

// PID 0x11: Throttle Position
std::vector<DecodedItem> Mode1Pid::DecodePID11(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        result.push_back({0x01,0x11, "Throttle Position", std::to_string(data[0] * 100.0f / 255.0f) + " %"});
    }
    return result;
}

// PID 0x12: Commanded Secondary Air Status
std::vector<DecodedItem> Mode1Pid::DecodePID12(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        std::string v = (data[0] == 0) ? "Not Commanded" : "Commanded";
        result.push_back({0x01,0x12, "Secondary Air Status", v});
    }
    return result;
}

// PID 0x13: O2 Sensor Present Bank 1 Sensor 1
std::vector<DecodedItem> Mode1Pid::DecodePID13(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) result.push_back({0x01,0x13, "O2 Sensor B1 S1", (data[0] ? "Present" : "Not Present")});
    return result;
}

// PID 0x14 - 1B: O2 Sensor 1 to 8
std::vector<DecodedItem> Mode1Pid::DecodePID14to1B(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        float voltage = data[0] / 200.0f; 
        std::string stft = (data[1] == 0xFF) ? "Not used" 
                                               : std::to_string((100.0f/128.0f)*data[1] - 100.0f) + " %";
        result.push_back({0x01,0x14, "O2 Sensor B1 S" , "Voltage: " + std::to_string(voltage) + " V, STFT: " + stft});
    }
    return result;
}

// PID 0x1F: Run time since engine start
std::vector<DecodedItem> Mode1Pid::DecodePID1F(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        int seconds = (data[0] << 8) | data[1];
        result.push_back({0x01,0x1F, "Run Time Since Engine Start", std::to_string(seconds) + " s"});
    }
    return result;
}

// PID 0x20: PIDs supported [21–40]
std::vector<DecodedItem> Mode1Pid::DecodePID20(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 4) return result;
    for (int byteIndex = 0; byteIndex < len; ++byteIndex) {
        for (int bit = 7; bit >= 0; --bit) {
            if (data[byteIndex] & (1 << bit)) {
                result.push_back({0x01,0x20, "Supported PID", std::to_string(byteIndex * 8 + (8 - bit) + 0x20)});
            }
        }
    }
    return result;
}

// PID 0x21: Distance traveled with MIL on
std::vector<DecodedItem> Mode1Pid::DecodePID21(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        int km = (data[0] << 8) | data[1];
        result.push_back({0x01,0x21, "Distance with MIL On", std::to_string(km) + " km"});
    }
    return result;
}

// PID 0x22: Fuel Rail Pressure (relative to manifold vacuum)
std::vector<DecodedItem> Mode1Pid::DecodePID22(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        int kPa = ((data[0] << 8) | data[1]) * 0.079; // spec: (A*256 + B) * 0.079 kPa
        result.push_back({0x01,0x22, "Fuel Rail Pressure (Relative)", std::to_string(kPa) + " kPa"});
    }
    return result;
}

// PID 0x23: Fuel Rail Gauge Pressure (diesel, gasoline direct injection)
std::vector<DecodedItem> Mode1Pid::DecodePID23(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        int kPa = ((data[0] << 8) | data[1]) * 10; // spec: (A*256 + B) * 10 kPa
        result.push_back({0x01,0x23, "Fuel Rail Gauge Pressure", std::to_string(kPa) + " kPa"});
    }
    return result;
}

// PID 0x2C: Commanded EGR
std::vector<DecodedItem> Mode1Pid::DecodePID2C(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float percent = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x2C, "Commanded EGR", std::to_string(percent) + " %"});
    }
    return result;
}

// PID 0x2D: EGR Error
std::vector<DecodedItem> Mode1Pid::DecodePID2D(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float percent = (data[0] * 100.0f / 128.0f) - 100.0f;
        result.push_back({0x01,0x2D, "EGR Error", std::to_string(percent) + " %"});
    }
    return result;
}

// PID 0x2E: Commanded Evaporative Purge
std::vector<DecodedItem> Mode1Pid::DecodePID2E(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float percent = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x2E, "Commanded Evaporative Purge", std::to_string(percent) + " %"});
    }
    return result;
}

// PID 0x2F: Fuel Tank Level Input
std::vector<DecodedItem> Mode1Pid::DecodePID2F(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float percent = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x2F, "Fuel Tank Level Input", std::to_string(percent) + " %"});
    }
    return result;
}

// PID 0x30: Warm-ups Since Codes Cleared
std::vector<DecodedItem> Mode1Pid::DecodePID30(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        int count = data[0];
        result.push_back({0x01,0x30, "Warm-ups Since Codes Cleared", std::to_string(count)});
    }
    return result;
}

// PID 0x31: Distance traveled since codes cleared
std::vector<DecodedItem> Mode1Pid::DecodePID31(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        int km = (data[0] << 8) | data[1];
        result.push_back({0x01,0x31, "Distance Since Codes Cleared", std::to_string(km) + " km"});
    }
    return result;
}

// PID 0x32: Evap System Vapor Pressure
std::vector<DecodedItem> Mode1Pid::DecodePID32(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        int Pa = ((data[0] << 8) | data[1]) / 4;
        result.push_back({0x01,0x32, "Evap System Vapor Pressure", std::to_string(Pa) + " Pa"});
    }
    return result;
}

// PID 0x33: Absolute Barometric Pressure
std::vector<DecodedItem> Mode1Pid::DecodePID33(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        int kPa = data[0];
        result.push_back({0x01,0x33, "Absolute Barometric Pressure", std::to_string(kPa) + " kPa"});
    }
    return result;
}

// PID 0x40: Supported PIDs 0x41–0x60
std::vector<DecodedItem> Mode1Pid::DecodePID40(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len < 4) return result;
    for (int byteIndex = 0; byteIndex < len; ++byteIndex) {
        for (int bit = 7; bit >= 0; --bit) {
            if (data[byteIndex] & (1 << bit)) {
                result.push_back({
                    0X01,
                    0X40,
                    "Supported PID",
                    "0x" + std::to_string(0x41 + byteIndex*8 + (7-bit))
                });
            }
        }
    }
    return result;
}

// PID 0x41: Monitor status this drive cycle
std::vector<DecodedItem> Mode1Pid::DecodePID41(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 4) {
        result.push_back({0x01,0x41, "Misfire Monitoring Complete", (data[0] & 0x01) ? "Yes" : "No"});
        result.push_back({0x01,0x41, "Fuel System Monitoring Complete", (data[0] & 0x02) ? "Yes" : "No"});
        result.push_back({0x01,0x41, "Components Monitoring Complete", (data[0] & 0x04) ? "Yes" : "No"});
    }
    return result;
}

// PID 0x42: Control module voltage
std::vector<DecodedItem> Mode1Pid::DecodePID42(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        float voltage = ((data[0] << 8) | data[1]) / 1000.0f;
        result.push_back({0x01,0x42, "Control Module Voltage", std::to_string(voltage) + " V"});
    }
    return result;
}

// PID 0x43: Absolute load value
std::vector<DecodedItem> Mode1Pid::DecodePID43(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        float load = ((data[0] << 8) | data[1]) * 100.0f / 255.0f;
        result.push_back({0x01,0x43, "Absolute Load Value", std::to_string(load) + " %"});
    }
    return result;
}

// PID 0x44: Commanded equivalence ratio
std::vector<DecodedItem> Mode1Pid::DecodePID44(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        float ratio = ((data[0] << 8) | data[1]) / ( 2 / 65536.0f);
        result.push_back({0x01,0x44, "Commanded Equivalence Ratio", std::to_string(ratio)});
    }
    return result;
}

// PID 0x45: Relative throttle position
std::vector<DecodedItem> Mode1Pid::DecodePID45(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float pos = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x45, "Relative Throttle Position", std::to_string(pos) + " %"});
    }
    return result;
}

// PID 0x46: Ambient air temperature
std::vector<DecodedItem> Mode1Pid::DecodePID46(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        int temp = data[0] - 40;
        result.push_back({0x01,0x46, "Ambient Air Temp", std::to_string(temp) + " °C"});
    }
    return result;
}

// PID 0x47: Absolute throttle position B
std::vector<DecodedItem> Mode1Pid::DecodePID47(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float pos = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x47, "Throttle Position B", std::to_string(pos) + " %"});
    }
    return result;
}

// PID 0x48: Absolute throttle position C
std::vector<DecodedItem> Mode1Pid::DecodePID48(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float pos = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x48, "Throttle Position C", std::to_string(pos) + " %"});
    }
    return result;
}

// PID 0x49: Accelerator pedal position D
std::vector<DecodedItem> Mode1Pid::DecodePID49(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float pos = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x49, "Accel Pedal Pos D", std::to_string(pos) + " %"});
    }
    return result;
}

// PID 0x4A: Accelerator pedal position E
std::vector<DecodedItem> Mode1Pid::DecodePID4A(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float pos = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x4A, "Accel Pedal Pos E", std::to_string(pos) + " %"});
    }
    return result;
}

// PID 0x4B: Accelerator pedal position F
std::vector<DecodedItem> Mode1Pid::DecodePID4B(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float pos = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x4B, "Accel Pedal Pos F", std::to_string(pos) + " %"});
    }
    return result;
}

// PID 0x4C: Commanded throttle actuator
std::vector<DecodedItem> Mode1Pid::DecodePID4C(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 1) {
        float pos = data[0] * 100.0f / 255.0f;
        result.push_back({0x01,0x4C, "Commanded Throttle Actuator", std::to_string(pos) + " %"});
    }
    return result;
}

// PID 0x4D: Time run with MIL on
std::vector<DecodedItem> Mode1Pid::DecodePID4D(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        int minutes = (data[0] << 8) | data[1];
        result.push_back({0x01,0x4D, "Time With MIL On", std::to_string(minutes) + " min"});
    }
    return result;
}

// PID 0x4E: Time since DTCs cleared
std::vector<DecodedItem> Mode1Pid::DecodePID4E(const uint8_t* data, uint8_t len) {
    std::vector<DecodedItem> result;
    if (len >= 2) {
        int minutes = (data[0] << 8) | data[1];
        result.push_back({0x01,0x4E, "Time Since DTCs Cleared", std::to_string(minutes) + " min"});
    }
    return result;
}

const size_t Mode1Pid::pidTableSize = sizeof(pidTable)/sizeof(pidTable[0]);

