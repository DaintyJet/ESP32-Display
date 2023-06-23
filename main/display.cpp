// C Libs
#include <time.h>

// ESP Libs
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
//Display Libs
// Not possible to use the ST7735.h lib.. atm, hence this will be added to a branch
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
// WiFi
#include "WiFi.h"
// AHT
#include "aht.h"


const char* TAG = "DisplayTest";

// Screen Macros
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
#define LED            GPIO_NUM_2 // on-board LED
// AHT pins
#define SDA            GPIO_NUM_27
#define SCL            GPIO_NUM_33

// For ST7735-based display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//Global variables
const float p = 3.1415926;

// Wifi Defines
// WIFI Defines
// These are from the station example reference linked to at the wifi function comment
/* The examples use WiFi configuration that you can set via project configuration menu
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
// End WIFI

// NTP constants
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -5 *3600;//for timezone
const int   daylightOffset_sec = 3600;

// Forward Definitions
int aht_read(float* temperature, float* humidity);
void init_ast_dev(void);
void Wifibars(void);
void HUD(void);
void welcome(void);
void logo(uint16_t color);
void Zeit(void);
//void loop(void* args);

aht_t aht_dev;

void setup(void) {  
   
    int i = 0;
    ESP_LOGI(TAG,"Hello! ST77xx TFT Test");
   
    // Configure LED
    // Reset GPIO
    gpio_reset_pin(LED);
    // Set mode of LED GPIO to OUTPUT
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    ESP_ERROR_CHECK(gpio_set_level(LED, HIGH));

    // Confiog nvm
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Config i2c
    ESP_ERROR_CHECK(i2cdev_init());
    vTaskDelay(pdMS_TO_TICKS(100));
   
    // Init ST7735S mini display, using a 0.96" 160x80 TFT:
    tft.initR(INITR_MINI160x80);  
    vTaskDelay(pdMS_TO_TICKS(100));
   
    // Init AHT device
    init_ast_dev();

    // Print the logo
    logo(ST77XX_GREEN);
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Blink LED 4 times... Why?
    for(; i < 4; i++){
        gpio_set_level(LED, HIGH);
        vTaskDelay(pdMS_TO_TICKS(250));
        gpio_set_level(LED, LOW);
        vTaskDelay(pdMS_TO_TICKS(250));
    }

    // Print welcome message
    welcome();
    vTaskDelay(pdMS_TO_TICKS(2000));
   
    // Print HUD
    HUD();
    vTaskDelay(pdMS_TO_TICKS(500));
   
    tft.setCursor(screenWidth-3*screenWidth/4+20, HUD_height-8);
    tft.setTextColor(ST77XX_YELLOW);
    tft.print("Connecting");

    for(i=0; i < 4;i++){
      tft.drawChar(screenWidth-3*screenWidth/4+80+i*2, HUD_height-10,'.',ST77XX_YELLOW,1,1);
      //void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
      vTaskDelay(pdMS_TO_TICKS(250));
    }
   
    // Connect to Wifi and print bars
    WiFi.begin("HOME-9E24_Ext", "triumph98");
vTaskDelay(500);
    vTaskDelay(pdMS_TO_TICKS(500));
    tft.fillRect(screenWidth/2-20, HUD_height - 10, 70,10, ST77XX_BLACK);
}

void loop() {
    static float humidity, temp;
    int j;
    int startY = HUD_height+8;
    int startX = 8;
    int textH = 12;

    // Print Labels
    for (int i = 0; i <= 1; ++i) {
        tft.setCursor(startX, startY + (textH * i));
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println("Temperature: ");
    }

    while (1) {
        gpio_set_level(LED, LOW);

        // Fill Screen of areas with updating text (startY - 3) is for the circle
        tft.fillRect(80, startY - 3 , 80, (80 - (startY - 3)), ST77XX_BLACK);
       
        // Print Bars
        Wifibars();
        // Print time
        Zeit();

        // Read temp and humidity
        aht_read(&temp, &humidity);


        // Print Temp in C
        tft.setCursor(90, startY);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println(temp);
        Serial.println(temp);  
        tft.drawCircle(125,startY-1,2,ST77XX_GREEN);

        tft.setCursor(130, startY);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println("C");
       
        // Print temp in F ... we need to convert!
        tft.setCursor(90, startY + textH);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);        
        tft.println((temp * (9.0/5)) + 32.0);

        tft.drawCircle(125,startY+textH-1,2,ST77XX_GREEN);
        // tft.drawCircle(x, y, radius, color);
        tft.setCursor(130, startY+textH);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println("F");

        // Print Humidity
        tft.setCursor(startX, startY+(2 * textH));
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println("Humidity: ");
   
        //tft.drawRect(x, y, width, height, color);
        tft.setCursor(80, startY + (2 * textH));
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println(humidity);
        Serial.println(humidity);
        tft.setCursor(115, startY + (2 * textH));
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println("%");

        vTaskDelay(pdMS_TO_TICKS(1000)); // wait between measurements

        // Print Cooldown timer and update
        for(j = 0; j < 100; j++){
            tft.fillRect(60, 65, 30, 40, ST77XX_BLACK);
           
            tft.setCursor(5, 70);
            tft.setTextColor(ST77XX_MAGENTA);
            tft.setTextSize(1);
            tft.println("Cooldown:");
           
            tft.setCursor(65, 70);
            tft.setTextColor(ST77XX_YELLOW);
            tft.setTextSize(1);
            tft.println(99-j);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        ESP_ERROR_CHECK(gpio_set_level(LED, HIGH));
    }
}

//********************************************************
// Function: Print current time
// Arguments: none
// Return Value: none
//********************************************************
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

//********************************************************
// Function: Print logo
// Arguments: None
// Return Value: None
//********************************************************
void logo(uint16_t color){
    tft.fillScreen(ST77XX_BLACK);    
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

//********************************************************
// Function: Print welcome message
// Arguments: None
// Return value: None
//********************************************************
void welcome(){
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(30, 20);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.println("SaTC_EDU");

    tft.setCursor(30, 50);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    tft.println("Welcome");
}

//********************************************************
// Function: Print HUD
// Args: none
// Return values: none
//********************************************************
void HUD() {
    tft.fillScreen(ST77XX_BLACK);
    tft.fillRect(0, 0, 160,HUD_height, ST77XX_BLACK); // black backround
    tft.setCursor(5,HUD_height-10);
    tft.setTextColor(HUD_color);
    tft.setTextSize(1);
    tft.print("SatC_EDU");
    tft.fillRect(0,HUD_height, screenWidth, 2,HUD_color);
}

//********************************************************
// Function: Print WiFi based in rssi values
// Arguments: none
// Return Values: none
//********************************************************
void Wifibars(){
    long rssi =-1* WiFi.RSSI();
    int barWidth = 4 ;
    int x = screenWidth - barWidth*4;
    int y = abs(HUD_height-5);
    
    tft.setCursor(x-35,y+55);
    tft.setTextColor(ST77XX_RED);
    tft.print("RSSI:");
    tft.setCursor(x-5,y+55);
    tft.print(rssi);
    
    // Fill Bars (Values are based on various online sources)
    tft.fillRect(x, y, 3, -4, (rssi <= 88) ? ST77XX_RED  : ST77XX_WHITE);
    tft.fillRect(x+barWidth, y, 3, -6, (rssi <= 77) ? ST77XX_RED  : ST77XX_WHITE);
    tft.fillRect(x+2*barWidth, y, 3, -8, (rssi <= 66) ? ST77XX_RED  : ST77XX_WHITE);
    tft.fillRect(x+3*barWidth, y, 3, -10, (rssi <= 55) ? ST77XX_RED  : ST77XX_WHITE ); 
}
//********************************************************
// Function: Utilize ESP32 AHT Library to load values into
// The pontes float arguments
//
// Arguments:
// float* temperature: Pointer to float, temp value read from
// the AHT will be stored here after return (If return of func is 1)
//
// float* humidity: Pointer to humidity value read from the AHT
// will be stored here after return (IF return of func is 1)
//
// Return Value:
// 1: Function successfully read Temp and Humidity values
// 0: otherwise
//
// Ref: https://github.com/UncleRus/esp-idf-lib/tree/master/examples/aht/default
//********************************************************
int aht_read(float* temperature, float* humidity) {

  // Calibrate AHT Device
  bool calibrated;
  ESP_ERROR_CHECK(aht_get_status(&aht_dev, NULL, &calibrated));
  if (calibrated)
      ESP_LOGI(TAG, "Sensor calibrated");
  else
      ESP_LOGW(TAG, "Sensor not calibrated!");
 
  // Read from device, write the values to pointers passed to the function.
  // Also Print values here.
  esp_err_t res = aht_get_data(&aht_dev, temperature, humidity);
  if (res != ESP_OK) {
     ESP_LOGE(TAG, "Error reading data: %d (%s)", res, esp_err_to_name(res));     
     vTaskDelay(pdMS_TO_TICKS(500));
     return 0;
  }
   
  ESP_LOGI(TAG, "\nTemperature: %.1f°C, Humidity: %.2f%%\nTemperature: %.1f°F, Humidity: %.2f%%", *temperature, *humidity, ((*temperature) * (9.0/5.0) + 32.0), *humidity);

  return 1;  
}

//**************************************
// Function: Given an AHT device initialize it, check it's calibration and
// Print to the LCD display
// Arguments: dev => aht object
// Return value: none
//**************************************
void init_ast_dev() {
    // Setup Display printout
    tft.setRotation(3);  
    tft.fillScreen(ST77XX_BLACK);
    tft.invertDisplay(true);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(20,25);
    tft.print("Connecting to sensor...");
   
    // Init AHT Print Status
    while (aht_init_desc(&aht_dev, 56, (i2c_port_t)0, SDA, SCL) != ESP_OK)
    {
        tft.print("Sensor not found");
        ESP_LOGI(TAG, "AHT10 not connected or fail to load calibration coefficient"); //(F()) save string to flash & keeps dynamic memory free
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    vTaskDelay(pdMS_TO_TICKS(2000));

    // clear screen
    tft.fillScreen(ST77XX_BLACK);
    tft.invertDisplay(true);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(20,25);
    // Calibrate AHT and print
    bool calibrated;
    ESP_ERROR_CHECK(aht_get_status(&aht_dev, NULL, &calibrated));
    if(calibrated) {
        tft.println("Senor calibrated");
        ESP_LOGI(TAG, "Sensor calibrated");
    }
    else {
        tft.println("Senor not calibrated");
        ESP_LOGI(TAG, "Sensor not calibrated");
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
    return;
}