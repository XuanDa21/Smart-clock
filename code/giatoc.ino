#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
//blynk
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <BlynkSimpleEsp32_SSL.h>
char auth[] = "f0fY6mdBCc8L3mnf_P385iytkHs9_Q2G";
char ssid[] = "DESKTOP-ARFTRKG 9463";
char pass[] = "datrinh318";
// gia toc
WidgetLCD lcd(V1);
// GPS
WidgetMap myMap(V2);
TinyGPS gps;
#define RXPin    15
#define TXPin    14
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
float a;
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
// nhip tim
#include <MAX30105.h>
#include "heartRate.h"
MAX30105 particleSensor;
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
float sensorData;
unsigned int move_index = 1;
BlynkTimer timer;
unsigned long time_now = 0;
#define debug Serial
// oled
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
void setup(void)
{

 Wire.begin(5, 4);
  // giu nguyen vi tri
  Blynk.begin(auth, ssid, pass);
  Serial.begin(9600);
  ss.begin(9600);

  timer.setInterval(5000L, checkGPS);
  if (!accel.begin(0x53))
  {
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);
  Serial.println("");
  if (particleSensor.begin(wire, I2C_SPEED_FAST) == false)
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);
  // oled
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.println("Hello, world!");
  display.display();
}

void loop(void)
{
  Blynk.run();
  timer.run();

  for (int i = 0; i < 50; i++)
  {

    if (i <= 10)
    {
      nhiptim();
    }
    else {
      giatoc();
      while (ss.available() > 0)
      {
        if (gps.encode(ss.read()))
        {
          vitri();
        }
      }
      float temperature = particleSensor.readTemperature();
      Blynk.virtualWrite(V4, temperature);
      int i = 0;
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);
      // Display static text
      display.print("Nhiệt độ: ");
      display.print(temperature);
      display.print(" C\t");
      display.display();

      // oled

      if (i == 49)
      {
        i = 0;
      }
    }
  }
}

void checkGPS() {
  if (gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    Blynk.virtualWrite(V5, "GPS ERROR"); // Value Display widget on V4 if GPS not detected
  }
}
void giatoc()
{
  sensors_event_t event;
  accel.getEvent(&event);
  a = sqrt(event.acceleration.x * event.acceleration.x * 0.0078 + event.acceleration.y * event.acceleration.y * 0.0078 + event.acceleration.z * event.acceleration.z * 0.0078);
  if (a > 1.2)
  {
    while (1)
    {
      lcd.clear(); //Use it to clear the LCD Widget
      lcd.print(4, 0, "Nguy Hiểm!!! "); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
      lcd.print(4, 1, " Bị Ngã ");
      Blynk.virtualWrite(V3, beatAvg); //sending to Blynk
      float temperature = particleSensor.readTemperature();
      Blynk.virtualWrite(V4, temperature); //sending to Blynk
    }
  }
  else if (a < 1.2)
  {
    lcd.clear(); //Use it to clear the LCD Widget
    lcd.print(3, 0, "Bình Thường");
  }


}
void nhiptim()
{
  Blynk.run();
  Blynk.virtualWrite(V3, particleSensor.getGreen());
}
void vitri()
{
  if (gps.location.isValid() )
  {
    float latitude = (gps.location.lat());     //Storing the Lat. and Lon.
    float longitude = (gps.location.lng());
    myMap.location(move_index, latitude, longitude, "value");
    Blynk.virtualWrite(V5, gps.speed.kmph());
  }
}
