#pragma once

#include <Arduino.h>

#define PAT9125E_REG_PRODUCT_ID_1 0x00
#define PAT9125E_REG_PRODUCT_ID_2 0x01
#define PAT9125E_REG_MOTION_STATUS 0x02
#define PAT9125E_REG_DELTA_X_LOW 0x03
#define PAT9125E_REG_DELTA_Y_LOW 0x04
#define PAT9125E_REG_OPERATION_MODE 0x05
#define PAT9125E_REG_CONFIGURATION 0x06
#define PAT9125E_REG_WRITE_PROTECT 0x09
#define PAT9125E_REG_SLEEP 0x0A
#define PAT9125E_REG_DEEP_SLEEP 0x0B
#define PAT9125E_REG_RESOLUTION_X 0x0D
#define PAT9125E_REG_RESOLUTION_Y 0x0E
#define PAT9125E_REG_DELTA_X_Y_HIGH 0x12
#define PAT9125E_REG_SHUTTER 0x14
#define PAT9125E_REG_FRAME_AVERAGE 0x17
#define PAT9125E_REG_ORIENTATION 0x19

#define PAT9125E_OPERATION_DISABLE_SLEEP_MODES PAT9125E_OperationModeFlags(0)
#define PAT9125E_OPERATION_SLEEP_ENABLED PAT9125E_OperationModeFlags(0b10000)
#define PAT9125E_OPERATION_DEEP_SLEEP_ENABLED PAT9125E_OperationModeFlags(0b11000)
#define PAT9125E_OPERATION_FORCE_SLEEP PAT9125E_OperationModeFlags(0b11010)
#define PAT9125E_OPERATION_FORCE_DEEP_SLEEP PAT9125E_OperationModeFlags(0b11100)
#define PAT9125E_OPERATION_FORCE_WAKEUP PAT9125E_OperationModeFlags(0b11001)

#define PAT9125E_MS_PER_POLL_INDEX_SLEEP 4
#define PAT9125E_MS_PER_TIMEOUT_INDEX_SLEEP 32

#define PAT9125E_MS_PER_POLL_INDEX_DEEP_SLEEP 64
#define PAT9125E_MS_PER_TIMEOUT_INDEX_DEEP_SLEEP 204800

#define PAT9125E_CPI_PER_RESOLUTION_INDEX 5

#define PAT9125E_MS_MOTION_READ_DELAY 8

enum class PAT9125E_OperationModeFlags : uint8_t {
    None = 0,
    WakeupSet = 1,
    SleepSet = 1 << 1,
    DeepSleepSet = 1 << 2,
    DeepSleepState = 1 << 3,
    SleepState = 1 << 4
};

enum class PAT9125E_ConfigurationFlags : uint8_t {
    None = 0,
    Reset = 1 << 7,
    EnterPowerDownMode = 1 << 3
};

enum class PAT9125E_WriteProtectFlags : uint8_t {
    Protected = 0x00,
    NotProtected = 0x5A
};

enum class PAT9125E_OrientationFlags : uint8_t {
    None = 0,
    Motion12Bit = 1 << 2,
    InvertX = 1 << 3,
    InvertY = 1 << 4,
    SwapXY = 1 << 5
};

class PAT9125E
{
protected:
    bool is12BitMovement = true;
    bool isConnected = false;
    bool hasPolled = false;
    uint8_t address;
public:
    const uint8_t ExpectedProductVersion = 1;
    const uint16_t ExpectedProductIdentifier = 12433;
    
    static void waitMovementCheckTime();

    PAT9125E(uint8_t address);

    bool getIsConnected(bool repoll);
    uint8_t getAddress();

    bool getProductInfo(uint16_t& productIdentifier, uint8_t& productVersion);
    uint8_t getProductVersion();
    uint16_t getProductIdentifier();
    bool getMotionStatus();
    bool getIsSleeping();
    bool getIsDeepSleeping();
    bool getIsAwake();

    void configureOperationFlags(PAT9125E_OperationModeFlags flags);
    void configureConfigurationFlags(PAT9125E_ConfigurationFlags flags);
    void configureWriteProtectionFlags(PAT9125E_WriteProtectFlags flags);
    void configureOrientationFlags(PAT9125E_OrientationFlags flags);

    void configureIsWriteProtected(bool isWriteProtected);
    void configure12BitMovement(bool is12Bit);

    void wakeup();
    void enterSleep();
    void enterDeepSleep();
    void configureSleepModes(bool enableSleep, bool enableDeepSleep);
    
    void configureSleepTimings(uint8_t pollFrequencyMilliseconds, uint16_t timeoutMilliseconds);
    void configureDeepSleepTimings(uint16_t pollFrequencyMilliseconds, uint32_t timeoutMilliseconds);

    void configureResolutionX(uint16_t resolutionX);
    void configureResolutionY(uint16_t resolutionY);
    void configureResolution(uint16_t resolutionX, uint16_t resolutionY);

    int16_t getDeltaMovementX();
    int16_t getDeltaMovementY();
    void getDeltaMovement(int16_t& deltaX, int16_t& deltaY);
    bool getDeltaMovementIfReady(int16_t& deltaX, int16_t& deltaY);

    uint8_t getShutterTimeIndex();
    uint8_t getAverageBrightness();
    
    uint16_t getResolutionX();
    uint16_t getResolutionY();
    void getResolution(uint16_t& resolutionX, uint16_t& resolutionY);

    PAT9125E_OperationModeFlags getOperationFlags();
    PAT9125E_ConfigurationFlags getConfigurationFlags();
    PAT9125E_WriteProtectFlags getWriteProtectionFlags();
    PAT9125E_OrientationFlags getOrientationFlags();
    
    bool getIsWriteProtected();
    bool getIs12BitMovement();
    
    void getSleepTimings(uint8_t& pollFrequencyMilliseconds, uint16_t& timeoutMilliseconds);
    void getDeepSleepTimings(uint16_t& pollFrequencyMilliseconds, uint32_t& timeoutMilliseconds);
    
    bool readByte(uint8_t reg, uint8_t& output);
    bool readBytes(uint8_t reg1, uint8_t reg2, uint16_t& output);
    void writeByte(uint8_t reg, uint8_t data);
};