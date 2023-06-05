# ESP32 Display Lab/Demo

In this exercise students will use the LCD display of the SaTC ESP32 kit to display information regarding the current temperature and humidity readings. This is based off of an Arduino project XXX.

Something to note is that the ESP-IDF arduino library, at the time of writing (6/1/23) only works with versions of ESP-IDF between *4.1* and *4.9*. Additionally, the workarounds to this problem are currently unstable.

## Anatomy of an Aduino Project

An Arduino project consists of at least two functions. The **setup** and **loop** functions. The entrypoint of the program is the **setup** functions, this is often used to configure the device and any attached components. Once the **setup** function has returned, the **loop** function will run, and will continue to be called each time it completes. This architecture is called the **super-loop**, and does not take advantage of the Real Time Operating System (RTOS) capabilities of the ESP32.

## Attempt to make into ESP project
We will need to use the ESP-IDF version 4.4 if we want to use it to build and flash this project. The following are all necessary steps to add the Arduino core as a ESP-IDF component. **Note** that steps 1, 2, and 3 may have already been done for you.

1. Install the [arduino-esp library](https://github.com/espressif/arduino-esp32) into a folder named arduino
    ```sh
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


## Components 
Components used:

Display: https://github.com/adafruit/Adafruit-ST7735-Library
* We need to add a CMakeList.txt file to the display library so it will be discovered, and linked. This has already been done in this project (Hence why the library is not a submodule!) 
    ```
    cmake_minimum_required(VERSION 3.5)

    idf_component_register(SRCS "Adafruit_ST77xx.cpp" "Adafruit_ST7735.cpp" "Adafruit_ST7789.cpp" 
                        INCLUDE_DIRS ".")

    project(Adafruit_ST77xx)
    ```

Core Graphics Library: https://github.com/adafruit/Adafruit-GFX-Library

BusIO Library: https://github.com/adafruit/Adafruit_BusIO