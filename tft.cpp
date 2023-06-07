/**************************************************************************
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Wire.h>
#include "WiFi.h"
#include "time.h"
//#include <AHT10.h> // there is no <AHT10.h> in official Library, replaced with <Adafruit_AHT10.h>
#include <Adafruit_AHT10.h>

#define screenWidth 160
#define screenHeight 80 
#define HUD_height 20

#define HUD_color ST77XX_RED
//SPI declarations
  #define TFT_CS         17
  #define TFT_RST        4 
  #define TFT_DC         21
  #define TFT_MOSI       23  // Data out
  #define TFT_SCLK       18  // Clock out

  #define LED            2 // on-board LED
  //  reference for ESP32 SatC_EDU REV-3 board's physical pins connections to VSPI:
  // SDA  GPIO23 aka VSPI MOSI
  // SCLK GPIO18 aka SCK aka VSPI SCK
  // D/C  GPIO15 aka A0 (also I2C SDA)
  // RST  GPIO4 aka RESET (also I2C SCL) 
  // CS   GPIO5  aka chip select
  // LED  3.3V
  // VCC  5V
  // GND - GND


  
  //Temp & Humidity Sensor
  //#define DHTPIN 22     // Digital pin connected to the DHT sensor
 // #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

  
// For ST7735-based display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);



 //DHT dht(DHTPIN, DHTTYPE);
uint8_t readStatus = 0;
//AHT10 myAHT10(AHT10_ADDRESS_0X38);
Adafruit_AHT10 myAHT10;
sensors_event_t humidity, temp;
float p = 3.1415926;

/*
 * //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
 */


const char* ssid = "SSID";
const char* password =  "password";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -5 *3600;//for timezone
const int   daylightOffset_sec = 3600;

void setup(void) {
  
  int i = 0;
  Wire.begin(27,33);
  Serial.begin(115200);
  Serial.print(F("Hello! ST77xx TFT Test"));
 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  delay(100);
  tft.initR(INITR_MINI160x80);  // Init ST7735S mini display, using a 0.96" 160x80 TFT:

  delay(100);
  tft.setRotation(3);
  //dht.begin();
  uint8_t sda = 27;
  uint8_t  scl = 33;
    tft.fillScreen(ST77XX_BLACK);
    tft.invertDisplay(true);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(20,25);
    size_t size = tft.print("connecting to sensor...");
    Serial.println(size);
    //while (myAHT10.begin(sda,scl) != true)
    while (myAHT10.begin() != true)
  {
   tft.print("Sensor not found");
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
    delay(5000);
  }
  Serial.println(F("AHT10 OK"));
  Serial.println(F("Initialized"));
  
  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);

  time = millis() - time;
  
  logo(ST77XX_GREEN);
  delay(1500);
  
 // welcome();  // display welcome message
  
  for(; i < 4; i++){
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  }
  // large block of text
 
  //WELCOME MESSAGE HERE!!!!

    tft.fillScreen(ST77XX_BLACK);
    welcome();
    delay(1000);
    tft.fillScreen(ST77XX_BLACK);
    HUD();
    
   
    delay(500);
    tft.setCursor(screenWidth-3*screenWidth/4+20, HUD_height-8);
    tft.setTextColor(ST77XX_YELLOW);
    tft.print("Connecting");
    for(i=0; i < 4;i++){
      tft.drawChar(screenWidth-3*screenWidth/4+80+i*2, HUD_height-10,'.',ST77XX_YELLOW,1,1);
      //void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
      delay(250);
    }
    WiFi.begin(ssid, password);
    delay(500);
    tft.fillRect(screenWidth/2-20, HUD_height - 10, 70,10, ST77XX_BLACK);
}

