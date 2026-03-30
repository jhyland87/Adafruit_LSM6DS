// FIFO linear acceleration demo for LSM6DSOX over SPI
//
// Subtracts gravity from accelerometer readings so you only see dynamic
// movement. On startup the sensor must be stationary for calibration —
// it averages several samples to learn the gravity vector, then subtracts
// it from every subsequent reading.
//
// Output is Arduino Serial Plotter friendly (label:value pairs).

#include <Adafruit_LSM6DSOX.h>

#define LSM_CS 10
// For software-SPI mode we also need SCK/MOSI/MISO pins
// #define LSM_SCK 13
// #define LSM_MISO 12
// #define LSM_MOSI 11

Adafruit_LSM6DSOX sox;

// Number of samples to average during gravity calibration
#define CAL_SAMPLES 200

// Gravity baseline (raw int16 values)
float gravity_x = 0, gravity_y = 0, gravity_z = 0;

// Latest values for plotter (all channels on every line)
int16_t lin_x = 0, lin_y = 0, lin_z = 0;
int16_t gyro_x = 0, gyro_y = 0, gyro_z = 0;

void calibrateGravity() {
  Serial.println("Calibrating — keep sensor still...");

  // Flush any stale FIFO data
  sox.resetFIFO();
  delay(50);

  long sum_x = 0, sum_y = 0, sum_z = 0;
  uint16_t collected = 0;

  while (collected < CAL_SAMPLES) {
    uint16_t count = sox.getFIFOCount();
    if (count == 0) {
      delay(5);
      continue;
    }

    lsm6ds_fifo_tag_t tag;
    int16_t x, y, z;

    while (sox.readFIFOWord(tag, x, y, z) && collected < CAL_SAMPLES) {
      if (tag == LSM6DS_FIFO_TAG_ACCEL_NC) {
        sum_x += x;
        sum_y += y;
        sum_z += z;
        collected++;
      }
    }
  }

  gravity_x = (float)sum_x / CAL_SAMPLES;
  gravity_y = (float)sum_y / CAL_SAMPLES;
  gravity_z = (float)sum_z / CAL_SAMPLES;

  Serial.print("Gravity baseline — X: ");
  Serial.print(gravity_x, 1);
  Serial.print("  Y: ");
  Serial.print(gravity_y, 1);
  Serial.print("  Z: ");
  Serial.println(gravity_z, 1);
  Serial.println("Calibration done.");

  // Flush again so the loop starts clean
  sox.resetFIFO();
}

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

  sox.setAccelDataRate(LSM6DS_RATE_104_HZ);
  sox.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  sox.setGyroDataRate(LSM6DS_RATE_104_HZ);
  sox.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);

  sox.setFIFOAccelBatchRate(LSM6DS_FIFO_RATE_104_HZ);
  sox.setFIFOGyroBatchRate(LSM6DS_FIFO_RATE_104_HZ);
  sox.setFIFOMode(LSM6DS_FIFO_CONTINUOUS);

  calibrateGravity();
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
      lin_x = (int16_t)(x - gravity_x);
      lin_y = (int16_t)(y - gravity_y);
      lin_z = (int16_t)(z - gravity_z);
      break;

    case LSM6DS_FIFO_TAG_GYRO_NC:
      gyro_x = x;
      gyro_y = y;
      gyro_z = z;
      break;

    default:
      continue;
    }

    Serial.printf("LinAccX:%-6i", lin_x);
    Serial.printf("LinAccY:%-6i", lin_y);
    Serial.printf("LinAccZ:%-6i", lin_z);
    Serial.printf("GyroX:%-6i", gyro_x);
    Serial.printf("GyroY:%-6i", gyro_y);
    Serial.printf("GyroZ:%-6i", gyro_z);
    Serial.println();
  }
}
