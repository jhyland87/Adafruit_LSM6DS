// FIFO with calculated timestamps - LSM6DSOX
//
// Since the FIFO batch data rate is fixed and crystal-driven, we can
// reconstruct accurate timestamps for each sample by knowing when we
// read the FIFO and how many samples were buffered.
//
// Output is CSV-formatted for easy graphing in the Arduino Serial Plotter,
// Excel, or any plotting tool.

#include <Adafruit_LSM6DSOX.h>

Adafruit_LSM6DSOX sox;

// Must match the FIFO batch data rate set in setup()
const float SAMPLE_RATE_HZ = 104.0;
const float SAMPLE_INTERVAL_MS = 1000.0 / SAMPLE_RATE_HZ;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("LSM6DSOX FIFO - Calculated Timestamps");

  if (!sox.begin_I2C()) {
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

  // Timestamp of this read
  unsigned long now = millis();

  // Count accel and gyro samples separately since they interleave
  // First pass: figure out how many of each type are in the FIFO
  // We can estimate: with both at 104 Hz, roughly half are accel, half gyro
  // For accurate indexing, we track per-type counters as we read

  // We'll assign timestamps based on position within the batch.
  // The newest sample was taken ~now, the oldest was taken
  // (count * SAMPLE_INTERVAL_MS / 2) ms ago (divided by 2 because
  // accel and gyro alternate, so each type has count/2 samples).
  uint16_t accel_index = 0;
  uint16_t gyro_index = 0;

  // Count how many accel vs gyro words to expect
  uint16_t accel_total = count / 2;
  uint16_t gyro_total = count / 2;

  lsm6ds_fifo_tag_t tag;
  int16_t x, y, z;

  while (sox.readFIFOWord(tag, x, y, z)) {
    float timestamp_ms;

    switch (tag) {
    case LSM6DS_FIFO_TAG_ACCEL_NC:
      // Oldest accel sample is (accel_total - 1) intervals before now
      timestamp_ms = now - (accel_total - 1 - accel_index) * SAMPLE_INTERVAL_MS;
      Serial.print(timestamp_ms, 2);
      Serial.print(",accel,");
      Serial.print(x);
      Serial.print(",");
      Serial.print(y);
      Serial.print(",");
      Serial.println(z);
      accel_index++;
      break;

    case LSM6DS_FIFO_TAG_GYRO_NC:
      timestamp_ms = now - (gyro_total - 1 - gyro_index) * SAMPLE_INTERVAL_MS;
      Serial.print(timestamp_ms, 2);
      Serial.print(",gyro,");
      Serial.print(x);
      Serial.print(",");
      Serial.print(y);
      Serial.print(",");
      Serial.println(z);
      gyro_index++;
      break;

    default:
      break;
    }
  }

  // Read every 500ms - accumulates ~52 accel + 52 gyro samples
  delay(500);
}
