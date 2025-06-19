#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // Initialize I2C with pins 21 (SDA) and 22 (SCL)

  Serial.println("Initializing MPU6050...");
  
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful!");
  } else {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    while (1);
  }
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Convert raw values to g's and degrees/s
  float accel_x = ax / 16384.0;
  float accel_y = ay / 16384.0;
  float accel_z = az / 16384.0;
  
  float gyro_x = gx / 131.0;
  float gyro_y = gy / 131.0;
  float gyro_z = gz / 131.0;
  
  // Calculate angles from accelerometer data
  float angle_x = atan(accel_y / sqrt(pow(accel_x, 2) + pow(accel_z, 2))) * 180 / PI;
  float angle_y = atan(-accel_x / sqrt(pow(accel_y, 2) + pow(accel_z, 2))) * 180 / PI;

  Serial.print("Accel X: "); Serial.print(accel_x); Serial.print(" g ");
  Serial.print(" Y: "); Serial.print(accel_y); Serial.print(" g ");
  Serial.print(" Z: "); Serial.print(accel_z); Serial.println(" g");
  
  Serial.print("Gyro X: "); Serial.print(gyro_x); Serial.print(" °/s ");
  Serial.print(" Y: "); Serial.print(gyro_y); Serial.print(" °/s ");
  Serial.print(" Z: "); Serial.print(gyro_z); Serial.println(" °/s");
  
  Serial.print("Angle X: "); Serial.print(angle_x); Serial.print(" ° ");
  Serial.print(" Y: "); Serial.print(angle_y); Serial.println(" °");

  delay(500); // Wait for 500 milliseconds before reading again
}
