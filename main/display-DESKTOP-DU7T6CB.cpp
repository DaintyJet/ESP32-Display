// C Libs
#include <time.h>

// ESP Libs
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
// NTP Libs 
#include "esp_sntp.h"
//Display Libs
// Not possible to use the ST7735.h lib.. atm, hence this will be added to a branch
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
//#include <SPI.h>
//#include <Wire.h>
// AHT
#include "aht.h"


const char* TAG = "DisplayTest";


/* ESP get RSSI

 wifi_ap_record_t ap;
esp_wifi_sta_get_ap_info(&ap);
printf("%d\n", ap.rssi);*/

//#include <AHT10.h> // there is no <AHT10.h> in official Library, replaced with <Adafruit_AHT10.h>
//#include <Adafruit_AHT10.h>

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
#define SDA            27
#define SCL            33

//  reference for ESP32 SatC_EDU REV-3 board's physical pins connections to VSPI:
// SDA  GPIO23 aka VSPI MOSI
// SCLK GPIO18 aka SCK aka VSPI SCK
// D/C  GPIO15 aka A0 (also I2C SDA)
// RST  GPIO4 aka RESET (also I2C SCL) 
// CS   GPIO5  aka chip select
// LED  3.3V
// VCC  5V
// GND - GND
  
// For ST7735-based display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//Global variables
const float p = 3.1415926;




const char* ssid = "SSID";
const char* password =  "password";

