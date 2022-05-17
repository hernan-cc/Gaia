#define BLYNK_TEMPLATE_ID "TMPL7qODvrzN"
#define BLYNK_DEVICE_NAME "Gaia v001"
#define BLYNK_AUTH_TOKEN "aP4dI-7Q1Ba4QRSE4JJl5p8wICvBbGnM"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "HCC";
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

// mantener VPD entre 0.5 y 0.7 con extractor


// prender extractor siempre en rh > 70% durante 10 min


float vpd;
float temp;
float hum;
const uint8_t FAN = D4;
const uint8_t LED = D6;


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

void printCenterText(const String &str, int x, int y){ // toma una string y unas coordenadas iniciales e imprime el texto en el centro de la pantalla
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(str, x, y, &x1, &y1, &w, &h); //devuelve las medidas del cuadro de texto
  display.setCursor(x + 64 - w / 2, y);
  display.println(str);
}

void updateDisplay(float hum, float temp, float vpd){ //tiene que haber alguna forma de optimizar esto
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_BLACK,SH110X_WHITE);
  printCenterText(" Temp ",0,0);
  display.setTextColor(SH110X_WHITE);
  display.println(" ");
  display.setTextSize(3);
//  std::string str1 = std::to_string(temp); tratar de pasar float temp a string para aplicar printCenterText()
//  printCenterText(str1,0,0);
  display.print(temp);
  display.setTextSize(2);display.println(" C");
  display.display();
  delay(2500);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_BLACK,SH110X_WHITE);
  printCenterText(" Humedad ",0,0);
  display.setTextColor(SH110X_WHITE);
  display.println(" ");
  display.setTextSize(3);
  display.print(hum);
  display.setTextSize(2);display.print(" %");
  display.display();
  delay(2500);
   
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_BLACK,SH110X_WHITE);
  printCenterText(" VPD ",0,0);
  display.setTextColor(SH110X_WHITE);
  display.println(" ");
  display.setTextSize(3);
  display.print(vpd);
  display.setTextSize(2);display.println(" kPa");
  display.display();
  delay(2500);

}

void getData() {
  temp = measureTemp(); 
  hum = measureHum();  
  vpd = getVPD(hum, temp);
}

void sendData()
{
  Blynk.virtualWrite(V4, vpd);  
  Blynk.virtualWrite(V3, temp);
  Blynk.virtualWrite(V2, hum);
}


void handleExhaust(float vpd){
  float min = 0.5;
  float max = 0.7 ; 
  if (vpd <= min){
    digitalWrite(FAN, HIGH);
    Blynk.virtualWrite(V1, 1);
  }else if (vpd >= max)
  {
    digitalWrite(FAN, LOW);
    Blynk.virtualWrite(V1, 0);
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

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);  // will cause BLYNK_WRITE(V0) to be executed
}

void setup()   {
  
  Serial.begin(9600);

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  display.setRotation(2);
  display.setContrast (1); // dim display
  display.display();
  dht.setup(D0, DHTesp::DHT22);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval((dht.getMinimumSamplingPeriod()), getData);
  timer.setInterval(10000, sendData);
  getData();
  sendData();
  pinMode (FAN, OUTPUT);
  pinMode(LED, OUTPUT);



}

void loop()
{
  Blynk.run();
  timer.run();
  updateDisplay(hum, temp, vpd);
  handleExhaust(vpd);
}

