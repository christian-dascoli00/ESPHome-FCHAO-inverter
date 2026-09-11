import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import uart
from esphome.const import CONF_ID, CONF_FLOW_CONTROL_PIN
from esphome.cpp_helpers import gpio_pin_expression

CODEOWNERS = ["@christian-dascoli00"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

fchao_inverter_ns = cg.esphome_ns.namespace("fchao_inverter")
FchaoInverterComponent = fchao_inverter_ns.class_(
    "FchaoInverterComponent", cg.PollingComponent, uart.UARTDevice
)

CONF_RX_TIMEOUT = "rx_timeout"
CONF_DATA_TIMEOUT = "data_timeout"
CONF_SEND_REQUEST = "send_request"

CONFIG_SCHEMA = cv.All(
    cv.require_esphome_version(2025, 7, 0),
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(FchaoInverterComponent),
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
            cv.Optional(
                CONF_RX_TIMEOUT, default="200ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_DATA_TIMEOUT, default="5s"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_SEND_REQUEST, default=True): cv.boolean,
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(uart.UART_DEVICE_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_rx_timeout(config[CONF_RX_TIMEOUT]))
    cg.add(var.set_data_timeout(config[CONF_DATA_TIMEOUT]))
    cg.add(var.set_send_request(config[CONF_SEND_REQUEST]))

    if CONF_FLOW_CONTROL_PIN in config:
        pin = await gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))