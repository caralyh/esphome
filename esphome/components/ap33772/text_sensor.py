import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import ICON_POWER, ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_AP33772_ID, AP33772Component, ap33772_ns

DEPENDENCIES = ["ap33772"]

CurrentPdoSensor = ap33772_ns.class_(
    "CurrentPdoSensor", text_sensor.TextSensor, cg.Component
)

CONF_CURRENT_PDO = "current_pdo"
CONF_AVAILABLE_PDOS = "available_pdos"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_AP33772_ID): cv.use_id(AP33772Component),
    cv.Optional(CONF_CURRENT_PDO): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon=ICON_POWER
    ),
    cv.Optional(CONF_AVAILABLE_PDOS): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon=ICON_POWER
    ),
}


async def to_code(config):
    pd_component = await cg.get_variable(config[CONF_AP33772_ID])
    if current_pdo_config := config.get(CONF_CURRENT_PDO):
        sens = await text_sensor.new_text_sensor(current_pdo_config)
        cg.add(pd_component.set_current_pdo_text_sensor(sens))
    if avail_pdos_config := config.get(CONF_AVAILABLE_PDOS):
        sens = await text_sensor.new_text_sensor(avail_pdos_config)
        cg.add(pd_component.set_available_pdos_text_sensor(sens))