void loop() {
  int j;
  int startY = HUD_height+8;
  int startX = 8;
  int textH = 12;
  digitalWrite(LED, LOW);
 
  /*
  float h = myAHT10.readHumidity();
  // Read temperature as Celsius (the default)
  float t = myAHT10.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(30, 30);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.println("FAILED TO READ SENSOR..");
 // tft.invertDisplay(true);
  delay(500);
 // tft.invertDisplay(false);
  delay(500);
//  tft.invertDisplay(true);
  delay(500);
 // tft.invertDisplay(false);
  delay(500);
  digitalWrite(LED, LOW);
   
    return;
  }
*/
  // Compute heat index in Fahrenheit (the default)
//  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
 // float hic = dht.computeHeatIndex(t, h, false);
 
  //tft.setTextWrap(false);
   
 // tft.fillScreen(ST77XX_BLACK);
  //tft.invertDisplay(true);

   tft.fillRect(83, startY, 100, 32, ST77XX_BLACK);
   
   tft.fillRect(80, startY+25, 80, 15, ST77XX_BLACK);
   //float tempF = 0;
   //float tempC = 0;
   //tempC = myAHT10.readTemperature();
   //tempF = (tempC*1.8)+32;
   myAHT10.getEvent(&humidity, &temp);
  tft.setCursor(startX, startY);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.println("Temperature: ");
  tft.setCursor(90, startY);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  //tft.println(tempC);
  tft.println(temp.temperature);
  Serial.println(temp.temperature);

  tft.drawCircle(125,startY-1,2,ST77XX_GREEN);
  // tft.drawCircle(x, y, radius, color);
  tft.setCursor(130, startY);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.println("C");

 

  tft.setCursor(startX, startY+textH);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.println("Temperature: ");
  tft.setCursor(90, startY+textH);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  //tft.println(tempF);
  tft.println(temp.temperature);

  tft.drawCircle(125,startY+textH-1,2,ST77XX_GREEN);
  // tft.drawCircle(x, y, radius, color);
  tft.setCursor(130, startY+textH);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.println("F");

  
  tft.setCursor(startX, startY+(2*textH));
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.println("Humidity: ");
 
  //tft.drawRect(x, y, width, height, color);
  tft.setCursor(80, startY+(2*textH));
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  //tft.println(myAHT10.readHumidity());
  tft.println(humidity.relative_humidity);
  Serial.println(humidity.relative_humidity);
  tft.setCursor(115, startY+(2*textH));
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.println("%");

  delay(1000); // wait between measurments 

for(j = 0; j < 100; j++){
  tft.fillRect(60, 65, 90, 75, ST77XX_BLACK);
  
  tft.setCursor(5, 70);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setTextSize(1);
  tft.println("cooldown:");
  
  tft.setCursor(65, 70);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.println(99-j);
  delay(100);
}
digitalWrite(LED, HIGH);
  /*
  tft.invertDisplay(true);
  delay(500);
  tft.invertDisplay(false);
  delay(500);
  */
  Zeit();
  /*
  if(f > 82)  {
     for(j=0 ; j < 4; j++)  {
          digitalWrite(LED, HIGH);
          delay(100);
          digitalWrite(LED, LOW);
          delay(100);
    }
  }
  else;
  */
}

/**********************************/
void tempMeas(){
  
}

