#include <Wire.h>

const int imuAddr = 0x68;
const int internalRegister = 0x43;
float x, y, z;
float anglePitch = 0, angleRoll = 0;
float offsetGyroX = 0, offsetGyroY = 0, offsetGyroZ = 0;
float offsetAccX = 0, offsetAccY = 0, offsetAccZ = 0;
bool isGyroCalibrated = false;
bool isAccCalibrated = false;
const float pi = 3.14159265358979323;
long loopTimer;
const int accRegister = 0x3B;
float accX, accY, accZ;
float accVector, accPitch, accRoll;
float pitch, roll;

void readGyro(){
  Wire.beginTransmission(imuAddr);
  Wire.write(internalRegister);
  Wire.endTransmission(false);
  Wire.requestFrom(imuAddr,6,true);
  if (!isGyroCalibrated){
    x = (Wire.read() << 8| Wire.read());
    y = (Wire.read() << 8| Wire.read());
    z = (Wire.read() << 8| Wire.read());
  }
  else{
    x = ((Wire.read() << 8| Wire.read()) - offsetGyroX);
    y = ((Wire.read() << 8| Wire.read()) - offsetGyroY);
    z = ((Wire.read() << 8| Wire.read()) - offsetGyroZ);
  }
}


void calibrateGyro(){
  unsigned long int start = millis();
  int count = 0;
  while (millis() < start+5000){
    readGyro();
    offsetGyroX += x;
    offsetGyroY += y;
    offsetGyroZ += z;
    count++;
  }
  offsetGyroX /= count;
  offsetGyroY /= count;
  offsetGyroZ /= count;
  isGyroCalibrated = true;
}

void calibrateAcc(){
  unsigned long int start = millis();
  int count = 0;
  while (millis() < start+5000){
    readGyro();
    offsetAccX += x;
    offsetAccY += y;
    offsetAccZ += z;
    count++;
  }
  offsetAccX /= count;
  offsetAccY /= count;
  offsetAccZ /= count;
  isAccCalibrated = true;
}

void readAcc(){
  Wire.beginTransmission(imuAddr);
  Wire.write(accRegister);
  Wire.endTransmission(false);
  Wire.requestFrom(imuAddr, 6);
  if (!isGyroCalibrated){
    accX = (Wire.read()<<8|Wire.read());
    accY = (Wire.read()<<8|Wire.read());
    accZ = (Wire.read()<<8|Wire.read());
  }
  else{
    accX = (Wire.read()<<8|Wire.read())-offsetAccX;
    accY = (Wire.read()<<8|Wire.read())-offsetAccY;
    accZ = (Wire.read()<<8|Wire.read())-offsetAccZ;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(imuAddr);
  Wire.beginTransmission(imuAddr);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true); // frees slave from communication so other devices can access (default is true);
  Wire.beginTransmission(imuAddr);
  Wire.write(0x1B);
  Wire.write(0x08);
  Wire.endTransmission(true);
  calibrateGyro();
  calibrateAcc();
}

void loop() {
  readGyro();  
  anglePitch += x*0.0000611;
  angleRoll += y*0.0000611;
  anglePitch += angleRoll * sin(z * 0.000001066);
  angleRoll -= anglePitch * sin(z * 0.000001066);
  readAcc();
  accVector = sqrt((accX*accX)+(accY*accY)+(accZ*accZ));
  accPitch = asin(accY/accVector)*57.296;
  accRoll = asin(accX/accVector)*57.296;
  pitch = anglePitch*0.9996 + accPitch*0.0004;
  roll = angleRoll*0.9996 + accRoll*0.0004;
  Serial.print(pitch); Serial.print(", "); Serial.println(roll);
  while(micros() - loopTimer < 3999);
  loopTimer = micros();
}