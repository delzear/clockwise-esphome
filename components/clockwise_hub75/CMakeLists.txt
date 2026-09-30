file(GLOB_RECURSE SOURCES "*.cpp")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES esphome wifi arduino Adafruit_BusIO Adafruit_GFX_Library ArduinoJson PNGdec
)

if(TARGET __idf_arduino)
    idf_component_get_property(ARDUINO_DIR arduino COMPONENT_DIR)
    target_include_directories(${COMPONENT_LIB} PUBLIC
        "${ARDUINO_DIR}/libraries/WiFiClientSecure/src"
        "${ARDUINO_DIR}/libraries/WiFi/src"
    )
endif()

if(TARGET __idf_Adafruit_GFX_Library AND TARGET __idf_Adafruit_BusIO)
    target_link_libraries(__idf_Adafruit_GFX_Library PUBLIC __idf_Adafruit_BusIO)
    target_include_directories(__idf_Adafruit_GFX_Library PUBLIC $<TARGET_PROPERTY:__idf_Adafruit_BusIO,INTERFACE_INCLUDE_DIRECTORIES>)
endif()