void Zeit(){
 
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println("GOT TIME!");
  tft.fillRect(screenWidth/2-10, HUD_height - 12, 38,12, ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(68, 8);
  tft.setTextSize(1);
  tft.println(&timeinfo, "%H:%M");

  
}


void logo(uint16_t color){
   
//void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
  tft.drawTriangle(80,10,70,30,80,50, color);
  tft.fillTriangle(80,10,90,30,80,50, color);

  tft.fillTriangle(79,60,75,70,79,80, color);
  tft.drawTriangle(80,60,85,70,80,80, color);
 
  tft.drawCircle(100,45,12, color); //right side
  tft.drawCircle(95,55,9, color);
  tft.drawCircle(65,55,9, color);

  tft.drawChar(40, 60, 'B', color,1,2);
  tft.drawChar(110, 60, 'K', color,1,2);
  tft.drawCircle(60,45,12, color);
  tft.drawCircle(71,65,7, color);
  tft.drawCircle(89,65,7, color);
  tft.fillCircle(95,55,7, ST77XX_BLACK);
  tft.fillCircle(65,55,7, ST77XX_BLACK);
  tft.fillCircle(98,61,4, ST77XX_BLACK);
  tft.fillCircle(62,61,4, ST77XX_BLACK);
  tft.fillCircle(80,55, 4, color);
  // tft.drawCircle(125,14,2,ST77XX_GREEN);
  //tft.drawTriangle(w, y, y, x, z, x, color);
}
void welcome(){
  tft.setCursor(30, 20);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.println("SaTC_EDU");

  tft.setCursor(30, 50);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.println("Welcome");
}

void heatGraph(){
  int graphH = 60;
  int graphW = 120;

  
}
void HUD(){
  int lineHeight = 1;
  tft.fillRect(0, 0, 160,HUD_height, ST77XX_BLACK); // black backround 
  tft.setCursor(5,HUD_height-10);
  tft.setTextColor(HUD_color);
  tft.setTextSize(1);
  tft.print("SatC_EDU");
  tft.fillRect(0,HUD_height, screenWidth, 2,HUD_color);

  Wifibars();
}

void Wifibars(){
   WiFi.begin(ssid, password);
   long rssi =-1* WiFi.RSSI();
   int barWidth = 4 ;
   int x = screenWidth - barWidth*6;
   int y = abs(HUD_height-5);

   tft.setCursor(x-35,y+55);
   tft.setTextColor(ST77XX_RED);
   tft.print("RSSI:");
   tft.setCursor(x-5,y+55);
   tft.print(rssi);
   if(rssi <= 55){
       tft.fillRect(x, y, 3, -4,  ST77XX_WHITE);
       tft.fillRect(x+barWidth, y, 3, -6, ST77XX_WHITE);
       tft.fillRect(x+2*barWidth, y, 3, -8, ST77XX_WHITE);
       tft.fillRect(x+3*barWidth, y, 3, -10, ST77XX_WHITE);
   }
   else if(rssi <= 100){
       tft.fillRect(x, y, 3, -4,  ST77XX_RED);
       tft.fillRect(x+barWidth, y, 3, -6, ST77XX_WHITE);
       tft.fillRect(x+2*barWidth, y, 3, -8, ST77XX_WHITE);
       tft.fillRect(x+3*barWidth, y, 3, -10, ST77XX_WHITE);
   }
   else if(rssi <= 175){
        tft.fillRect(x, y, 3, -4,  ST77XX_RED);
       tft.fillRect(x+barWidth, y, 3, -6, ST77XX_RED);
       tft.fillRect(x+2*barWidth, y, 3, -8, ST77XX_WHITE);
       tft.fillRect(x+3*barWidth, y, 3, -10, ST77XX_WHITE);
   }
   else if(rssi <= 255){
        tft.fillRect(x, y, 3, -4,  ST77XX_RED);
       tft.fillRect(x+barWidth, y, 3, -6, ST77XX_RED);
       tft.fillRect(x+2*barWidth, y, 3, -8, ST77XX_RED);
       tft.fillRect(x+3*barWidth, y, 3, -10, ST77XX_WHITE);
   }
   else if(rssi < 55){
       tft.fillRect(x, y, 3, -4,  ST77XX_RED);
       tft.fillRect(x+barWidth, y, 3, -6, ST77XX_RED);
       tft.fillRect(x+2*barWidth, y, 3, -8, ST77XX_RED);
       tft.fillRect(x+3*barWidth, y, 3, -10, ST77XX_RED);
   }
   else;
  // tft.setCursor(20,20);
  // tft.print(rssi);
  WiFi.disconnect(true);
}

void testlines(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, 0, x, tft.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, 0, tft.width()-1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, 0, x, tft.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, 0, 0, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, tft.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, tft.height()-1, tft.width()-1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, 0, y, color);
    delay(0);
  }
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t y=0; y < tft.height(); y+=5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x=0; x < tft.width(); x+=5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}


void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < tft.width(); x+=radius*2) {
    for (int16_t y=radius; y < tft.height(); y+=radius*2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 0xF800;
  int t;
  int w = tft.width()/2;
  int x = tft.height()-1;
  int y = 0;
  int z = tft.width();
  for(t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  tft.fillScreen(ST77XX_BLACK);
  int color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = tft.width()-2;
    int h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}