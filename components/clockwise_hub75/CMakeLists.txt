# This CMakeLists.txt is not directly used by ESPHome's ESP-IDF build system.
# ESPHome folds external component sources into the 'src' component.
# The actual build configuration is handled by __init__.py via cg.add_build_flag()
# and cg.add_library() calls.
#
# Vendored Adafruit BusIO headers are in busio_headers/ and added globally
# via cg.add_build_flag("-Isrc/esphome/components/clockwise_hub75/busio_headers")
