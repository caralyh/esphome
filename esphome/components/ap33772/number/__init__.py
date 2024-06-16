import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_STEP,
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_MAX_CURRENT,
    CONF_RESTORE_VALUE,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    ENTITY_CATEGORY_CONFIG,
    ICON_POWER,
    ICON_CURRENT_AC,
)

from .. import CONF_AP33772_ID, AP33772Component, ap33772_ns

DEPENDENCIES = ["ap33772"]

VoltageNumber = ap33772_ns.class_("VoltageNumber", cg.Component, number.Number)
CurrentNumber = ap33772_ns.class_("CurrentNumber", cg.Component, number.Number)
MaxCurrentNumber = ap33772_ns.class_("MaxCurrentNumber", cg.Component, number.Number)

UNIT_MILLIVOLT = "mV"
UNIT_MILLIAMPERE = "mA"

RESTORE_CONFIG_SCHEMA = cv.Schema(
    {cv.Optional(CONF_RESTORE_VALUE, default=True): cv.boolean}  # type: ignore
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_AP33772_ID): cv.use_id(AP33772Component),
        cv.Optional(CONF_VOLTAGE): number.number_schema(
            VoltageNumber,
            unit_of_measurement=UNIT_MILLIVOLT,
            device_class=DEVICE_CLASS_VOLTAGE,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_POWER,
        ).extend(RESTORE_CONFIG_SCHEMA),
        cv.Optional(CONF_CURRENT): number.number_schema(
            CurrentNumber,
            unit_of_measurement=UNIT_MILLIAMPERE,
            device_class=DEVICE_CLASS_CURRENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_CURRENT_AC,
        ).extend(RESTORE_CONFIG_SCHEMA),
        cv.Optional(CONF_MAX_CURRENT): number.number_schema(
            MaxCurrentNumber,
            unit_of_measurement=UNIT_MILLIAMPERE,
            device_class=DEVICE_CLASS_CURRENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon=ICON_CURRENT_AC,
        ).extend(RESTORE_CONFIG_SCHEMA),
    }
)


async def to_code(config):
    pd_component = await cg.get_variable(config[CONF_AP33772_ID])

    for key in [CONF_VOLTAGE, CONF_CURRENT, CONF_MAX_CURRENT]:
        if num_config := config.get(key):
            var = cg.new_Pvariable(num_config.get(CONF_ID))
            await cg.register_component(var, num_config)
            cg.add(var.set_restore(num_config[CONF_RESTORE_VALUE]))
            default_max_value = 28000 if key is CONF_VOLTAGE else 6400
            await number.register_number(
                var,
                num_config,
                min_value=num_config.get(CONF_MIN_VALUE, 0),
                max_value=num_config.get(CONF_MAX_VALUE, default_max_value),
                step=num_config.get(CONF_STEP, 10),
            )
            await cg.register_parented(var, config[CONF_AP33772_ID])
            cg.add(pd_component.set_number_by_name(key, var))

    # if voltage_config := config.get(CONF_VOLTAGE):
    #     p_volt = cg.new_Pvariable(voltage_config.get(CONF_ID))
    #     await cg.register_component(p_volt, voltage_config)
    #     await number.register_number(
    #         p_volt,
    #         voltage_config,
    #         min_value=voltage_config[CONF_MIN_VALUE],
    #         max_value=voltage_config[CONF_MAX_VALUE],
    #         step=voltage_config[CONF_STEP]
    #     )
    #     await cg.register_parented(p_volt, config[CONF_AP33772_ID])
    #     cg.add(pd_component.set_voltage_number(p_volt))
    # if current_config := config.get(CONF_CURRENT):
    #     current_n = await number.new_number(
    #         current_config, min_value=0, max_value=20000, step=10
    #     )
    #     await cg.register_parented(current_n, config[CONF_AP33772_ID])
    #     cg.add(pd_component.set_current_number(current_n))
    # if max_current_config := config.get(CONF_MAX_CURRENT):
    #     max_current_n = await number.new_number(
    #         max_current_config, min_value=0, max_value=20000, step=10
    #     )
    #     await cg.register_parented(max_current_n, config[CONF_AP33772_ID])
    #     cg.add(pd_component.set_max_current_number(current_n))
