file(GLOB_RECURSE SOURCES "*.cpp")

# Globally add Adafruit_BusIO header include directory for all components
file(GLOB_RECURSE BUSIO_HEADERS "${CMAKE_BINARY_DIR}/../pio_components/*/Adafruit_BusIO/Adafruit_I2CDevice.h" "${CMAKE_BINARY_DIR}/../../pio_components/*/Adafruit_BusIO/Adafruit_I2CDevice.h" "C:/esphb/*/Adafruit_I2CDevice.h")
foreach(BUSIO_HEADER ${BUSIO_HEADERS})
    get_filename_component(BUSIO_DIR ${BUSIO_HEADER} DIRECTORY)
    include_directories(SYSTEM "${BUSIO_DIR}")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} APPEND PROPERTY INCLUDE_DIRECTORIES "${BUSIO_DIR}")
    message(STATUS "[Clockwise] Added global include directory for Adafruit_BusIO: ${BUSIO_DIR}")
    
    get_filename_component(BUSIO_COMP_DIR ${BUSIO_DIR} DIRECTORY)
    set(GFX_CMAKE "${BUSIO_COMP_DIR}/Adafruit_GFX_Library/CMakeLists.txt")
    if(EXISTS ${GFX_CMAKE})
        file(READ ${GFX_CMAKE} GFX_CONTENT)
        if(NOT GFX_CONTENT MATCHES "Adafruit_BusIO")
            string(REPLACE "idf_component_register(" "idf_component_register(\n    REQUIRES Adafruit_BusIO\n" GFX_NEW "${GFX_CONTENT}")
            file(WRITE ${GFX_CMAKE} "${GFX_NEW}")
            message(STATUS "[Clockwise] Patched Adafruit_GFX_Library CMakeLists.txt at ${GFX_CMAKE}")
        endif()
    endif()
endforeach()

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "fonts"
    REQUIRES esphome wifi arduino esp_http_client Adafruit_BusIO Adafruit_GFX_Library ArduinoJson PNGdec
)
