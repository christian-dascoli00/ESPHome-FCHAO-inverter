import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_PROBLEM

from . import FchaoInverterComponent, fchao_inverter_ns

DEPENDENCIES = ["fchao_inverter"]

CONF_FCHAO_INVERTER_ID = "fchao_inverter_id"
CONF_OVERLOAD = "overload"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_FCHAO_INVERTER_ID): cv.use_id(FchaoInverterComponent),
        cv.Optional(CONF_OVERLOAD): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_PROBLEM,
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_FCHAO_INVERTER_ID])

    if CONF_OVERLOAD in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_OVERLOAD])
        cg.add(hub.set_overload_sensor(sens))