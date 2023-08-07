#include <Arduino.h>
#include <Wire.h>
#include <PAT9125E.h>

#define PIN_LASER_SENSOR_SCL 32
#define PIN_LASER_SENSOR_SDA 33

#define PIN_TEST_BUTTON 0

#define LASER_SENSOR_ADDRESS uint8_t(0x75)

static PAT9125E* LaserSensor;

static bool laserSensorPresent;

void setup()
{
  Serial.begin(115200);
  Wire.setPins(PIN_LASER_SENSOR_SDA, PIN_LASER_SENSOR_SCL);
  Wire.begin();

  LaserSensor = new PAT9125E(LASER_SENSOR_ADDRESS);
  laserSensorPresent = LaserSensor->getIsConnected(true);
  if (!laserSensorPresent) while (true);

  Serial.println("== LASER SENSOR PRINTOUT ==");
  Serial.printf("Address: %x\r\n", LaserSensor->getAddress());
  uint16_t deepPollFreq;
  uint32_t deepTimeout;
  LaserSensor->getDeepSleepTimings(deepPollFreq, deepTimeout);
  Serial.printf("(DEEP SLEEP) Poll frequency: %u, Timeout: %u\r\n", deepPollFreq, deepTimeout);
  uint8_t pollFreq;
  uint16_t timeout;
  LaserSensor->getSleepTimings(pollFreq, timeout);
  Serial.printf("(SLEEP) Poll frequency: %u, Timeout: %u.\r\n", pollFreq, timeout);
  Serial.printf("Configuration: 0x%x.\r\n", LaserSensor->getConfigurationFlags());
  Serial.printf("Is write protected: %u.\r\n", LaserSensor->getIsWriteProtected());
  Serial.printf("Is 12 bit: %u.\r\n", LaserSensor->getIs12BitMovement());
  Serial.printf("Operation flags: 0x%x.\r\n", LaserSensor->getOperationFlags());
  Serial.printf("Orientation flags: 0x%x.\r\n", LaserSensor->getOrientationFlags());
  uint16_t resX, resY;
  LaserSensor->getResolution(resX, resY);
  Serial.printf("Resolution: %u x %u.\r\n", resX, resY);

  delay(2000);
}

uint32_t lastMovementCheck;
int32_t totalX = 0;
int32_t totalY = 0;
void loop()
{
  if (digitalRead(PIN_TEST_BUTTON) == LOW)
  {
    totalX = 0; totalY = 0;
  }
  uint32_t mil = millis();
  if (mil > lastMovementCheck + 5)
  {
    lastMovementCheck = mil;
    int16_t dx, dy;
    do
    {
      PAT9125E::waitMovementCheckTime();
    } while (!LaserSensor->getDeltaMovementIfReady(dx, dy));

    totalX += dx;
    totalY += dy;

    Serial.printf("Delta movement: (%i, %i) Total: (%i, %i).\r\n", dx, dy, totalX, totalY);
  }
}