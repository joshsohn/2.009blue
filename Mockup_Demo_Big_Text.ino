#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_LPS35HW.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
#define LED_1_PIN 10
#define LED_2_PIN 11
#define LED_3_PIN 12
#define BUZZER_PIN 13
#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5

bool currentlyUnderwater = false;
bool currentlyDrowning = false;
bool currentlyAlarming = false;
float seaLevelPressurehPa = 1013.15;

Adafruit_BMP3XX bmp;
// Basic demo for readings from Adafruit BNO08x
#include <Adafruit_BNO08x.h>

// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9

// For SPI mode, we also need a RESET
//#define BNO08X_RESET 5
// but not for I2C or UART
#define BNO08X_RESET -1

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

void setup(void) {
  Serial.begin(115200);
//  while (!Serial)
//    delay(10); // will pause Zero, Leonardo, etc until serial console opens
   /// PRESSURE SENSOR
    if (!bmp.begin_I2C()) {   // hardware I2C mode, can pass in address & alt Wire
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
  
  ////PRESSURE SENSOR
  ////OLED
  Serial.println("128x64 OLED FeatherWing test");
  delay(250); // wait for the OLED to power up
  
  display.begin(0x3C, true); // Address 0x3C default
  
  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  Serial.println("Button test");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  //OLED End
  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C()) {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte
    // UART buffer! if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");

  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }

  setReports();

  Serial.println("Reading events");
  delay(100);
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION)) {
    Serial.println("Could not enable linear acceleration");
  }

}

float xAccel = 0;
float yAccel = 0;
float zAccel = 0;

float alpha = 0.01;
float filteredAccel = 5;
float accel = 0;

long lastOLEDmillis = 0;
long OLEDdelaymillis = 20;
float pressurehPa = 0;
float relativePressure = 0;
float depth = 0;
float depthFiltered = -0.5;
float depthAlpha = 0.01;
long lastMillisSwimming = 0;
bool currentlyAboveWater = true;
long lastMillisDrowning = 1000; //don't check drowning in the first 5 seconds

#define MIN_ALARM_TIME 5 //seconds
#define ABOVE_WATER_DELAY 5 //seconds
#define WATER_DENSITY 997.0474
#define GRAV_ACCEL 9.80665  
#define MIN_ACCELERATION 0.0 //m/s^2

#define DEEP_WATER_DEPTH 0.15
#define LED_BLINK_MILLIS 100 //Time between LED blinks in millis
long lastLEDBlink = 0;

void loop() {
  delay(10);

  if (bno08x.wasReset()) {

    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }
    switch (sensorValue.sensorId){
      case SH2_LINEAR_ACCELERATION:
        xAccel = sensorValue.un.linearAcceleration.x;
        yAccel = sensorValue.un.linearAcceleration.y;
        zAccel = sensorValue.un.linearAcceleration.z;
        break;
    }

    if (bmp.performReading()){
      pressurehPa = bmp.pressure / 100.0;
      relativePressure = (pressurehPa - seaLevelPressurehPa)*100;
      depth = relativePressure/(WATER_DENSITY*GRAV_ACCEL);
    }    
    if(!digitalRead(BUTTON_A)) {
      //Zero the pressure
      seaLevelPressurehPa = pressurehPa;
      depthFiltered = 0;
    }   
      depthFiltered = depthAlpha*depth + (1-depthAlpha)*depthFiltered;
      accel = sqrt(sq(xAccel) + sq(yAccel) + sq(zAccel));
      filteredAccel = alpha*accel + (1-alpha)*filteredAccel;
      Serial.println(filteredAccel);

    currentlyDrowning = (depthFiltered > DEEP_WATER_DEPTH) || (filteredAccel < MIN_ACCELERATION && currentlyUnderwater);
    
    if (currentlyDrowning) {
      lastMillisDrowning = millis();
    } 
    //don't alarm for the first 10 seconds
    currentlyAlarming = ((millis() - lastMillisDrowning) < MIN_ALARM_TIME*1000) && millis() > 10000;
    
    #define MIN_UNDERWATER_DEPTH 0.04 //m
    currentlyUnderwater = depthFiltered > MIN_UNDERWATER_DEPTH; 
    
    if (currentlyUnderwater) {
      lastMillisSwimming = millis();
    } 
   
    currentlyAboveWater = (millis() - lastMillisSwimming) > ABOVE_WATER_DELAY*1000;
    
    if (currentlyAlarming){
      if (millis() - lastLEDBlink > LED_BLINK_MILLIS){
        digitalWrite(LED_1_PIN, HIGH);
        digitalWrite(LED_2_PIN, HIGH);
        digitalWrite(LED_3_PIN, HIGH);
        tone(BUZZER_PIN, 2500);
        lastLEDBlink = millis();
      } else {
        digitalWrite(LED_1_PIN, LOW);
        digitalWrite(LED_2_PIN, LOW);
        digitalWrite(LED_3_PIN, LOW);
        noTone(8);
      }   
    } else { //turn it off when not alarming
         digitalWrite(LED_1_PIN, LOW);
        digitalWrite(LED_2_PIN, LOW);
        digitalWrite(LED_3_PIN, LOW);
        noTone(8);
    }
    
    if (millis() - lastOLEDmillis > OLEDdelaymillis){
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SH110X_WHITE);
      
      display.setCursor(0,0);
      display.print("Accel: ");
      display.print(filteredAccel);
      display.println(" m/s^2");
      display.print("Pressure: ");
      display.print(pressurehPa);
      display.println(" hPa");
      display.print("Depth: ");
      display.print(depth);
      display.println(" m");
      display.print("Depth filt: ");
      display.print(depthFiltered);
      display.println(" m");
      display.print("Rel Pres: ");
      display.print(relativePressure);
      display.println("Pa");
      
      display.setTextSize(2);
        if (currentlyDrowning || currentlyAlarming){
            display.print("DROWNING!");
        } else if (currentlyAboveWater){
          display.print("Landlubber");
        } else {
          display.print("Swimming");
        }
      
      display.display(); // actually display all of the above
      lastOLEDmillis = millis();
    }
}