// Wifi Defines 
// WIFI Defines
// These are from the station example reference linked to at the wifi function comment
/* The examples use WiFi configuration that you can set via project configuration menu
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
const unsigned char EXAMPLE_ESP_WIFI_SSID[32] = CONFIG_ESP_WIFI_SSID;
const unsigned char EXAMPLE_ESP_WIFI_PASS[64] = CONFIG_ESP_WIFI_PASSWORD;
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
// END WIFI DEFINES


// Forward Definitions
void wifi_init_sta(void);
int aht_read(float* temperature, float* humidity);
void init_ast_dev(void);
void Wifibars(void);
void HUD(void);
void welcome(void);
void logo(uint16_t color);
void Zeit(void);
void loop(void* args);

aht_t aht_dev;

void app_main(void) {  
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
    init_ast_dev();

    logo(ST77XX_GREEN);
    vTaskDelay(pdMS_TO_TICKS(1500));

    // Blink LED 4 times... Why?
    for(; i < 4; i++){
        gpio_set_level(LED, HIGH);
        vTaskDelay(100);
        gpio_set_level(LED, LOW);
        vTaskDelay(100);
    }

    welcome();
    vTaskDelay(1000);
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
    
    // Connect to Wifi
    wifi_init_sta();
    Wifibars();

    vTaskDelay(pdMS_TO_TICKS(500));
    tft.fillRect(screenWidth/2-20, HUD_height - 10, 70,10, ST77XX_BLACK);
    xTaskCreate(&loop, "Display Loop", 8192, NULL, 1, NULL);
}

void loop(void* args) {
    static float humidity, temp;
    int j;
    int startY = HUD_height+8;
    int startX = 8;
    int textH = 12;
    while (1) {
        gpio_set_level(LED, LOW);

        tft.fillRect(83, startY, 100, 32, ST77XX_BLACK);
    
        tft.fillRect(80, startY+25, 80, 15, ST77XX_BLACK);
        
        aht_read(&humidity, &temp);
        tft.setCursor(startX, startY);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println("Temperature: ");
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

        tft.setCursor(startX, startY+textH);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println("Temperature: ");
        tft.setCursor(90, startY+textH);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        //tft.println(tempF);
        tft.println(temp);

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
        tft.println(humidity);
        Serial.println(humidity);
        tft.setCursor(115, startY+(2*textH));
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(1);
        tft.println("%");

        vTaskDelay(pdMS_TO_TICKS(1000)); // wait between measurments 

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
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        ESP_ERROR_CHECK(gpio_set_level(LED, HIGH));
        Zeit();

    }
}

//********************************************************
// Function: Print current time
// Arguments: none
// Return Value: none
//********************************************************
void Zeit(){    
    // Time vars
    struct tm timeinfo;
    time_t now;
    char strtime_buff[64];
    
    // Set timezone tp EST
    setenv("TZ","EST5EDT,M3.2.0/2,M11.1.0",1);
    tzset();

    // Get time
    localtime_r(&now, &timeinfo);

    // Turn time value into str
    strftime(strtime_buff, sizeof(strtime_buff), "%c", &timeinfo);

    ESP_LOGI(TAG,"GOT TIME!");
    tft.fillRect(screenWidth/2-10, HUD_height - 12, 38,12, ST77XX_BLACK);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(68, 8);
    tft.setTextSize(1);
    tft.println(strtime_buff);
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
    // Wifi is already connected

    // Get the RSSI 
    wifi_ap_record_t ap;
    esp_wifi_sta_get_ap_info(&ap);
    printf("%d\n", ap.rssi);

    long rssi = -1 * ap.rssi;
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
    else{};
}


//********************************************************
// Function: This is a function called on the completion of an event
// This is used for the WIFI functions
// Arguments: argument list, event, event_id and data (generic)
// Return Value: None
// Ref: https://github.com/espressif/esp-idf/tree/master/examples/wifi/getting_started/station
//********************************************************
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    static int s_retry_num = 0;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


//********************************************************
// Function: Initialize WiFi, blocking operation
// Arguments: None
// Return Value: None
//
// Ref: https://github.com/espressif/esp-idf/tree/master/examples/wifi/getting_started/station
//
// NOTE: We can use a helper function instep of this as referenced 
// https://github.com/espressif/esp-idf/tree/master/examples/protocols
//********************************************************
void wifi_init_sta(void)
{
    
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
  
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    wifi_sta_config_t msta;
    memcpy(msta.ssid, EXAMPLE_ESP_WIFI_SSID, 32);
    memcpy(msta.password, EXAMPLE_ESP_WIFI_PASS, 64);
    // Worst case is I remove the following two
    msta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
    msta.sae_pwe_h2e = ESP_WIFI_SAE_MODE;
    
    wifi_config_t wifi_config;
    wifi_config.sta = msta;
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
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
     ESP_LOGE(TAG, "Error reading data: %d (%s)", res, esp_err_to_name(res));     vTaskDelay(pdMS_TO_TICKS(500));
     return 0;
  }
    
  ESP_LOGI(TAG, "Temperature: %.1f°C, Humidity: %.2f%%", *temperature, *humidity);
  return 1;  
}

//**************************************
// Function: Given an AHT device initalize it, check it's calibration and 
// Print to the LCD display
// Arguments: dev => aht object 
// Return value: none
//**************************************
void init_ast_dev() {
    // Setup Disaply printout
    tft.setRotation(3);   
    tft.fillScreen(ST77XX_BLACK);
    tft.invertDisplay(true);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(20,25);
    size_t size = tft.print("connecting to sensor...");
    
    // Init AHT Print Status
    while (aht_init_desc(&aht_dev, AHT_IC2_ADDRESS_GND, 0, SDA, SCL) != ESP_OK)
    {
        tft.print("Sensor not found");
        ESP_LOGI(TAG, "AHT10 not connected or fail to load calibration coefficient"); //(F()) save string to flash & keeps dynamic memory free
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    // clear screen
    vTaskDelay(pdMS_TO_TICKS(100));
    tft.fillScreen(ST77XX_BLACK);
    tft.invertDisplay(true);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(20,25);
    // Calibrate AHT and print
    bool calibrated;
    ESP_ERROR_CHECK(aht_get_status(&aht_dev, NULL, &calibrated));
    if(calibrated) {
        tft.println("Sesor calibrated");
        ESP_LOGI(TAG, "Sensor calibrated");
    }
    else {
        tft.println("Sesor not calibrated");
        ESP_LOGI(TAG, "Sensor not calibrated");
    }
    return;
}