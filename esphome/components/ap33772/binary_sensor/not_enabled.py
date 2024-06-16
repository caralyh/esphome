# import esphome.codegen as cg
# import esphome.config_validation as cv
# from esphome.components import binary_sensor
# from esphome.const import (
#     ENTITY_CATEGORY_NONE,
#     DEVICE_CLASS_RUNNING,
#     DEVICE_CLASS_EMPTY,
#     CONF_ID,
#     ICON_POWER
# )
# from .. import CONF_AP33772_ID, AP33772Component

# CONF_NEGOTIATION_SUCCESS = "negotiation_success"
# CONF_HAS_PPS = "has_pps"


# DEPENDENCIES = ["ap33772"]

# CONFIG_SCHEMA = cv.Schema({
#     cv.GenerateID(CONF_AP33772_ID): cv.use_id(AP33772Component),
#     cv.Optional(CONF_NEGOTIATION_SUCCESS): binary_sensor.binary_sensor_schema(
#         icon=ICON_POWER
#     ),
#     cv.Optional(CONF_HAS_PPS): binary_sensor.binary_sensor_schema(
#         icon=ICON_POWER
#     )
# })

# async def to_code(config):
#     pd_component = await cg.get_variable(config[CONF_AP33772_ID])
#     if nego_success_config := config.get(CONF_NEGOTIATION_SUCCESS):
#         nego_success = await binary_sensor.new_binary_sensor(nego_success_config)
#         cg.add(pd_component.set_negotiation_success_binary_sensor(nego_success))
#     if has_pps_config := config.get(CONF_HAS_PPS):
#         has_pps = await binary_sensor.new_binary_sensor(has_pps_config)
#         cg.add(pd_component.set_has_pps_binary_sensor(has_pps))
