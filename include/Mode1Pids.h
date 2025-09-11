#ifndef MODE1_PIDS
#define MODE1_PIDS

#include <stdint.h>
#include <vector>
#include <string>
#include "TroubleCodes.h"

struct DecodedItem {
    std::string label;  // Ex: "RPM"
    std::string value;  // Ex: "2300 rpm"
};

using DecoderFunc = std::vector<DecodedItem> (*)(const uint8_t* data, uint8_t len);

struct PIDEntry {
    uint8_t pid;
    DecoderFunc decoder;
};

class Mode1Pid {

public:
// PID 0x00: Supported PIDs 01–20
static std::vector<DecodedItem> DecodePID00(const uint8_t* data, uint8_t len);
// PID 0x01: Monitor status since DTCs cleared
static std::vector<DecodedItem> DecodePID01(const uint8_t* data, uint8_t len);
// PID 0x02: DTC that causes freez frame
static std::vector<DecodedItem> DecodePID02(const uint8_t* data, uint8_t len);
//TODO - verify
// PID 0x03: Fuel System Status
static std::vector<DecodedItem> DecodePID03(const uint8_t* data, uint8_t len);
// PID 0x04: Calculate Engine Load
static std::vector<DecodedItem> DecodePID04(const uint8_t* data, uint8_t len);
// PID 0x05: Engine Coolant Temp
static std::vector<DecodedItem> DecodePID05(const uint8_t* data, uint8_t len);
// PID 0x06: Short term fuel trim (STFT)—Bank 1 
static std::vector<DecodedItem> DecodePID06(const uint8_t* data, uint8_t len);
// PID 0x07: Long term fuel trim (LTFT)—Bank 1 
static std::vector<DecodedItem> DecodePID07(const uint8_t* data, uint8_t len);
// PID 0x08: Short term fuel trim (STFT)—Bank 2
static std::vector<DecodedItem> DecodePID08(const uint8_t* data, uint8_t len);
// PID 0x08: Long term fuel trim (LTFT)—Bank 2
static std::vector<DecodedItem> DecodePID09(const uint8_t* data, uint8_t len);
// PID 0x0A: Fuel Presure
static std::vector<DecodedItem> DecodePID0A(const uint8_t* data, uint8_t len);
// PID 0x0B: Intake manifold absolute pressure 
static std::vector<DecodedItem> DecodePID0B(const uint8_t* data, uint8_t len);
// PID 0x0C: Engine RPM
static std::vector<DecodedItem> DecodePID0C(const uint8_t* data, uint8_t len);
// PID 0x0D: Vehicle Speed
static std::vector<DecodedItem> DecodePID0D(const uint8_t* data, uint8_t len);
// PID 0x0E: Timing Advance
static std::vector<DecodedItem> DecodePID0E(const uint8_t* data, uint8_t len);
// PID 0x0F: Intake Air Temp
static std::vector<DecodedItem> DecodePID0F(const uint8_t* data, uint8_t len);
// PID 0x10: Mass Air Flow Rate
static std::vector<DecodedItem> DecodePID10(const uint8_t* data, uint8_t len);
// PID 0x11: Throttle Position
static std::vector<DecodedItem> DecodePID11(const uint8_t* data, uint8_t len);
// PID 0x12: Commanded Secondary Air Status
static std::vector<DecodedItem> DecodePID12(const uint8_t* data, uint8_t len);
// PID 0x13: O2 Sensor Present Bank 1 Sensor 1
static std::vector<DecodedItem> DecodePID13(const uint8_t* data, uint8_t len);
// PID 0x14 - 1B: O2 Sensor 1 to 8
static std::vector<DecodedItem> DecodePID14to1B(const uint8_t* data, uint8_t len);
// PID 0x1F: Run time since engine start
static std::vector<DecodedItem> DecodePID1F(const uint8_t* data, uint8_t len);
// PID 0x20: PIDs supported [21–40]
static std::vector<DecodedItem> DecodePID20(const uint8_t* data, uint8_t len);
// PID 0x21: Distance traveled with MIL on
static std::vector<DecodedItem> DecodePID21(const uint8_t* data, uint8_t len);
// PID 0x22: Fuel Rail Pressure (relative to manifold vacuum)
static std::vector<DecodedItem> DecodePID22(const uint8_t* data, uint8_t len);
// PID 0x23: Fuel Rail Gauge Pressure (diesel, gasoline direct injection)
static std::vector<DecodedItem> DecodePID23(const uint8_t* data, uint8_t len);
// PID 0x2C: Commanded EGR
static std::vector<DecodedItem> DecodePID2C(const uint8_t* data, uint8_t len);
// PID 0x2D: EGR Error
static std::vector<DecodedItem> DecodePID2D(const uint8_t* data, uint8_t len);
// PID 0x2E: Commanded Evaporative Purge
static std::vector<DecodedItem> DecodePID2E(const uint8_t* data, uint8_t len);
// PID 0x2F: Fuel Tank Level Input
static std::vector<DecodedItem> DecodePID2F(const uint8_t* data, uint8_t len);
// PID 0x30: Warm-ups Since Codes Cleared
static std::vector<DecodedItem> DecodePID30(const uint8_t* data, uint8_t len);
// PID 0x31: Distance traveled since codes cleared
static std::vector<DecodedItem> DecodePID31(const uint8_t* data, uint8_t len);
// PID 0x32: Evap System Vapor Pressure
static std::vector<DecodedItem> DecodePID32(const uint8_t* data, uint8_t len);
// PID 0x33: Absolute Barometric Pressure
static std::vector<DecodedItem> DecodePID33(const uint8_t* data, uint8_t len);
// PID 0x40: Supported PIDs 0x41–0x60
static std::vector<DecodedItem> DecodePID40(const uint8_t* data, uint8_t len);
// PID 0x41: Monitor status this drive cycle
static std::vector<DecodedItem> DecodePID41(const uint8_t* data, uint8_t len);
// PID 0x42: Control module voltage
static std::vector<DecodedItem> DecodePID42(const uint8_t* data, uint8_t len);
// PID 0x43: Absolute load value
static std::vector<DecodedItem> DecodePID43(const uint8_t* data, uint8_t len);
// PID 0x44: Commanded equivalence ratio
static std::vector<DecodedItem> DecodePID44(const uint8_t* data, uint8_t len);
// PID 0x45: Relative throttle position
static std::vector<DecodedItem> DecodePID45(const uint8_t* data, uint8_t len);
// PID 0x46: Ambient air temperature
static std::vector<DecodedItem> DecodePID46(const uint8_t* data, uint8_t len);
// PID 0x47: Absolute throttle position B
static std::vector<DecodedItem> DecodePID47(const uint8_t* data, uint8_t len);
// PID 0x48: Absolute throttle position C
static std::vector<DecodedItem> DecodePID48(const uint8_t* data, uint8_t len);
// PID 0x49: Accelerator pedal position D
static std::vector<DecodedItem> DecodePID49(const uint8_t* data, uint8_t len);
// PID 0x4A: Accelerator pedal position E
static std::vector<DecodedItem> DecodePID4A(const uint8_t* data, uint8_t len);
// PID 0x4B: Accelerator pedal position F
static std::vector<DecodedItem> DecodePID4B(const uint8_t* data, uint8_t len);
// PID 0x4C: Commanded throttle actuator
static std::vector<DecodedItem> DecodePID4C(const uint8_t* data, uint8_t len);
// PID 0x4D: Time run with MIL on
static std::vector<DecodedItem> DecodePID4D(const uint8_t* data, uint8_t len);
// PID 0x4E: Time since DTCs cleared
static std::vector<DecodedItem> DecodePID4E(const uint8_t* data, uint8_t len);

static const PIDEntry* getTable();

static const PIDEntry pidTable[];

static const size_t pidTableSize;

};

#endif
