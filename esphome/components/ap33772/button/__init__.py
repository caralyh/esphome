import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_RESTART,
    ENTITY_CATEGORY_CONFIG,
    ICON_RESTART,
)

from .. import CONF_AP33772_ID, AP33772Component, ap33772_ns

DEPENDENCIES = ["ap33772"]

ResetButton = ap33772_ns.class_("ResetButton", button.Button)
ClearButton = ap33772_ns.class_("ClearButton", button.Button)
NegoButton = ap33772_ns.class_("NegoButton", button.Button)

CONF_RESET = "reset"
CONF_CLEAR = "clear"
CONF_NEGO = "negotiate"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_AP33772_ID): cv.use_id(AP33772Component),
    cv.Optional(CONF_RESET): button.button_schema(
        ResetButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_RESTART,
    ),
    cv.Optional(CONF_CLEAR): button.button_schema(
        ClearButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_RESTART,
    ),
    cv.Optional(CONF_NEGO): button.button_schema(
        NegoButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_RESTART,
    ),
}


async def to_code(config):
    pd_component = await cg.get_variable(config[CONF_AP33772_ID])

    if reset_config := config.get(CONF_RESET):
        reset_button = await button.new_button(reset_config)
        await cg.register_parented(reset_button, config[CONF_AP33772_ID])
        cg.add(pd_component.set_reset_button(reset_button))

    if clear_config := config.get(CONF_CLEAR):
        clear_button = await button.new_button(clear_config)
        await cg.register_parented(clear_button, config[CONF_AP33772_ID])
        cg.add(pd_component.set_clear_button(clear_button))

    if nego_config := config.get(CONF_NEGO):
        nego_button = await button.new_button(nego_config)
        await cg.register_parented(nego_button, config[CONF_AP33772_ID])
        cg.add(pd_component.set_nego_button(nego_button))
