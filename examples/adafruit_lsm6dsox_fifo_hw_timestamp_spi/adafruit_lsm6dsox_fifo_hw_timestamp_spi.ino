// FIFO with hardware timestamps - LSM6DSOX (SPI)
//
// The LSM6DSOX has an internal 25-microsecond-resolution timestamp counter
// that can be batched directly into the FIFO alongside sensor data.
// Each timestamp word gives you a 32-bit tick count (25 us per tick),
// which you can convert to milliseconds or seconds.
//
// This is more accurate than calculating from the data rate, especially
// when mixing different sensor rates or correlating with external events.
//
// This version uses the SPI interface for faster bus throughput.
// Output is CSV-formatted for easy graphing.

#include <Adafruit_LSM6DSOX.h>

// For hardware SPI mode, we need a CS pin
#define LSM_CS 10
// For software-SPI mode we also need SCK/MOSI/MISO pins
#define LSM_SCK 13
#define LSM_MISO 12
#define LSM_MOSI 11

Adafruit_LSM6DSOX sox;

// Current hardware timestamp in 25us ticks, updated as we read FIFO
uint32_t current_hw_timestamp = 0;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("LSM6DSOX FIFO - Hardware Timestamps (SPI)");

  if (!sox.begin_SPI(LSM_CS)) {
    // if (!sox.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
    Serial.println("Failed to find LSM6DSOX chip");
    while (1) {
      delay(10);
    }
  }

  sox.setAccelDataRate(LSM6DS_RATE_104_HZ);
  sox.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  sox.setGyroDataRate(LSM6DS_RATE_104_HZ);
  sox.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);

  // Batch both sensors into FIFO at 104 Hz
  sox.setFIFOAccelBatchRate(LSM6DS_FIFO_RATE_104_HZ);
  sox.setFIFOGyroBatchRate(LSM6DS_FIFO_RATE_104_HZ);

  // Enable the internal hardware timestamp counter
  sox.enableTimestamp(true);

  // Batch a timestamp into the FIFO at every BDR cycle (no decimation).
  // Use DEC_8 or DEC_32 to reduce FIFO usage at the cost of fewer
  // timestamp markers.
  sox.enableFIFOTimestamp(LSM6DS_FIFO_TS_BATCH_DEC_1);

  // Continuous mode overwrites oldest data when full
  sox.setFIFOMode(LSM6DS_FIFO_CONTINUOUS);

  // Print CSV header
  Serial.println();
  Serial.println("timestamp_ms,type,x,y,z");
}

void loop() {
  uint16_t count = sox.getFIFOCount();
  if (count == 0) {
    delay(10);
    return;
  }

  lsm6ds_fifo_tag_t tag;
  int16_t x, y, z;

  while (sox.readFIFOWord(tag, x, y, z)) {
    switch (tag) {

    case LSM6DS_FIFO_TAG_TIMESTAMP:
      // Timestamp word: 32-bit counter in first 4 bytes (little-endian)
      // x = bytes [0:1], y = bytes [2:3], z is unused
      current_hw_timestamp =
          ((uint32_t)(uint16_t)x) | (((uint32_t)(uint16_t)y) << 16);
      break;

    case LSM6DS_FIFO_TAG_ACCEL_NC: {
      // Convert 25us ticks to milliseconds
      float ts_ms = current_hw_timestamp * 0.025;
      Serial.print(ts_ms, 3);
      Serial.print(",accel,");
      Serial.print(x);
      Serial.print(",");
      Serial.print(y);
      Serial.print(",");
      Serial.println(z);
      break;
    }

    case LSM6DS_FIFO_TAG_GYRO_NC: {
      float ts_ms = current_hw_timestamp * 0.025;
      Serial.print(ts_ms, 3);
      Serial.print(",gyro,");
      Serial.print(x);
      Serial.print(",");
      Serial.print(y);
      Serial.print(",");
      Serial.println(z);
      break;
    }

    default:
      break;
    }
  }

  // Read every 500ms
  delay(500);
}
