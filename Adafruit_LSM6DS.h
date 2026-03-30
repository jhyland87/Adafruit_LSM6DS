/*!
 *  @file Adafruit_LSM6DS.h
 *
 * 	I2C Driver base for Adafruit LSM6DSxx 6-DoF Accelerometer and Gyroscope
 *      library
 *
 * 	Adafruit invests time and resources providing this open source code,
 *      please support Adafruit and open-source hardware by purchasing products
 *from Adafruit!
 *
 *	BSD license (see license.txt)
 */

#ifndef _ADAFRUIT_LSM6DS_H
#define _ADAFRUIT_LSM6DS_H

#include "Arduino.h"
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define LSM6DS_I2CADDR_DEFAULT 0x6A ///< LSM6DS default i2c address

#define LSM6DS_FUNC_CFG_ACCESS 0x1 ///< Enable embedded functions register
#define LSM6DS_INT1_CTRL 0x0D      ///< Interrupt control for INT 1
#define LSM6DS_INT2_CTRL 0x0E      ///< Interrupt control for INT 2
#define LSM6DS_WHOAMI 0x0F         ///< Chip ID register
#define LSM6DS_CTRL1_XL 0x10       ///< Main accelerometer config register
#define LSM6DS_CTRL2_G 0x11        ///< Main gyro config register
#define LSM6DS_CTRL3_C 0x12        ///< Main configuration register
#define LSM6DS_CTRL8_XL 0x17       ///< High and low pass for accel
#define LSM6DS_CTRL10_C 0x19       ///< Main configuration register
#define LSM6DS_WAKEUP_SRC 0x1B     ///< Why we woke up
#define LSM6DS_STATUS_REG 0X1E     ///< Status register
#define LSM6DS_OUT_TEMP_L 0x20     ///< First data register (temperature low)
#define LSM6DS_OUTX_L_G 0x22       ///< First gyro data register
#define LSM6DS_OUTX_L_A 0x28       ///< First accel data register
#define LSM6DS_STEPCOUNTER 0x4B    ///< 16-bit step counter
#define LSM6DS_TAP_CFG 0x58        ///< Tap/pedometer configuration
#define LSM6DS_WAKEUP_THS                                                      \
  0x5B ///< Single and double-tap function threshold register
#define LSM6DS_WAKEUP_DUR                                                      \
  0x5C ///< Free-fall, wakeup, timestamp and sleep mode duration
#define LSM6DS_MD1_CFG 0x5E ///< Functions routing on INT1 register

#define LSM6DS_FIFO_CTRL1 0x07 ///< FIFO control register 1 (watermark low)
#define LSM6DS_FIFO_CTRL2 0x08 ///< FIFO control register 2 (watermark high)
#define LSM6DS_FIFO_CTRL3 0x09 ///< FIFO control register 3 (batch data rates)
#define LSM6DS_FIFO_CTRL4 0x0A ///< FIFO control register 4 (FIFO mode)
#define LSM6DS_FIFO_STATUS1 0x3A ///< FIFO status register 1
#define LSM6DS_FIFO_STATUS2 0x3B ///< FIFO status register 2
#define LSM6DS_FIFO_DATA_OUT_TAG 0x78 ///< FIFO data out tag register
#define LSM6DS_FIFO_DATA_OUT_X_L 0x79 ///< FIFO data output X low byte

/** The accelerometer data rate */
typedef enum data_rate {
  LSM6DS_RATE_SHUTDOWN,
  LSM6DS_RATE_12_5_HZ,
  LSM6DS_RATE_26_HZ,
  LSM6DS_RATE_52_HZ,
  LSM6DS_RATE_104_HZ,
  LSM6DS_RATE_208_HZ,
  LSM6DS_RATE_416_HZ,
  LSM6DS_RATE_833_HZ,
  LSM6DS_RATE_1_66K_HZ,
  LSM6DS_RATE_3_33K_HZ,
  LSM6DS_RATE_6_66K_HZ,
} lsm6ds_data_rate_t;

