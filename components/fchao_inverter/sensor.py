import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_POWER,
    CONF_TEMPERATURE,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_VOLT,
    UNIT_WATT,
)

from . import FchaoInverterComponent, fchao_inverter_ns

DEPENDENCIES = ["fchao_inverter"]

CONF_FCHAO_INVERTER_ID = "fchao_inverter_id"
CONF_AC_VOLTAGE = "ac_voltage"
CONF_BATTERY_VOLTAGE = "battery_voltage"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_FCHAO_INVERTER_ID): cv.use_id(FchaoInverterComponent),
        cv.Optional(CONF_AC_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=0,
        ),
        cv.Optional(CONF_POWER): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=0,
        ),
        cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=1,
        ),
        cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=0,
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_FCHAO_INVERTER_ID])

    if CONF_AC_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_AC_VOLTAGE])
        cg.add(hub.set_voltage_sensor(sens))

    if CONF_POWER in config:
        sens = await sensor.new_sensor(config[CONF_POWER])
        cg.add(hub.set_power_sensor(sens))

    if CONF_BATTERY_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_VOLTAGE])
        cg.add(hub.set_battery_voltage_sensor(sens))

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(hub.set_temperature_sensor(sens))