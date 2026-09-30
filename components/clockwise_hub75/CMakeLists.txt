file(GLOB_RECURSE SOURCES "*.cpp")

# Glob all Adafruit_GFX_Library CMakeLists.txt files in pio_components and inject REQUIRES Adafruit_BusIO
file(GLOB_RECURSE GFX_CMAKES "${CMAKE_BINARY_DIR}/../pio_components/*/*/*/CMakeLists.txt" "${CMAKE_BINARY_DIR}/../../pio_components/*/*/*/CMakeLists.txt" "C:/esphb/*/*/*/*/CMakeLists.txt")
foreach(GFX_CMAKE ${GFX_CMAKES})
    if(GFX_CMAKE MATCHES "Adafruit_GFX_Library/CMakeLists.txt")
        file(READ ${GFX_CMAKE} GFX_CONTENT)
        if(NOT GFX_CONTENT MATCHES "Adafruit_BusIO")
            string(REPLACE "idf_component_register(" "idf_component_register(\n    REQUIRES Adafruit_BusIO\n" GFX_NEW "${GFX_CONTENT}")
            file(WRITE ${GFX_CMAKE} "${GFX_NEW}")
            message(STATUS "[Clockwise] Successfully patched Adafruit_GFX_Library CMakeLists.txt at ${GFX_CMAKE}")
        endif()
    endif()
endforeach()

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES esphome wifi arduino esp_http_client Adafruit_BusIO Adafruit_GFX_Library ArduinoJson PNGdec
)
