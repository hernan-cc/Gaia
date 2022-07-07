#define BLYNK_TEMPLATE_ID "TMPL7qODvrzN"
#define BLYNK_DEVICE_NAME "Gaia v001"
#define BLYNK_AUTH_TOKEN "aP4dI-7Q1Ba4QRSE4JJl5p8wICvBbGnM"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "HCC 2.4";
char pass[] = "caravana";

BlynkTimer timer;

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include "DHTesp.h"
DHTesp dht;

float vpd;
float temp;
float hum;
int light;
float vpdMax;
float vpdMin;
float tempMax;
float tempMin;
int heatOn = 1;
const uint8_t FAN = D4;
const uint8_t LED = D6;
const uint8_t Sensor_LED = D8;
const uint8_t HEAT = D7;


float measureHum() {
  float humidity = dht.getHumidity();
  return humidity;
}

float measureTemp() {
  float temp = dht.getTemperature();
  return temp;
}

float getVPD(float hum, float temp) { // calcula vapor preasure deficit en kPa
 float svp = (610.78 * exp(temp/(temp + 238.3) * 17.269)/1000);
 float vpd = svp * ( 1 - hum/100);
 return vpd;
}

// int measureLight() {
//   int light = digitalRead(Sensor_LED);
//   if (light == HIGH){
//     Blynk.virtualWrite(V6, 0);
//     return 0;
//   }else{
//     Blynk.virtualWrite(V6, 1);
//     return 1;
//   }
// }

// void printCenterText(const String &str, int x, int y){ // toma una string y unas coordenadas iniciales e imprime el texto en el centro de la pantalla
//   int16_t x1, y1;
//   uint16_t w, h;
//   display.getTextBounds(str, x, y, &x1, &y1, &w, &h); //devuelve las medidas del cuadro de texto
//   display.setCursor(x + 64 - w / 2, y);
//   display.println(str);
// }

void updateDisplay(float hum, float temp, float vpd, int light){ 
  display.setRotation(2);
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.print("Temp:");
  display.print(temp);
  display.setTextSize(1);display.println("C");display.println(' ');

  display.setTextSize(2);
  display.print("RH:");
  display.print(hum);
  display.setTextSize(1);display.println("%");display.println(' ');

  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.print("VPD:");
  display.print(vpd);
  display.setTextSize(1);display.println("kPa");display.println(' ');

  // display.setTextSize(2);
  // display.setTextColor(SH110X_WHITE);
  // display.print("Luz:");
  // if (light==1){
  //   display.print("ON");
  // }else{
  //   display.print("OFF");
  // }
  display.display();

}

void getData() {
  temp = measureTemp(); 
  hum = measureHum();  
  vpd = getVPD(hum, temp);
  // light = measureLight();
}

void sendData()
{
  Blynk.virtualWrite(V4, vpd);  
  Blynk.virtualWrite(V3, temp);
  Blynk.virtualWrite(V2, hum);
  updateDisplay(hum, temp, vpd, light);
}


void handleExhaust(float vpd, float vpdMax, float vpdMin){
  float min = vpdMin; 
  float max = vpdMax; 
  if (vpd <= min){
    digitalWrite(FAN, HIGH);
    Blynk.virtualWrite(V1, 1);
  }else if (vpd >= max)
  {
    digitalWrite(FAN, LOW);
    Blynk.virtualWrite(V1, 0);
  }
}

void handleHeat(float temp, float tempMin, float tempMax, float vpdMax, float vpd){
  if (temp <= tempMin && vpd < (vpdMax - 0.15)){
    digitalWrite(HEAT, HIGH);
    Blynk.virtualWrite(V9, 0);
  }else if (temp >= tempMax || vpd >= vpdMax){
    digitalWrite(HEAT, LOW);
    Blynk.virtualWrite(V9, 1);
  }
}


BLYNK_WRITE(V0)
{
  if (param.asInt()==1){
    digitalWrite(LED, HIGH);
  }else{
    digitalWrite(LED, LOW);
  }
}
BLYNK_WRITE(V7)
{
  vpdMin = param.asFloat();
}
BLYNK_WRITE(V8)
{
  vpdMax = param.asFloat();
}
BLYNK_WRITE(V10)
{
  tempMin = param.asFloat();
}
BLYNK_WRITE(V11)
{
  tempMax = param.asFloat();
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V7);
  Blynk.syncVirtual(V8);  // will cause BLYNK_WRITE(V0) to be executed
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
  Blynk.syncVirtual(V12);
}

void setup()   {
  
  Serial.begin(9600);

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  display.setContrast (-1); // dim display
  display.display();
  dht.setup(D0, DHTesp::DHT22);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval((dht.getMinimumSamplingPeriod()), getData);
  timer.setInterval(10000, sendData);
  getData();
  sendData();
  pinMode (FAN, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(Sensor_LED, INPUT);
  pinMode(HEAT, OUTPUT);


}

void loop()
{
  Blynk.run();
  timer.run();
  handleExhaust(vpd, vpdMax, vpdMin);
  handleHeat(temp, tempMin, tempMax, vpdMax, vpd);
}

