#include <Wire.h>
#include <PAT9125E.h>
#include <math.h>

PAT9125E::PAT9125E(uint8_t address)
{
    this->address = address;
}

bool PAT9125E::getIsConnected(bool poll)
{
    if (!poll && this->hasPolled && this->isConnected)
        return true;
        
    this->hasPolled = true;
    uint16_t productId = 0;
    uint8_t versionId = 0;
    if (this->getProductInfo(productId, versionId))
    {
        if (productId == PAT9125E::ExpectedProductIdentifier && versionId == PAT9125E::ExpectedProductVersion)
        {
            Serial.printf("Laser sensor connected at address %x. Product Identifier: %u, Version: %u.\r\n", this->address, productId, versionId);
            this->isConnected = true;
            return true;
        }
        else
        {
            Serial.printf("Received invalid product information from laser sensor at address %x.\r\n", this->address);
            this->isConnected = false;
        }
    }
    else
    {
        Serial.printf("Unable to find laser sensor at address %x.\r\n", this->address);
        this->isConnected = false;
    }
    return false;
}

void PAT9125E::writeByte(uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(this->address);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

bool PAT9125E::readByte(uint8_t reg, uint8_t& output)
{
    Wire.beginTransmission(this->address);
    Wire.write(reg);
    Wire.endTransmission();

    Wire.requestFrom(this->address, uint8_t(1));

    if (Wire.available() >= 1)
    {
        this->isConnected = true;
        output = Wire.read();
        return true;
    }
    else
    {
        this->isConnected = false;
        return false;
    }
}

bool PAT9125E::readBytes(uint8_t reg1, uint8_t reg2, uint16_t& output)
{
    Wire.beginTransmission(this->address);
    Wire.write(reg1);
    Wire.write(reg2);
    Wire.endTransmission();

    Wire.requestFrom(this->address, uint8_t(2));

    if (Wire.available() >= 2)
    {
        this->isConnected = true;
        output = (Wire.read() << 8) | Wire.read();
        return true;
    }
    else
    {
        this->isConnected = false;
        return false;
    }
}

uint8_t PAT9125E::getAddress()
{
    return this->address;
}

uint16_t PAT9125E::getProductIdentifier()
{
    uint16_t identifier;
    uint8_t version;
    getProductInfo(identifier, version);
    return identifier;
}
uint8_t PAT9125E::getProductVersion()
{
    uint8_t version = 0;
    if (!this->readByte(PAT9125E_REG_PRODUCT_ID_2, version))
        return 0;

    return version & 0xF;
}
bool PAT9125E::getProductInfo(uint16_t& productIdentifier, uint8_t& productVersion)
{
    uint16_t info = 0;
    if (!this->readBytes(PAT9125E_REG_PRODUCT_ID_1, PAT9125E_REG_PRODUCT_ID_2, info))
        return false;

    productIdentifier = (info & 0xFF) | (info & 0xF000);
    productVersion = (info >> 8) & 0xF;
    return true;
}

bool PAT9125E::getMotionStatus()
{
    uint8_t motionStatus = 0;
    if (!this->readByte(PAT9125E_REG_MOTION_STATUS, motionStatus))
        return false;
    
    return motionStatus & (1 << 7) > 0;
}

PAT9125E_OperationModeFlags PAT9125E::getOperationFlags()
{
    uint8_t operationFlags = 0;
    if (!this->readByte(PAT9125E_REG_OPERATION_MODE, operationFlags))
        return PAT9125E_OperationModeFlags::None;

    return (PAT9125E_OperationModeFlags)operationFlags;
}

PAT9125E_ConfigurationFlags PAT9125E::getConfigurationFlags()
{
    uint8_t operationFlags = 0;
    if (!this->readByte(PAT9125E_REG_CONFIGURATION, operationFlags))
        return PAT9125E_ConfigurationFlags::None;

    return (PAT9125E_ConfigurationFlags)operationFlags;
}

PAT9125E_WriteProtectFlags PAT9125E::getWriteProtectionFlags()
{
    uint8_t operationFlags = 0;
    if (!this->readByte(PAT9125E_REG_WRITE_PROTECT, operationFlags))
        return PAT9125E_WriteProtectFlags::Protected;

    return (PAT9125E_WriteProtectFlags)operationFlags;
}

PAT9125E_OrientationFlags PAT9125E::getOrientationFlags()
{
    uint8_t operationFlags = 0;
    if (!this->readByte(PAT9125E_REG_ORIENTATION, operationFlags))
        return PAT9125E_OrientationFlags::None;

    this->is12BitMovement = (operationFlags & (uint8_t)PAT9125E_OrientationFlags::Motion12Bit) != 0;
    return (PAT9125E_OrientationFlags)operationFlags;
}

void PAT9125E::configureOperationFlags(PAT9125E_OperationModeFlags flags)
{
    this->writeByte(PAT9125E_REG_OPERATION_MODE, (uint8_t)flags);
}

void PAT9125E::configureConfigurationFlags(PAT9125E_ConfigurationFlags flags)
{
    this->writeByte(PAT9125E_REG_CONFIGURATION, (uint8_t)flags);
}

void PAT9125E::configureWriteProtectionFlags(PAT9125E_WriteProtectFlags flags)
{
    this->writeByte(PAT9125E_REG_WRITE_PROTECT, (uint8_t)flags);
}

void PAT9125E::configureOrientationFlags(PAT9125E_OrientationFlags flags)
{
    this->writeByte(PAT9125E_REG_ORIENTATION, uint8_t(flags));
    this->is12BitMovement = (uint8_t(flags) & uint8_t(PAT9125E_OrientationFlags::Motion12Bit)) != 0;
}

bool PAT9125E::getIsWriteProtected()
{
    PAT9125E_WriteProtectFlags flags = this->getWriteProtectionFlags();
    return ((uint8_t)flags & (uint8_t)PAT9125E_WriteProtectFlags::Protected) == (uint8_t)PAT9125E_WriteProtectFlags::Protected;
}

bool PAT9125E::getIs12BitMovement()
{
    this->getOrientationFlags();
    return this->is12BitMovement;
}

bool PAT9125E::getIsSleeping()
{
    PAT9125E_OperationModeFlags flags = this->getOperationFlags();
    return ((uint8_t)flags & (uint8_t)PAT9125E_OperationModeFlags::SleepState) != 0;
}

bool PAT9125E::getIsDeepSleeping()
{
    PAT9125E_OperationModeFlags flags = this->getOperationFlags();
    return ((uint8_t)flags & (uint8_t)PAT9125E_OperationModeFlags::DeepSleepState) != 0;
}

bool PAT9125E::getIsAwake()
{
    PAT9125E_OperationModeFlags flags = this->getOperationFlags();
    return ((uint8_t)flags & ((uint8_t)PAT9125E_OperationModeFlags::SleepState | (uint8_t)PAT9125E_OperationModeFlags::DeepSleepState)) == 0;
}
void PAT9125E::configureIsWriteProtected(bool isWriteProtected)
{
    this->configureWriteProtectionFlags(isWriteProtected ? PAT9125E_WriteProtectFlags::Protected : PAT9125E_WriteProtectFlags::NotProtected);
}
void PAT9125E::configure12BitMovement(bool is12Bit)
{
    uint8_t flags = (uint8_t)getOrientationFlags();
    uint8_t newFlags = is12Bit ? uint8_t(flags | (uint8_t)PAT9125E_OrientationFlags::Motion12Bit) : flags & uint8_t(~(uint8_t(PAT9125E_OrientationFlags::Motion12Bit)));
    if (newFlags != flags)
    {
        this->configureOrientationFlags(PAT9125E_OrientationFlags(newFlags));
        Serial.printf("Setting orientation flags: 0x%x.\r\n", newFlags);
    }
}
void PAT9125E::wakeup()
{
    this->configureOperationFlags(PAT9125E_OPERATION_FORCE_WAKEUP);
}
void PAT9125E::enterSleep()
{
    this->configureOperationFlags(PAT9125E_OPERATION_FORCE_SLEEP);
}
void PAT9125E::enterDeepSleep()
{
    this->configureOperationFlags(PAT9125E_OPERATION_FORCE_DEEP_SLEEP);
}
void PAT9125E::configureSleepModes(bool enableSleep, bool enableDeepSleep)
{
    uint8_t sleep = enableSleep ? 0 : (uint8_t)PAT9125E_OPERATION_SLEEP_ENABLED;
    sleep |= (enableDeepSleep ? 0 : (uint8_t)PAT9125E_OPERATION_DEEP_SLEEP_ENABLED);
    this->configureOperationFlags((PAT9125E_OperationModeFlags)sleep);
}
void PAT9125E::configureSleepTimings(uint8_t pollFrequencyMilliseconds, uint16_t timeoutMilliseconds)
{
    float pollTiming = roundf((float)pollFrequencyMilliseconds / float(PAT9125E_MS_PER_POLL_INDEX_SLEEP)) - 1;
    if (pollTiming < 0.0f)
        pollTiming = 0.0f;
    else if (pollTiming > 15.0f)
        pollTiming = 15.0f;
    float timeoutTiming = roundf((float)timeoutMilliseconds / float(PAT9125E_MS_PER_TIMEOUT_INDEX_SLEEP)) - 1;
    if (timeoutTiming < 0.0f)
        timeoutTiming = 0.0f;
    else if (timeoutTiming > 15.0f)
        timeoutTiming = 15.0f;
    uint8_t data = (static_cast<uint8_t>(pollTiming) << 8) |
                    static_cast<uint8_t>(timeoutTiming);
    this->writeByte(PAT9125E_REG_SLEEP, data);
}
void PAT9125E::configureDeepSleepTimings(uint16_t pollFrequencyMilliseconds, uint32_t timeoutMilliseconds)
{
    float pollTiming = roundf((float)pollFrequencyMilliseconds / float(PAT9125E_MS_PER_POLL_INDEX_DEEP_SLEEP)) - 1;
    if (pollTiming < 0.0f)
        pollTiming = 0.0f;
    else if (pollTiming > 15.0f)
        pollTiming = 15.0f;
    float timeoutTiming = roundf((float)timeoutMilliseconds / float(PAT9125E_MS_PER_TIMEOUT_INDEX_DEEP_SLEEP)) - 1;
    if (timeoutTiming < 0.0f)
        timeoutTiming = 0.0f;
    else if (timeoutTiming > 15.0f)
        timeoutTiming = 15.0f;
    uint8_t data = (static_cast<uint8_t>(pollTiming) << 8) |
                    static_cast<uint8_t>(timeoutTiming);
    this->writeByte(PAT9125E_REG_DEEP_SLEEP, data);
}
void PAT9125E::getSleepTimings(uint8_t& pollFrequencyMilliseconds, uint16_t& timeoutMilliseconds)
{
    uint8_t data = 0;
    if (!this->readByte(PAT9125E_REG_SLEEP, data))
        return;
    pollFrequencyMilliseconds = ((data >> 8) + 1) * PAT9125E_MS_PER_POLL_INDEX_SLEEP;
    timeoutMilliseconds = ((data & 0xF) + 1) * PAT9125E_MS_PER_TIMEOUT_INDEX_SLEEP;
}
void PAT9125E::getDeepSleepTimings(uint16_t& pollFrequencyMilliseconds, uint32_t& timeoutMilliseconds)
{
    uint8_t data = 0;
    if (!this->readByte(PAT9125E_REG_DEEP_SLEEP, data))
        return;
    pollFrequencyMilliseconds = (uint16_t)((data >> 8) + 1) * uint16_t(PAT9125E_MS_PER_POLL_INDEX_DEEP_SLEEP);
    timeoutMilliseconds = (uint32_t)((data & 0xF) + 1) * uint32_t(PAT9125E_MS_PER_TIMEOUT_INDEX_DEEP_SLEEP);
}
void PAT9125E::configureResolution(uint16_t resolutionX, uint16_t resolutionY)
{
    this->configureResolutionX(resolutionX);
    this->configureResolutionY(resolutionY);
}
void PAT9125E::configureResolutionX(uint16_t resolutionX)
{
    float resolution = roundf((float)resolutionX / float(PAT9125E_CPI_PER_RESOLUTION_INDEX));
    if (resolution < 0.0f)
        resolution = 0.0f;
    else if (resolution > 255.0f)
        resolution = 255.0f;
    this->writeByte(PAT9125E_REG_RESOLUTION_X, static_cast<uint8_t>(resolution));
}
void PAT9125E::configureResolutionY(uint16_t resolutionY)
{
    float resolution = roundf((float)resolutionY / float(PAT9125E_CPI_PER_RESOLUTION_INDEX));
    if (resolution < 0.0f)
        resolution = 0.0f;
    else if (resolution > 255.0f)
        resolution = 255.0f;
    this->writeByte(PAT9125E_REG_RESOLUTION_Y, static_cast<uint8_t>(resolution));
}
uint16_t PAT9125E::getResolutionX()
{
    uint8_t data = 0;
    if (!this->readByte(PAT9125E_REG_RESOLUTION_X, data))
        return 0;
    return (uint16_t)data * uint16_t(PAT9125E_CPI_PER_RESOLUTION_INDEX);
}
uint16_t PAT9125E::getResolutionY()
{
    uint8_t data = 0;
    if (!this->readByte(PAT9125E_REG_RESOLUTION_Y, data))
        return 0;
    return (uint16_t)data * uint16_t(PAT9125E_CPI_PER_RESOLUTION_INDEX);
}
void PAT9125E::getResolution(uint16_t& resolutionX, uint16_t& resolutionY)
{
    resolutionX = getResolutionX();
    resolutionY = getResolutionY();
}
int16_t PAT9125E::getDeltaMovementX()
{
    uint8_t data = 0;
    if (!this->readByte(PAT9125E_REG_DELTA_X_LOW, data))
        return 0;
    uint8_t highData = 0;
    if (!is12BitMovement || !this->readByte(PAT9125E_REG_DELTA_X_Y_HIGH, highData))
        return (int8_t)data;
    
    
    uint16_t deltaX = (uint16_t)data | (uint16_t)(highData >> 4) << 8;
    return deltaX >= 2048 ? -int16_t(4096 - deltaX) : (int16_t)deltaX;
}
int16_t PAT9125E::getDeltaMovementY()
{
    uint8_t data = 0;
    if (!this->readByte(PAT9125E_REG_DELTA_Y_LOW, data))
        return 0;
    uint8_t highData = 0;
    if (!is12BitMovement || !this->readByte(PAT9125E_REG_DELTA_X_Y_HIGH, highData))
        return (int8_t)data;
    
    uint16_t deltaY = (uint16_t)data | (uint16_t)(highData & 0xF) << 8;
    return deltaY >= 2048 ? -int16_t(4096 - deltaY) : (int16_t)deltaY;
}
void PAT9125E::getDeltaMovement(int16_t& deltaX, int16_t& deltaY)
{
    uint8_t dataX = 0;
    uint8_t dataY = 0;
    if (!this->readByte(PAT9125E_REG_DELTA_X_LOW, dataX) || !this->readByte(PAT9125E_REG_DELTA_Y_LOW, dataY))
        return;
    uint8_t highData = 0;
    if (!is12BitMovement || !this->readByte(PAT9125E_REG_DELTA_X_Y_HIGH, highData))
    {
        deltaX = (int8_t)dataX;
        deltaY = (int8_t)dataY;
    }
    else
    {
        uint16_t deltaX2 = (uint16_t)dataX | (uint16_t)(highData >> 4) << 8;
        uint16_t deltaY2 = (uint16_t)dataY | (uint16_t)(highData & 0xF) << 8;

        // apply overflow
        deltaX = deltaX2 >= 2048 ? -int16_t(4096 - deltaX2) : (int16_t)deltaX2;
        deltaY = deltaY2 >= 2048 ? -int16_t(4096 - deltaY2) : (int16_t)deltaY2;
    }
}

void PAT9125E::waitMovementCheckTime()
{
    delay(PAT9125E_MS_MOTION_READ_DELAY);
}

bool PAT9125E::getDeltaMovementIfReady(int16_t& deltaX, int16_t& deltaY)
{
    if (!this->getMotionStatus())
        return false;
    
    this->getDeltaMovement(deltaX, deltaY);
    return true;
}

uint8_t PAT9125E::getShutterTimeIndex()
{
    uint8_t shutterTime = 0;
    if (!this->readByte(PAT9125E_REG_SHUTTER, shutterTime))
        return 0;

    return shutterTime;
}

uint8_t PAT9125E::getAverageBrightness()
{
    uint8_t averageBrightness = 0;
    if (!this->readByte(PAT9125E_REG_FRAME_AVERAGE, averageBrightness))
        return 0;

    return averageBrightness;
}