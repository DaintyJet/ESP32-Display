# ESP32 Display Lab/Demo

In this exercise students will use the LCD display of the SaTC ESP32 kit to display information regarding the current temperature and humidity readings. This is a Arduino project with the ESP-IDF toolchain using the [ESP-Arduino Core](www.google.com) as a component.

Something to note is that the ESP-IDF arduino library, at the time of writing (6/1/23) only works with versions of ESP-IDF between *4.4* and *4.4.99* or 5.1. The branch for version 5.1 refers to the the pre-release version of esp-idf version 5.1, which may be unstable.

## Anatomy of an Aduino Project

An Arduino project consists of at least two functions. The *setup* and *loop* functions. The entrypoint of the program is the *setup* function, this is often used to configure the device and any attached components. Once the *setup* function has ended and returned, the *loop* function will begin to run, and will continue to be called each time it completes a pass. This is called a *super-loop* architecture, and does not take advantage of the Real Time Operating System (RTOS) capabilities of the ESP32.

## Starting this project
1. Download the git repository and it's submodules. The use of && \ allows this to be ran as one large command.
    ```sh
    git clone <INSERT FINAL LINK HERE> && \  # Clone the repository 
    cd ESP-Display && \                      # Enter into repository folder
    git submodule init && \                  # Init submodules
    git submodule update                     # Install submodules
    ```
    **Note**: By default, this project is already located in the ``` ~/esp/IoT-Examples/ ``` directory of the Ubuntu VM.
## Arduino as an ESP-IDF Component ESP version 5.1
We will need to use the ESP-IDF version 4.4 or 5.1 if we want to use the ESP-IDF to build and flash this project. The following are all necessary steps to add the Arduino core as a ESP-IDF component. **Note** that this will have already been done for you when using the provided VM.

### ESP-IDF Setup
**NOTICE**: This has already been done if you are using the provided Virtual Machine
1. Enter into the esp-idf directory or install [esp-idf](https://github.com/espressif/esp-idf) as shown below.
    ```sh
    # Installing ESP-IDF
    cd ~ && \                                           # Enter into the home directory 
    cd esp && \                                         # Enter into the newly create folder
    git clone https://github.com/espressif/esp-idf.git  # Clone esp-idf
    ```
2. Switch the esp-idf repository to release version 5.1
    ```sh 
    git switch release/v5.1
    ```
3. Run the install script located in the newly cloned ``` esp-idf ``` repository.
    ```sh 
    ./install.sh
    ```

### Arduino component setup
The following instructions should be done within the *project repository* ``` ESP-DISPLAY ```. Additionally if you downloaded this repository correctly **steps 2 through 7 will already have been done for you**. When using the *provided VM* all of this has already been done for you.

**Notice**: When running a project with the arduino core for the first time the ``` idf.py ``` commands will *fail*. We need to edit the generated SDKCONFIG to increase the ``` CONFIG_FREERTOS_HZ ``` value. Use your preferred editor.
``` 
# Before 
CONFIG_FREERTOS_HZ=100

# After 
CONFIG_FREERTOS_HZ=1000
```

1. Enter into the project directory if you are not already there
2. Now **Install** the [arduino-esp library](https://github.com/espressif/arduino-esp32) into a folder named arduino
    ```sh
    # Clone into arduino-esp32 repository into a directory ./components/arduino
    git clone https://github.com/espressif/arduino-esp32 components/arduino  # Clone arduino core
    ```
3. **Install** the [Adafruit-ST7735 Display Library](https://github.com/adafruit/Adafruit-ST7735-Library)
    ```sh
    # Clone into Adafruit-ST7735-Library repository into a directory ./components/Adafruit-ST7735-Library
    git clone https://github.com/DaintyJet/Adafruit-ST7735-Library components/Adafruit-ST7735-Library
    ```
4. **Install** the [Arduino Core Graphics Library](https://github.com/adafruit/Adafruit-GFX-Library)
    ```sh
    # Clone into Adafruit-GFX-Library repository into a directory ./components/Adafruit-GFX-Library
    git clone https://github.com/adafruit/Adafruit-GFX-Library components/Adafruit-GFX-Library
    ```
5. **Install** the [Arduino BusIO Library](https://github.com/adafruit/Adafruit_BusIO)
    ``` 
    # Clone into Adafruit_BusIO repository into a directory ./components/Adafruit_BusIO
    git clone https://github.com/adafruit/Adafruit_BusIO components/Adafruit_BusIO
    ```
6. Enter into the component arduino library
    ```sh
    cd components/arduino 
    ```
7. Switch the branch used by the arduino repository
    ```sh
    git switch esp-idf-v5.1-libs
    ```
8. Modify the ``` components/arduino/CMakeList.txt ``` so the esp_partition can be located.
    ```
    # Before
    set(requires spi_flash mbedtls mdns wifi_provisioning wpa_supplicant esp_adc esp_eth http_parse)

    # After
    set(requires spi_flash mbedtls mdns wifi_provisioning wpa_supplicant esp_adc esp_eth http_parser esp_partition)
    ``` 
9.  Move back to the *ESP32-Display* project directory, otherwise the menuconfig will not work.
    ```sh
    cd ../..
    ```
10. Enabled backwards compatibility within FREERTOS
    ```sh
    # compoent config -> FreeRtos -> Kernel -> configENABLE_BACKWARDS_COMPATIBILITY 
    idf.py menuconfig 
    ```
11. Enable Enable pre-shaired-cipher suites 
    ```sh
    #  Component config -> mbedTLS -> TLS Key Exchange Methods -> Enable pre shared-key ciphersuites
    #  Component config -> mbedTLS -> TLS Key Exchange Methods -> Enable PSK based ciphersuite modes
    idf.py menuconfig   
    ```
12. Add necessary component dependency
    ```sh
    idf.py add-dependency "espressif/mdns^1.1.0"
    ```

**Note when moving between versions**: run ``` idf.py clean ``` and ``` idf.py fullclean ```

## Project Flashing
1. Build the project using the build button located at the bottom of VS Code, or run the command ``` idf.py build ``` in an esp-idf terminal. 
2. Flash the project using the flash button located at the bottom of VS Code, or run the command ``` idf.py flash ``` in an esp-idf terminal. 
3. Start a Serial Monitor using the button located at the bottom of VS Code, or run the command ``` idf.py monitor ``` in an esp-idf terminal. 
4. Observe the visual, and serial outputs 

**Note**: We can combine all three commands with one button (build flash monitor) or use the terminal ``` idf.py build flash monitor ```

## Components used
The following are the components used in this ESP-IDF project 

* Display: https://github.com/adafruit/Adafruit-ST7735-Library
   * We need to add a CMakeList.txt file in the display library so it will be discovered, and linked. This has already been done in this project (Hence why the library is a fork and submodule!) 
        ```
        cmake_minimum_required(VERSION 3.16)

        idf_component_register(SRCS "Adafruit_ST77xx.cpp" "Adafruit_ST7735.cpp" "Adafruit_ST7789.cpp" 
                                INCLUDE_DIRS "."
                                REQUIRES arduino Adafruit_BusIO Adafruit-GFX-Library)

        project(Adafruit-ST7735-Library)

        ```

* Core Graphics Library: https://github.com/adafruit/Adafruit-GFX-Library

* BusIO Library: https://github.com/adafruit/Adafruit_BusIO

* ESP32 Arduino Core: https://github.com/espressif/arduino-esp32   