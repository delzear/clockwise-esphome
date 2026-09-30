file(GLOB_RECURSE SOURCES "*.cpp")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES esphome wifi arduino esp_http_client Adafruit_BusIO Adafruit_GFX_Library ArduinoJson PNGdec
)

# After all components are registered, inject Adafruit_BusIO's include directory
# into the Adafruit_GFX_Library target so it can find Adafruit_I2CDevice.h.
# This works because our REQUIRES ensures both targets exist by this point.
idf_component_get_property(busio_dir Adafruit_BusIO COMPONENT_DIR)
if(busio_dir)
    message(STATUS "[Clockwise] Injecting Adafruit_BusIO include dir: ${busio_dir}")
    target_include_directories(__idf_Adafruit_GFX_Library PUBLIC "${busio_dir}")
endif()
