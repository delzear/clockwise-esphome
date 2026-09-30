# SPDX-FileCopyrightText: 2025 Clockwise Integration
# SPDX-License-Identifier: MIT

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

import os

DEPENDENCIES = ["wifi"]
CODEOWNERS = ["@clockwise"]


clockwise_hub75_ns = cg.esphome_ns.namespace("clockwise_hub75")
# Use MockObj to match hub75 component's fully-qualified namespace
hub75_ns = cg.MockObj("::esphome::hub75", "::")
HUB75Display = hub75_ns.class_("HUB75Display")
ClockwiseHUB75 = clockwise_hub75_ns.class_("ClockwiseHUB75", cg.PollingComponent)

# Declare the C++ enum
ClockfaceType = clockwise_hub75_ns.enum("ClockfaceType")
PanelColorOrder = clockwise_hub75_ns.enum("PanelColorOrder")

CONF_CLOCKWISE_HUB75_ID = "clockwise_hub75_id"

CONF_CLOCKFACE_TYPE = "clockface_type"
CONF_INITIAL_BRIGHTNESS = "initial_brightness"
CONF_PANEL_COLOR_ORDER = "panel_color_order"
CONF_CANVAS_SERVER = "canvas_server"
CONF_CANVAS_FILE = "canvas_file"

CLOCKFACE_TYPES = {
    "PACMAN": ClockfaceType.PACMAN,
    "MARIO": ClockfaceType.MARIO,
    "CANVAS": ClockfaceType.CANVAS,
    "CLOCK": ClockfaceType.CLOCK,
}

PANEL_COLOR_ORDERS = {
    "RGB": PanelColorOrder.RGB,
    "RBG": PanelColorOrder.RBG,
    "GRB": PanelColorOrder.GRB,
    "GBR": PanelColorOrder.GBR,
    "BRG": PanelColorOrder.BRG,
    "BGR": PanelColorOrder.BGR,
}

CONF_HUB75_ID = "hub75_id"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ClockwiseHUB75),
    cv.Required(CONF_HUB75_ID): cv.use_id(HUB75Display),
    cv.Optional(CONF_CLOCKFACE_TYPE, default="PACMAN"): cv.enum(CLOCKFACE_TYPES, upper=True),
    cv.Optional(CONF_PANEL_COLOR_ORDER, default="RGB"): cv.enum(PANEL_COLOR_ORDERS, upper=True),
    cv.Optional(CONF_INITIAL_BRIGHTNESS, default=128): cv.int_range(min=0, max=255),
    cv.Optional(CONF_CANVAS_SERVER, default="raw.githubusercontent.com"): cv.string,
    cv.Optional(CONF_CANVAS_FILE, default="pac-man"): cv.string,
}).extend(cv.polling_component_schema("16ms"))


async def to_code(config):
    cg.add_build_flag("-DNO_SIMD")
    cg.add_library("adafruit/Adafruit BusIO", "^1.14.1")
    cg.add_library("adafruit/Adafruit GFX Library", "^1.11.5")
    cg.add_library("bblanchon/ArduinoJson", "^6.21.4")
    cg.add_library("bitbank2/PNGdec", "1.0.1")

    # Auto-patch Adafruit_GFX_Library CMakeLists.txt if downloaded into pio_components
    import glob
    for root_dir in ["C:/esphb", os.path.expanduser("~")]:
        pattern = os.path.join(root_dir, "**", "Adafruit_GFX_Library", "CMakeLists.txt")
        for cmake_file in glob.glob(pattern, recursive=True):
            try:
                with open(cmake_file, "r") as f:
                    content = f.read()
                if "Adafruit_BusIO" not in content:
                    content = content.replace("idf_component_register(", "idf_component_register(\n    REQUIRES Adafruit_BusIO\n")
                    with open(cmake_file, "w") as f:
                        f.write(content)
            except Exception:
                pass

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    hub75_var = await cg.get_variable(config[CONF_HUB75_ID])
    cg.add(var.set_hub75_display(hub75_var))

    cg.add(var.set_clockface_type(config[CONF_CLOCKFACE_TYPE]))
    cg.add(var.set_panel_color_order(config[CONF_PANEL_COLOR_ORDER]))
    cg.add(var.set_initial_brightness(config[CONF_INITIAL_BRIGHTNESS]))
    cg.add(var.set_canvas_server(config[CONF_CANVAS_SERVER]))
    cg.add(var.set_canvas_file(config[CONF_CANVAS_FILE]))