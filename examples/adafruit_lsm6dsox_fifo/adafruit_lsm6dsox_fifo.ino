// FIFO demo for accelerometer & gyro readings from Adafruit LSM6DSOX sensor
//
// The FIFO buffers sensor data so you can read in batches instead of polling.
// This is useful for reducing I2C traffic and ensuring no samples are missed.

#include <Adafruit_LSM6DSOX.h>

Adafruit_LSM6DSOX sox;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Adafruit LSM6DSOX FIFO test!");

  if (!sox.begin_I2C()) {
    Serial.println("Failed to find LSM6DSOX chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("LSM6DSOX Found!");

  // Configure sensor data rates
  sox.setAccelDataRate(LSM6DS_RATE_104_HZ);
  sox.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  sox.setGyroDataRate(LSM6DS_RATE_104_HZ);
  sox.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);

  // Configure FIFO: batch both accel and gyro at 104 Hz
  sox.setFIFOAccelBatchRate(LSM6DS_FIFO_RATE_104_HZ);
  sox.setFIFOGyroBatchRate(LSM6DS_FIFO_RATE_104_HZ);

  // Set watermark to 10 samples (optional, for interrupt-driven reads)
  sox.setFIFOWatermark(10);

  // Enable continuous mode - old data is overwritten when FIFO is full
  sox.setFIFOMode(LSM6DS_FIFO_CONTINUOUS);

  Serial.println("FIFO configured in continuous mode");
  Serial.println();
}

void loop() {
  // Wait until we have some data in the FIFO
  uint16_t count = sox.getFIFOCount();
  if (count == 0) {
    delay(10);
    return;
  }

  Serial.print("FIFO samples available: ");
  Serial.println(count);

  if (sox.getFIFOOverrun()) {
    Serial.println("WARNING: FIFO overrun detected!");
  }

  // Read all available FIFO words
  lsm6ds_fifo_tag_t tag;
  int16_t x, y, z;

  while (sox.readFIFOWord(tag, x, y, z)) {
    switch (tag) {
    case LSM6DS_FIFO_TAG_ACCEL_NC:
      Serial.print("  Accel raw X: ");
      Serial.print(x);
      Serial.print(" Y: ");
      Serial.print(y);
      Serial.print(" Z: ");
      Serial.println(z);
      break;

    case LSM6DS_FIFO_TAG_GYRO_NC:
      Serial.print("  Gyro  raw X: ");
      Serial.print(x);
      Serial.print(" Y: ");
      Serial.print(y);
      Serial.print(" Z: ");
      Serial.println(z);
      break;

    case LSM6DS_FIFO_TAG_TEMP:
      Serial.print("  Temp raw: ");
      Serial.println(x);
      break;

    default:
      Serial.print("  Unknown tag: 0x");
      Serial.println(tag, HEX);
      break;
    }
  }

  Serial.println();
  delay(500); // Read FIFO every 500ms
}
