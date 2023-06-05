# ESP32 Display Lab/Demo

In this exercise students will use the LCD display of the SaTC ESP32 kit to display information regarding the current temperature and humidity readings. This is based off of an Arduino project XXX.

Something to note is that the ESP-IDF arduino library, at the time of writing (6/1/23) only works with versions of ESP-IDF between *4.1* and *4.9*. Additionally, the workarounds to this problem are currently unstable.

## Anatomy of an Aduino Project

An Arduino project consists of at least two functions. The **setup** and **loop** functions. The entrypoint of the program is the **setup** functions, this is often used to configure the device and any attached components. Once the **setup** function has returned, the **loop** function will run, and will continue to be called each time it completes. This architecture is called the **super-loop**, and does not take advantage of the Real Time Operating System (RTOS) capabilities of the ESP32.

## Start this project
1. Download the git repository and it's submodules.
   * ``` git clone <INSERT FINAL LINK HERE>```
   * ``` cd ESP-Display ```
   * ``` git submodule init ```
   * ``` git submodule update ```


## Arduino as an ESP-IDF Component
We will need to use the ESP-IDF version 4.4 if we want to use it to build and flash this project. The following are all necessary steps to add the Arduino core as a ESP-IDF component. **Note** that steps 1, 2, and 3 may have already been done for you.

1. Install the [arduino-esp library](https://github.com/espressif/arduino-esp32) into a folder named arduino
    ```sh
    cd components
    git clone https://github.com/espressif/arduino-esp32 arduino
    ```
2. Install [esp-idf](https://github.com/espressif/esp-idf), we need to do this as the installed version at ``` /home/iot/esp/esp-idf ``` does not have the ``` release/v4.4 ``` branch which is necessary.
3. Enter the esp-idf repository, and switch the brach to ``` release/v4.4 ```
    ```sh
    git switch release/v4.4
    ```
4. Set the ``` IDF_PATH ``` environment variable to the esp-idf version 4.4 folder.
    ```sh
    # Example 
    export IDF_PATH=/home/iot/esp-4.4/esp-idf
    ```
5. Now follow the [install instructions](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
   * ``` ./install.sh  ```
   * ``` . ./export.sh ```
6. Run the idf.py build command in the ESP-Display project folder
   * This will install a number of components the first time you start a project 
   * If this fails for whatever reason (interrupted, exited, etc.) you can run the following command from the esp-idf directory
        ``` git submodule update --init --recursive ```


## Components 
Components used:

Display: https://github.com/adafruit/Adafruit-ST7735-Library
* We need to add a CMakeList.txt file in the display library so it will be discovered, and linked. This has already been done in this project (Hence why the library is not a submodule!) 
    ```
    idf_component_register(SRCS "Adafruit_ST77xx.cpp" "Adafruit_ST7735.cpp" "Adafruit_ST7789.cpp" 
                        INCLUDE_DIRS "."
                        REQUIRES arduino Adafruit_BusIO Adafruit_GFX)
    ```

Core Graphics Library: https://github.com/adafruit/Adafruit-GFX-Library
* We need to modify a CMakeList.txt file in the core graphics library so it will be discovered, and linked. This has already been done in this project (Hence why the library is not a submodule!) 
    ```
    idf_component_register(SRCS "Adafruit_GFX.cpp" "Adafruit_GrayOLED.cpp" "Adafruit_SPITFT.cpp" "glcdfont.c"
                       INCLUDE_DIRS "."
                       REQUIRES arduino Adafruit_BusIO)
    ```
BusIO Library: https://github.com/adafruit/Adafruit_BusIO
* Just as with the core graphics library  we need to modify the CMake file
    ```
    idf_component_register(SRCS "Adafruit_I2CDevice.cpp" "Adafruit_BusIO_Register.cpp" "Adafruit_SPIDevice.cpp" 
                        INCLUDE_DIRS "."
                        REQUIRES arduino)
    ```