file(GLOB_RECURSE SOURCES "*.cpp")

# Auto-patch Adafruit_GFX_Library CMakeLists.txt if missing REQUIRES Adafruit_BusIO
file(GLOB_RECURSE GFX_CMAKES "${CMAKE_BINARY_DIR}/../pio_components/*/Adafruit_GFX_Library/CMakeLists.txt" "${CMAKE_BINARY_DIR}/../../pio_components/*/Adafruit_GFX_Library/CMakeLists.txt")
foreach(GFX_CMAKE ${GFX_CMAKES})
    if(EXISTS ${GFX_CMAKE})
        file(READ ${GFX_CMAKE} CMAKE_CONTENT)
        if(NOT CMAKE_CONTENT MATCHES "Adafruit_BusIO")
            string(REPLACE "idf_component_register(" "idf_component_register(\n    REQUIRES Adafruit_BusIO\n" NEW_CONTENT "${CMAKE_CONTENT}")
            file(WRITE ${GFX_CMAKE} "${NEW_CONTENT}")
            message(STATUS "Patched Adafruit_GFX_Library CMakeLists.txt: injected REQUIRES Adafruit_BusIO")
        endif()
    endif()
endforeach()

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES esphome wifi arduino esp_http_client Adafruit_BusIO Adafruit_GFX_Library ArduinoJson PNGdec
)
