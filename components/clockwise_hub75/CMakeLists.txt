file(GLOB_RECURSE SOURCES "*.cpp")

# Locate Adafruit_GFX_Library CMakeLists.txt in pio_components relative to CMAKE_BINARY_DIR and inject REQUIRES Adafruit_BusIO
file(GLOB_RECURSE ALL_CMAKES 
    "${CMAKE_BINARY_DIR}/../../../pio_components/*CMakeLists.txt"
    "${CMAKE_BINARY_DIR}/../../../../pio_components/*CMakeLists.txt"
    "C:/esphb/*CMakeLists.txt"
)

foreach(GFX_CMAKE ${ALL_CMAKES})
    if(GFX_CMAKE MATCHES "Adafruit_GFX_Library/CMakeLists.txt")
        if(EXISTS ${GFX_CMAKE})
            file(READ ${GFX_CMAKE} GFX_CONTENT)
            if(NOT GFX_CONTENT MATCHES "Adafruit_BusIO")
                string(REPLACE "idf_component_register(" "idf_component_register(\n    REQUIRES Adafruit_BusIO\n" GFX_NEW "${GFX_CONTENT}")
                file(WRITE ${GFX_CMAKE} "${GFX_NEW}")
                message(STATUS "[Clockwise] Successfully patched Adafruit_GFX_Library CMakeLists.txt at ${GFX_CMAKE}")
            endif()
        endif()
    endif()
endforeach()

# Also locate Adafruit_BusIO header directory and add to global include directories
file(GLOB_RECURSE BUSIO_HEADER 
    "${CMAKE_BINARY_DIR}/../../../pio_components/*Adafruit_I2CDevice.h"
    "C:/esphb/*Adafruit_I2CDevice.h"
)
if(BUSIO_HEADER)
    list(GET BUSIO_HEADER 0 FIRST_BUSIO)
    get_filename_component(BUSIO_DIR ${FIRST_BUSIO} DIRECTORY)
    include_directories("${BUSIO_DIR}")
    message(STATUS "[Clockwise] Global include added for Adafruit_BusIO: ${BUSIO_DIR}")
endif()

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES esphome wifi arduino esp_http_client Adafruit_BusIO Adafruit_GFX_Library ArduinoJson PNGdec
)
