file(GLOB_RECURSE SOURCES "*.cpp")

# Recursively find all CMakeLists.txt files in build directories and patch Adafruit_GFX_Library CMakeLists.txt
file(GLOB_RECURSE ALL_CMAKES 
    "C:/esphb/CMakeLists.txt" 
    "C:/esphb/*"
    "${CMAKE_BINARY_DIR}/../CMakeLists.txt"
    "${CMAKE_BINARY_DIR}/../../CMakeLists.txt"
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

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES esphome wifi arduino esp_http_client Adafruit_BusIO Adafruit_GFX_Library ArduinoJson PNGdec
)