/** The accelerometer data range */
typedef enum accel_range {
  LSM6DS_ACCEL_RANGE_2_G,
  LSM6DS_ACCEL_RANGE_16_G,
  LSM6DS_ACCEL_RANGE_4_G,
  LSM6DS_ACCEL_RANGE_8_G
} lsm6ds_accel_range_t;

/** The gyro data range */
typedef enum gyro_range {
  LSM6DS_GYRO_RANGE_125_DPS = 0b0010,
  LSM6DS_GYRO_RANGE_250_DPS = 0b0000,
  LSM6DS_GYRO_RANGE_500_DPS = 0b0100,
  LSM6DS_GYRO_RANGE_1000_DPS = 0b1000,
  LSM6DS_GYRO_RANGE_2000_DPS = 0b1100,
  ISM330DHCX_GYRO_RANGE_4000_DPS = 0b0001
} lsm6ds_gyro_range_t;

/** The high pass filter bandwidth */
typedef enum hpf_range {
  LSM6DS_HPF_ODR_DIV_50 = 0,
  LSM6DS_HPF_ODR_DIV_100 = 1,
  LSM6DS_HPF_ODR_DIV_9 = 2,
  LSM6DS_HPF_ODR_DIV_400 = 3,
} lsm6ds_hp_filter_t;

/** FIFO operating modes */
typedef enum fifo_mode {
  LSM6DS_FIFO_BYPASS = 0b000,              ///< FIFO disabled
  LSM6DS_FIFO_MODE = 0b001,                ///< Stop collecting when full
  LSM6DS_FIFO_CONTINUOUS_TO_FIFO = 0b011,  ///< Continuous until trigger, then FIFO
  LSM6DS_FIFO_BYPASS_TO_CONTINUOUS = 0b100, ///< Bypass until trigger, then continuous
  LSM6DS_FIFO_CONTINUOUS = 0b110,           ///< Continuous mode (overwrite old data)
  LSM6DS_FIFO_BYPASS_TO_FIFO = 0b111,      ///< Bypass until trigger, then FIFO
} lsm6ds_fifo_mode_t;

/** FIFO batch data rate (BDR) for accel/gyro */
typedef enum fifo_data_rate {
  LSM6DS_FIFO_RATE_DISABLED = 0b0000, ///< Not batched in FIFO
  LSM6DS_FIFO_RATE_12_5_HZ = 0b0001, ///< 12.5 Hz
  LSM6DS_FIFO_RATE_26_HZ = 0b0010,   ///< 26 Hz
  LSM6DS_FIFO_RATE_52_HZ = 0b0011,   ///< 52 Hz
  LSM6DS_FIFO_RATE_104_HZ = 0b0100,  ///< 104 Hz
  LSM6DS_FIFO_RATE_208_HZ = 0b0101,  ///< 208 Hz
  LSM6DS_FIFO_RATE_416_HZ = 0b0110,  ///< 416 Hz
  LSM6DS_FIFO_RATE_833_HZ = 0b0111,  ///< 833 Hz
  LSM6DS_FIFO_RATE_1_66K_HZ = 0b1000, ///< 1.66 kHz
  LSM6DS_FIFO_RATE_3_33K_HZ = 0b1001, ///< 3.33 kHz
  LSM6DS_FIFO_RATE_6_66K_HZ = 0b1010, ///< 6.66 kHz
} lsm6ds_fifo_data_rate_t;

/** FIFO timestamp batch decimation */
typedef enum fifo_timestamp_batch {
  LSM6DS_FIFO_TS_BATCH_DISABLED = 0b00, ///< Timestamp not batched
  LSM6DS_FIFO_TS_BATCH_DEC_1 = 0b01,    ///< Timestamp at every BDR sample
  LSM6DS_FIFO_TS_BATCH_DEC_8 = 0b10,    ///< Timestamp every 8 BDR samples
  LSM6DS_FIFO_TS_BATCH_DEC_32 = 0b11,   ///< Timestamp every 32 BDR samples
} lsm6ds_fifo_ts_batch_t;

