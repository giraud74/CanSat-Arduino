#include "Wire.h"

#include <SD.h>

#include "I2Cdev.h"
#include "MPU6050.h"
#include "BMP085.h"
#include "HMC5883L.h"

#define LED_PIN 13

MPU6050  accelgyro;
BMP085   barometer;
HMC5883L mag;

const int chipSelect = 10;

bool    start = false;
bool    blinkState = false;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;
int32_t lastMicros;
float   temperature;
float   pressure;
float   altitude;
float   heading;
String  dataString = "";

void setup() {

    Wire.begin();

    Serial.begin(9600);
    
    barometer.initialize();
    accelgyro.initialize();
    mag.initialize();

    barometer.testConnection();
    accelgyro.testConnection();
    mag.testConnection();

    pinMode(10, OUTPUT);

    SD.begin(chipSelect);

    pinMode(LED_PIN, OUTPUT);

}

void loop() {

    if (!start)
    {
        File dataFile = SD.open("datalog.txt", FILE_WRITE);

        dataString = String("millis,ax,ay,az,gx,gy,gz,temp,press,alt,mx,my,mz,heading");

        dataFile.println(dataString);
        dataFile.close();  
    
        Serial.println(dataString);

        start = true;
    }   

    mag.getHeading(&mx, &my, &mz);
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    barometer.setControl(BMP085_MODE_TEMPERATURE);
    
    lastMicros = micros();                         // wait appropriate time for conversion (4.5ms delay)
    while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());
    temperature = barometer.getTemperatureC();     // read calibrated temperature value in degrees Celsius

    barometer.setControl(BMP085_MODE_PRESSURE_3);  // request pressure (3x oversampling mode, high detail, 23.5ms delay)
    while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());

    pressure = barometer.getPressure();            // read calibrated pressure value in Pascals (Pa)
    altitude = barometer.getAltitude(pressure);    // calculate absolute altitude in meters uses the standard value of 101325 Pa)

    heading = atan2(my, mx);    // To calculate heading in degrees. 0 degree indicates North
    if(heading < 0)
      heading += 2 * M_PI;

    dataString = ""; 
    dataString += millis();             dataString += ","; 
    dataString += ax;                   dataString += ","; 
    dataString += ay;                   dataString += ","; 
    dataString += az;                   dataString += ","; 
    dataString += gx;                   dataString += ","; 
    dataString += gy;                   dataString += ","; 
    dataString += gz;                   dataString += ","; 
    dataString += int(temperature*100); dataString += ","; 
    dataString += int(pressure*100);    dataString += ","; 
    dataString += int(altitude*100);    dataString += ","; 
    dataString += mx;                   dataString += ","; 
    dataString += my;                   dataString += ","; 
    dataString += mz;                   dataString += ","; 
    dataString += int((heading * 180/M_PI)*100); 

    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    Serial.println(dataString);
  
    dataFile.println(dataString);
    dataFile.close();  

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
        
    delay(100);    // delay 100 msec to allow visually parsing blink and any serial output
}
