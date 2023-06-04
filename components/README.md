Possibly add a CMakefile to the ST77xx  
```
cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "Adafruit_ST77xx.cpp" "Adafruit_ST7735.cpp" "Adafruit_ST7789.cpp" 
                       INCLUDE_DIRS ".")

project(Adafruit_ST77xx)
```
