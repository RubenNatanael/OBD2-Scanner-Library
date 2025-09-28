#ifndef OBD2_SCANNER
#define OBD2_SCANNER

#include <chrono>
#include <functional>

#include "MultiLogger.h"
#include "CustomSocket.h"
#include "GenerateFrame.h"
#include "ReceiverFrames.h"

#include <cstdint>

namespace OBD2 {

    // Mode 01 PIDs (current data)
    enum Pid : uint8_t {
        Supported0                 = 0x00,
        MonitorStatusSinceDTC      = 0x01,
        FreezeDTC                  = 0x02,
        FuelSystemStatus           = 0x03,
        CalculatedEngineLoad       = 0x04,
        CoolantTemp                = 0x05,
        ShortTermFuelTrimBank1     = 0x06,
        LongTermFuelTrimBank1      = 0x07,
        ShortTermFuelTrimBank2     = 0x08,
        LongTermFuelTrimBank2      = 0x09,
        FuelPressure               = 0x0A,
        IntakeManifoldPressure     = 0x0B,
        EngineRPM                  = 0x0C,
        VehicleSpeed               = 0x0D,
        TimingAdvance              = 0x0E,
        IntakeAirTemp              = 0x0F,
        MAF_AirFlowRate            = 0x10,
        ThrottlePosition           = 0x11,
        CommandedSecondaryAir      = 0x12,
        OxygenSensorsPresent       = 0x13,
        O2Sensor1_Voltage          = 0x14,
        O2Sensor2_Voltage          = 0x15,
        O2Sensor3_Voltage          = 0x16,
        O2Sensor4_Voltage          = 0x17,
        O2Sensor5_Voltage          = 0x18,
        O2Sensor6_Voltage          = 0x19,
        O2Sensor7_Voltage          = 0x1A,
        O2Sensor8_Voltage          = 0x1B,
        OBD_Standards              = 0x1C,
        OxygenSensorsPresent2      = 0x1D,
        AuxiliaryInputStatus       = 0x1E,
        Supported20                = 0x20,
        DistanceWithMILOn          = 0x21,
        FuelRailPressureVac        = 0x22,
        FuelRailPressureDirect     = 0x23,
        CommandedEGR               = 0x2C,
        EGR_Error                  = 0x2D,
        CommandedEvapPurge         = 0x2E,
        FuelLevelInput             = 0x2F,
        WarmupsSinceClear          = 0x30,
        DistanceSinceClear         = 0x31,
        EvapVaporPressure          = 0x32,
        BarometricPressure         = 0x33,
        Supported40                = 0x40,
        MonitorStatusThisCycle     = 0x41,
        ControlModuleVoltage       = 0x42,
        AbsoluteLoadValue          = 0x43,
        CommandedAirFuelEqRatio    = 0x44,
        RelativeThrottlePosition   = 0x45,
        AmbientAirTemp             = 0x46,
        AbsoluteThrottleBPosB      = 0x47,
        AbsoluteThrottleCPosC      = 0x48,
        AbsoluteThrottleDPosD      = 0x49,
        AbsoluteThrottleEPosE      = 0x4A,
        AbsoluteThrottleFPosF      = 0x4B,
        CommandedThrottleActuator  = 0x4C,
        TimeWithMILOn              = 0x4D,
        TimeSinceClear             = 0x4E,
    };

} // namespace OBD2


class OBD2Scanner {

    ICANInterface* transport;
    GenerateFrame* generator;
    ReceiverFrames* receiver;

    public:
        OBD2Scanner(ICANInterface *transporter);
        std::vector<DecodedItem> getPid(uint8_t pid);
        void getPid(uint8_t pid, std::function<void(const std::vector<DecodedItem>&)> callback);
        std::vector<DecodedItem> getDTCs();
        void getDTCs(std::function<void(const std::vector<DecodedItem>&)> callback);
        std::vector<DecodedItem> getFreezFrame(uint8_t pid);
        void getFreezFrame(uint8_t pid, std::function<void(const std::vector<DecodedItem>&)> callback);
        std::vector<DecodedItem> getPermanentDTCs();
        void getPermanentDTCs(std::function<void(const std::vector<DecodedItem>&)> callback);
        std::vector<DecodedItem> getPendingDTCs();
        void getPendingDTCs(std::function<void(const std::vector<DecodedItem>&)> callback);
        std::vector<DecodedItem> ClearDTCs();
        void ClearDTCs(std::function<void(const std::vector<DecodedItem>&)> callback);

        std::string getProtocol();
        
        ~OBD2Scanner();
};

#endif