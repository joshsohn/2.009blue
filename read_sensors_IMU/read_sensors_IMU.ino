#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <Adafruit_BNO08x.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BMP_SCK 13

#define SEALEVELPRESSURE_HPA (1013.25)
#define BNO08X_RESET -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_BMP3XX bmp;
Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION)) {
    Serial.println("Could not enable linear acceleration");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Sensors test");

  if (!bmp.begin_I2C()) {
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }
  Serial.println("BMP3 Found!");

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

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  setReports();
  
  Serial.println("");
}

float xAccel = 0;
float yAccel = 0;
float zAccel = 0;

void loop() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  if (! bmp.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  
  Serial.print("Pressure = ");
  Serial.print(bmp.pressure / 100.0);
  Serial.println(" hPa");
  
  display.print("Pressure = ");
  display.print(bmp.pressure / 100.0);
  display.println("   hPa");

  if (bno08x.wasReset()) {

    Serial.print("sensor was reset ");
    setReports();
  }
  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }
  switch (sensorValue.sensorId) {
    case SH2_LINEAR_ACCELERATION:
      xAccel = sensorValue.un.linearAcceleration.x;
      yAccel = sensorValue.un.linearAcceleration.y;
      zAccel = sensorValue.un.linearAcceleration.z;
      break;
  }

  Serial.print("Linear Acceration - x: ");
  Serial.print(xAccel);
  Serial.print(" y: ");
  Serial.print(yAccel);
  Serial.print(" z: ");
  Serial.println(zAccel);
  
  display.println("Linear Acceration = ");
  display.print("x: ");
  display.print(xAccel);
  display.println("  m/s^2");
  display.print("y: ");
  display.print(yAccel);
  display.println("  m/s^2 ");
  display.print("z: ");
  display.print(zAccel);
  display.println("  m/s^2 ");
  display.println();
  display.display();      // Show initial text
  
  delay(500);
}
