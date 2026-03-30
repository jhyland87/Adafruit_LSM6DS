// FIFO plotter demo for LSM6DSOX over SPI
//
// Reads accel and gyro data from the FIFO and outputs in Arduino Serial
// Plotter format: label:value pairs separated by tabs, one set per line.
// Open Tools > Serial Plotter in the Arduino IDE to see live graphs.
//
// SPI is faster than I2C, making it ideal for high-throughput FIFO reads.

#include <Adafruit_LSM6DSOX.h>

// For hardware SPI mode, we need a CS pin
#define LSM_CS 10
// For software-SPI mode we also need SCK/MOSI/MISO pins
#define LSM_SCK 13
#define LSM_MISO 12
#define LSM_MOSI 11

Adafruit_LSM6DSOX sox;

// Latest values for each axis - the plotter needs all values on every line
int16_t accel_x = 0, accel_y = 0, accel_z = 0;
int16_t gyro_x = 0, gyro_y = 0, gyro_z = 0;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  if (!sox.begin_SPI(LSM_CS)) {
    // if (!sox.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
    Serial.println("Failed to find LSM6DSOX chip");
    while (1) {
      delay(10);
    }
  }

  // Configure sensor data rates
  sox.setAccelDataRate(LSM6DS_RATE_104_HZ);
  sox.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  sox.setGyroDataRate(LSM6DS_RATE_104_HZ);
  sox.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);

  // Batch both sensors into FIFO at 104 Hz
  sox.setFIFOAccelBatchRate(LSM6DS_FIFO_RATE_104_HZ);
  sox.setFIFOGyroBatchRate(LSM6DS_FIFO_RATE_104_HZ);

  // Continuous mode - oldest data is overwritten when FIFO is full
  sox.setFIFOMode(LSM6DS_FIFO_CONTINUOUS);
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
    case LSM6DS_FIFO_TAG_ACCEL_NC:
      accel_x = x;
      accel_y = y;
      accel_z = z;
      break;

    case LSM6DS_FIFO_TAG_GYRO_NC:
      gyro_x = x;
      gyro_y = y;
      gyro_z = z;
      break;

    default:
      continue; // skip non-sensor tags without printing
    }

    // Print all 6 channels on every line so the plotter stays in sync
    Serial.print("AccelX:");
    Serial.print(accel_x);
    Serial.print("\tAccelY:");
    Serial.print(accel_y);
    Serial.print("\tAccelZ:");
    Serial.print(accel_z);
    Serial.print("\tGyroX:");
    Serial.print(gyro_x);
    Serial.print("\tGyroY:");
    Serial.print(gyro_y);
    Serial.print("\tGyroZ:");
    Serial.println(gyro_z);
  }
}
