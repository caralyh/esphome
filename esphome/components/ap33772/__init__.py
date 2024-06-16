import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor, binary_sensor
from esphome import pins, automation
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_INTERRUPT,
    CONF_INTERRUPT_PIN,
    CONF_TEMPERATURE,
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_MAX_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    UNIT_CELSIUS,
    STATE_CLASS_MEASUREMENT,
    ICON_POWER,
)


DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor", "preferences", "binary_sensor"]

CONF_AP33772_ID = "ap33772_id"
CONF_INTERRUPT_READY = "ready"
CONF_INTERRUPT_SUCCESS = "success"
CONF_INTERRUPT_NEWPDO = "new_pdo"
CONF_INTERRUPT_OVP = "over_voltage"
CONF_INTERRUPT_OCP = "over_current"
CONF_INTERRUPT_OTP = "over_temperature"
CONF_INTERRUPT_DR = "derating"
CONF_NEGOTIATION_SUCCESS = "negotiation_success"
CONF_HAS_PPS = "has_pps"

UNIT_MILLIVOLT = "mV"
UNIT_MILLIAMPERE = "mA"

ap33772_ns = cg.esphome_ns.namespace("ap33772")
AP33772Component = ap33772_ns.class_(
    "AP33772Component", cg.PollingComponent, i2c.I2CDevice
)

AP33772RequestVoltageAction = ap33772_ns.class_(
    "RequestVoltageAction", automation.Action
)
AP33772RequestCurrentAction = ap33772_ns.class_(
    "RequestCurrentAction", automation.Action
)
AP33772RequestMaxCurrentAction = ap33772_ns.class_(
    "RequestMaxCurrentAction", automation.Action
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AP33772Component),
            cv.Optional(CONF_INTERRUPT): cv.Schema(
                {
                    cv.Optional(
                        CONF_INTERRUPT_PIN
                    ): pins.internal_gpio_input_pin_schema,
                    cv.Optional(CONF_INTERRUPT_READY, default=False): cv.boolean,  # type: ignore
                    cv.Optional(CONF_INTERRUPT_SUCCESS, default=False): cv.boolean,  # type: ignore
                    cv.Optional(CONF_INTERRUPT_NEWPDO, default=False): cv.boolean,  # type: ignore
                    cv.Optional(CONF_INTERRUPT_OVP, default=False): cv.boolean,  # type: ignore
                    cv.Optional(CONF_INTERRUPT_OCP, default=False): cv.boolean,  # type: ignore
                    cv.Optional(CONF_INTERRUPT_OTP, default=False): cv.boolean,  # type: ignore
                    cv.Optional(CONF_INTERRUPT_DR, default=False): cv.boolean,  # type: ignore
                }
            ),
            cv.Optional(CONF_TEMPERATURE, default={CONF_NAME: "Temperature", CONF_ID: None}): sensor.sensor_schema(  # type: ignore
                unit_of_measurement=UNIT_CELSIUS,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VOLTAGE, default={CONF_NAME: "Voltage", CONF_ID: None}): sensor.sensor_schema(  # type: ignore
                unit_of_measurement=UNIT_MILLIVOLT,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT, default={CONF_NAME: "Current", CONF_ID: None}): sensor.sensor_schema(  # type: ignore
                unit_of_measurement=UNIT_MILLIAMPERE,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_NEGOTIATION_SUCCESS, default={CONF_NAME: "Negotiation Success"}): binary_sensor.binary_sensor_schema(icon=ICON_POWER),  # type: ignore
            cv.Optional(CONF_HAS_PPS, default={CONF_NAME: "PPS"}): binary_sensor.binary_sensor_schema(icon=ICON_POWER),  # type: ignore
        }
    )
    .extend(i2c.i2c_device_schema(0x51))
    .extend(cv.polling_component_schema("30s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if interrupt := config.get(CONF_INTERRUPT):
        if interrupt_pin := interrupt.get(CONF_INTERRUPT_PIN):
            cg.add(var.set_interrupt_pin(await cg.gpio_pin_expression(interrupt_pin)))
            mask_ready = interrupt.get(CONF_INTERRUPT_READY, False)
            mask_success = interrupt.get(CONF_INTERRUPT_SUCCESS, False)
            mask_newpdo = interrupt.get(CONF_INTERRUPT_NEWPDO, False)
            mask_ovp = interrupt.get(CONF_INTERRUPT_OVP, False)
            mask_ocp = interrupt.get(CONF_INTERRUPT_OCP, False)
            mask_otp = interrupt.get(CONF_INTERRUPT_OTP, False)
            mask_dr = interrupt.get(CONF_INTERRUPT_DR, False)
            cg.add(
                var.configure_mask(
                    mask_ready,
                    mask_success,
                    mask_newpdo,
                    mask_ovp,
                    mask_ocp,
                    mask_otp,
                    mask_dr,
                )
            )

    if config_temp := config.get(CONF_TEMPERATURE):
        temp_sensor = await sensor.new_sensor(config_temp)
        cg.add(var.set_temperature_sensor(temp_sensor))
    if config_volt := config.get(CONF_VOLTAGE):
        volt_sensor = await sensor.new_sensor(config_volt)
        cg.add(var.set_voltage_sensor(volt_sensor))
    if config_current := config.get(CONF_CURRENT):
        current_sensor = await sensor.new_sensor(config_current)
        cg.add(var.set_current_sensor(current_sensor))

    if config_nego_success := config.get(CONF_NEGOTIATION_SUCCESS):
        nego_binary = await binary_sensor.new_binary_sensor(config_nego_success)
        cg.add(var.set_negotiation_success_binary_sensor(nego_binary))
    if config_has_pps := config.get(CONF_HAS_PPS):
        pps_binary = await binary_sensor.new_binary_sensor(config_has_pps)
        cg.add(var.set_has_pps_binary_sensor(pps_binary))


@automation.register_action(
    "ap33772.request_voltage",
    AP33772RequestVoltageAction,
    cv.Schema(
        {
            cv.GenerateID(CONF_AP33772_ID): cv.use_id(AP33772Component),
            cv.Required(CONF_VOLTAGE): cv.templatable(cv.voltage),
        }
    ),
)
async def request_voltage_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    template_ = await cg.templatable(config[CONF_VOLTAGE], args, float)
    cg.add(var.request_voltage(template_))
    return var


@automation.register_action(
    "ap33772.request_current",
    AP33772RequestCurrentAction,
    cv.Schema(
        {
            cv.GenerateID(CONF_AP33772_ID): cv.use_id(AP33772Component),
            cv.Required(CONF_CURRENT): cv.templatable(cv.current),
        }
    ),
)
async def request_current_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    template_ = await cg.templatable(config[CONF_CURRENT], args, float)
    cg.add(var.request_current(template_))
    return var


@automation.register_action(
    "ap33772.request_max_current",
    AP33772RequestMaxCurrentAction,
    cv.Schema(
        {
            cv.GenerateID(CONF_AP33772_ID): cv.use_id(AP33772Component),
            cv.Required(CONF_MAX_CURRENT): cv.templatable(cv.current),
        }
    ),
)
async def request_max_current_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    template_ = await cg.templatable(config[CONF_MAX_CURRENT], args, float)
    cg.add(var.request_max_current(template_))
    return var