/** FIFO data tag identifying the sensor source */
typedef enum fifo_tag {
  LSM6DS_FIFO_TAG_GYRO_NC = 0x01,    ///< Gyroscope data
  LSM6DS_FIFO_TAG_ACCEL_NC = 0x02,   ///< Accelerometer data
  LSM6DS_FIFO_TAG_TEMP = 0x03,       ///< Temperature data
  LSM6DS_FIFO_TAG_TIMESTAMP = 0x04,  ///< Timestamp data
} lsm6ds_fifo_tag_t;

class Adafruit_LSM6DS;

/** Adafruit Unified Sensor interface for temperature component of LSM6DS */
class Adafruit_LSM6DS_Temp : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the temp sensor
      @param parent A pointer to the LSM6DS class */
  Adafruit_LSM6DS_Temp(Adafruit_LSM6DS *parent) { _theLSM6DS = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x6D0;
  Adafruit_LSM6DS *_theLSM6DS = NULL;
};

/** Adafruit Unified Sensor interface for accelerometer component of LSM6DS */
class Adafruit_LSM6DS_Accelerometer : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the accelerometer
     sensor
      @param parent A pointer to the LSM6DS class */
  Adafruit_LSM6DS_Accelerometer(Adafruit_LSM6DS *parent) {
    _theLSM6DS = parent;
  }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x6D1;
  Adafruit_LSM6DS *_theLSM6DS = NULL;
};

/** Adafruit Unified Sensor interface for gyro component of LSM6DS */
class Adafruit_LSM6DS_Gyro : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the gyro sensor
      @param parent A pointer to the LSM6DS class */
  Adafruit_LSM6DS_Gyro(Adafruit_LSM6DS *parent) { _theLSM6DS = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x6D2;
  Adafruit_LSM6DS *_theLSM6DS = NULL;
};

/*!
 *    @brief  Base class for use with LSM6DS series acclerometer gyro sensors
 * from STMicroelectronics. DO NOT USE DIRECTLY. Specific sensor variants should
 * be subclassed as needed.
 */
class Adafruit_LSM6DS {
public:
  Adafruit_LSM6DS();
  virtual ~Adafruit_LSM6DS();

  bool begin_I2C(uint8_t i2c_addr = LSM6DS_I2CADDR_DEFAULT,
                 TwoWire *wire = &Wire, int32_t sensorID = 0);

