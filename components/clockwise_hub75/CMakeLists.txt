file(GLOB_RECURSE SOURCES "*.cpp")

# Globally add Adafruit_BusIO header include directory for all components
file(GLOB_RECURSE BUSIO_HEADERS "${CMAKE_BINARY_DIR}/../pio_components/*/Adafruit_BusIO/Adafruit_I2CDevice.h" "${CMAKE_BINARY_DIR}/../../pio_components/*/Adafruit_BusIO/Adafruit_I2CDevice.h")
foreach(BUSIO_HEADER ${BUSIO_HEADERS})
    get_filename_component(BUSIO_DIR ${BUSIO_HEADER} DIRECTORY)
    include_directories(SYSTEM "${BUSIO_DIR}")
    message(STATUS "[Clockwise] Added global include directory for Adafruit_BusIO: ${BUSIO_DIR}")
endforeach()

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "fonts"
    REQUIRES esphome wifi arduino esp_http_client Adafruit_BusIO Adafruit_GFX_Library ArduinoJson PNGdec
)
