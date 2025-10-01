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

/**
 * @class OBD2Scanner
 * @brief Provides an interface to scan and retrieve vehicle diagnostic information via OBD-II protocol.
 *
 * This class uses a CAN transport interface to communicate with the vehicle's ECU and provides
 * methods to retrieve diagnostic trouble codes (DTCs), freeze frame data, vehicle information, and more.
 */
class OBD2Scanner {
    ICANInterface* transport;    ///< Transport interface for CAN communication
    GenerateFrame* generator;    ///< Helper class for generating OBD-II request frames
    ReceiverFrames* receiver;    ///< Helper class for receiving and parsing OBD-II responses

public:
    /**
     * @brief Constructor for OBD2Scanner
     * @param transporter Pointer to an ICANInterface used for vehicle communication
     */
    OBD2Scanner(ICANInterface *transporter);

    /**
     * @brief Request a PID (Parameter ID) synchronously
     * @param pid The OBD-II PID to request
     * @return A vector of decoded items containing the PID response
     */
    std::vector<DecodedItem> getPid(uint8_t pid);

    /**
     * @brief Request a PID asynchronously
     * @param pid The OBD-II PID to request
     * @param callback Function to call with the decoded result
     */
    void getPid(uint8_t pid, std::function<void(const std::vector<DecodedItem>&)> callback);

    /**
     * @brief Retrieve stored Diagnostic Trouble Codes (DTCs)
     * @return A vector of decoded DTCs
     */
    std::vector<DecodedItem> getDTCs();

    /**
     * @brief Retrieve stored Diagnostic Trouble Codes (DTCs) asynchronously
     * @param callback Function to call with the decoded DTCs
     */
    void getDTCs(std::function<void(const std::vector<DecodedItem>&)> callback);

    /**
     * @brief Retrieve freeze frame data for a given PID
     * @param pid The PID for which freeze frame data is requested
     * @return A vector of decoded freeze frame data
     */
    std::vector<DecodedItem> getFreezFrame(uint8_t pid);

    /**
     * @brief Retrieve freeze frame data asynchronously
     * @param pid The PID for which freeze frame data is requested
     * @param callback Function to call with the decoded data
     */
    void getFreezFrame(uint8_t pid, std::function<void(const std::vector<DecodedItem>&)> callback);

    /**
     * @brief Retrieve permanent Diagnostic Trouble Codes (DTCs)
     * @return A vector of permanent DTCs
     */
    std::vector<DecodedItem> getPermanentDTCs();

    /**
     * @brief Retrieve permanent DTCs asynchronously
     * @param callback Function to call with the permanent DTCs
     */
    void getPermanentDTCs(std::function<void(const std::vector<DecodedItem>&)> callback);

    /**
     * @brief Retrieve pending Diagnostic Trouble Codes (DTCs)
     * @return A vector of pending DTCs
     */
    std::vector<DecodedItem> getPendingDTCs();

    /**
     * @brief Retrieve pending DTCs asynchronously
     * @param callback Function to call with the pending DTCs
     */
    void getPendingDTCs(std::function<void(const std::vector<DecodedItem>&)> callback);

    /**
     * @brief Retrieve vehicle information based on a given PID
     * @param pid The vehicle information PID to request
     * @return A vector of decoded vehicle info
     */
    std::vector<DecodedItem> getVehicleInfo(uint8_t pid);

    /**
     * @brief Retrieve vehicle information asynchronously
     * @param pid The vehicle information PID to request
     * @param callback Function to call with the decoded info
     */
    void getVehicleInfo(uint8_t pid, std::function<void(const std::vector<DecodedItem>&)> callback);

    /**
     * @brief Clear stored Diagnostic Trouble Codes (DTCs)
     * @return A vector of decoded response items
     */
    std::vector<DecodedItem> ClearDTCs();

    /**
     * @brief Clear DTCs asynchronously
     * @param callback Function to call with the response
     */
    void ClearDTCs(std::function<void(const std::vector<DecodedItem>&)> callback);

    /**
     * @brief Get the current communication protocol
     * @return A string representing the protocol (e.g., "ISO 15765-4 (CAN)")
     */
    std::string getProtocol();

    /**
     * @brief Destructor for OBD2Scanner
     */
    ~OBD2Scanner();
};

#endif