  bool begin_SPI(uint8_t cs_pin, SPIClass *theSPI = &SPI, int32_t sensorID = 0,
                 uint32_t frequency = 1000000);
  bool begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                 int8_t mosi_pin, int32_t sensorID = 0,
                 uint32_t frequency = 1000000);

  bool getEvent(sensors_event_t *accel, sensors_event_t *gyro,
                sensors_event_t *temp);

  lsm6ds_data_rate_t getAccelDataRate(void);
  void setAccelDataRate(lsm6ds_data_rate_t data_rate);

  lsm6ds_accel_range_t getAccelRange(void);
  void setAccelRange(lsm6ds_accel_range_t new_range);

  lsm6ds_data_rate_t getGyroDataRate(void);
  void setGyroDataRate(lsm6ds_data_rate_t data_rate);

  lsm6ds_gyro_range_t getGyroRange(void);
  void setGyroRange(lsm6ds_gyro_range_t new_range);

  void reset(void);
  void configIntOutputs(bool active_low, bool open_drain);
  void configInt1(bool drdy_temp, bool drdy_g, bool drdy_xl,
                  bool step_detect = false, bool wakeup = false);
  void configInt2(bool drdy_temp, bool drdy_g, bool drdy_xl);
  void highPassFilter(bool enabled, lsm6ds_hp_filter_t filter);

  void enableWakeup(bool enable, uint8_t duration = 0, uint8_t thresh = 20);
  bool awake(void);
  bool shake(void);

  void enablePedometer(bool enable);
  void resetPedometer(void);
  uint16_t readPedometer(void);

  void setFIFOMode(lsm6ds_fifo_mode_t mode);
  lsm6ds_fifo_mode_t getFIFOMode(void);
  void setFIFOWatermark(uint16_t watermark);
  uint16_t getFIFOWatermark(void);
  void setFIFOAccelBatchRate(lsm6ds_fifo_data_rate_t rate);
  void setFIFOGyroBatchRate(lsm6ds_fifo_data_rate_t rate);
  uint16_t getFIFOCount(void);
  bool getFIFOFull(void);
  bool getFIFOOverrun(void);
  bool readFIFOWord(lsm6ds_fifo_tag_t &tag, int16_t &x, int16_t &y,
                    int16_t &z);
  void resetFIFO(void);
  void enableFIFOTimestamp(lsm6ds_fifo_ts_batch_t decimation);
  void enableTimestamp(bool enable);

  // Arduino compatible API
  int readAcceleration(float &x, float &y, float &z);
  float accelerationSampleRate(void);
  int accelerationAvailable(void);

  int readGyroscope(float &x, float &y, float &z);
  float gyroscopeSampleRate(void);
  int gyroscopeAvailable(void);

  int16_t rawAccX, ///< Last reading's raw accelerometer X axis
      rawAccY,     ///< Last reading's raw accelerometer Y axis
      rawAccZ,     ///< Last reading's raw accelerometer Z axis
      rawTemp,     ///< Last reading's raw temperature reading
      rawGyroX,    ///< Last reading's raw gyro X axis
      rawGyroY,    ///< Last reading's raw gyro Y axis
      rawGyroZ;    ///< Last reading's raw gyro Z axis

  float temperature, ///< Last reading's temperature (C)
      accX,          ///< Last reading's accelerometer X axis m/s^2
      accY,          ///< Last reading's accelerometer Y axis m/s^2
      accZ,          ///< Last reading's accelerometer Z axis m/s^2
      gyroX,         ///< Last reading's gyro X axis in rad/s
      gyroY,         ///< Last reading's gyro Y axis in rad/s
      gyroZ;         ///< Last reading's gyro Z axis in rad/s

  Adafruit_Sensor *getTemperatureSensor(void);
  Adafruit_Sensor *getAccelerometerSensor(void);
  Adafruit_Sensor *getGyroSensor(void);

protected:
  uint8_t chipID(void);
  uint8_t status(void);
  virtual void _read(void);
  virtual bool _init(int32_t sensor_id);

  uint16_t _sensorid_accel, ///< ID number for accelerometer
      _sensorid_gyro,       ///< ID number for gyro
      _sensorid_temp;       ///< ID number for temperature

  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
  Adafruit_SPIDevice *spi_dev = NULL; ///< Pointer to SPI bus interface

  float temperature_sensitivity =
      256.0; ///< Temp sensor sensitivity in LSB/degC
  Adafruit_LSM6DS_Temp *temp_sensor = NULL; ///< Temp sensor data object
  Adafruit_LSM6DS_Accelerometer *accel_sensor =
      NULL;                                 ///< Accelerometer data object
  Adafruit_LSM6DS_Gyro *gyro_sensor = NULL; ///< Gyro data object

  //! buffer for the accelerometer range
  lsm6ds_accel_range_t accelRangeBuffered = LSM6DS_ACCEL_RANGE_2_G;
  //! buffer for the gyroscope range
  lsm6ds_gyro_range_t gyroRangeBuffered = LSM6DS_GYRO_RANGE_250_DPS;

private:
  friend class Adafruit_LSM6DS_Temp; ///< Gives access to private members to
                                     ///< Temp data object
  friend class Adafruit_LSM6DS_Accelerometer; ///< Gives access to private
                                              ///< members to Accelerometer data
                                              ///< object
  friend class Adafruit_LSM6DS_Gyro; ///< Gives access to private members to
                                     ///< Gyro data object

  void fillTempEvent(sensors_event_t *temp, uint32_t timestamp);
  void fillAccelEvent(sensors_event_t *accel, uint32_t timestamp);
  void fillGyroEvent(sensors_event_t *gyro, uint32_t timestamp);
};

#endif
