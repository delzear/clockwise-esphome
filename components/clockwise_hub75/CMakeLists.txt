file(GLOB_RECURSE SOURCES "*.cpp")

# Recursively locate Adafruit_BusIO directory across pio_components and add to global INCLUDE_DIRECTORIES
file(GLOB_RECURSE BUSIO_HEADER "C:/esphb/*Adafruit_I2CDevice.h" "${CMAKE_BINARY_DIR}/*Adafruit_I2CDevice.h")
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
