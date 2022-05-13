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


#include <iostream>
#include <string>

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

DHTesp dht;

float vpd;
float temp;
float hum;


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

// void sendData(){                 //se ejecuta en cada tick del timer
//   hum = measureHum(0);           // sin delay porque usamos timer de 2 seg que es el minimo del sensor
//   temp = measureTemp(0);         // estoy tomando la medida dos veces??
//   vpd = getVPD(hum,temp);        // si no declaro la variable aca no me la toma de la global
//   Blynk.virtualWrite(V4, vpd);   // tendria que pasarlas como parametros de sendData()?
//   Blynk.virtualWrite(V3, temp);
//   Blynk.virtualWrite(V2, hum);
// }

void printCenterText(const String &str, int x, int y){
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(str, x, y, &x1, &y1, &w, &h); //devuelve las medidas del cuadro de texto
  display.setCursor(x + 64 - w / 2, y);
  display.println(str);

 // toma una string y unas coordenadas iniciales e imprime el texto en el centro de la pantalla
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
  delay(5000);

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
  delay(5000);
   
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
  delay(5000);

}

void sendData()
{
  temp = measureTemp(); 
  hum = measureHum();  
  vpd = getVPD(hum, temp);
  Blynk.virtualWrite(V4, vpd);  
  Blynk.virtualWrite(V3, temp);
  Blynk.virtualWrite(V2, hum);
  updateDisplay(hum, temp, vpd);
}

void setup()   {
  
  Serial.begin(9600);

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  display.setContrast (1); // dim display
  display.display();
  dht.setup(D0, DHTesp::DHT22);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval((dht.getMinimumSamplingPeriod()), sendData);

}

void loop()
{
  Blynk.run();
  timer.run();
}

