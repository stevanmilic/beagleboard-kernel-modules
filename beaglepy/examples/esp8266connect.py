"""
Module for using esp8266 wifi module with uart pins on the bbb

Args:
    param1 (str): esp8266 uart pin
    param2 (str): ssid to connect to
    param3 (str): password for given ssid
    param4 (str): esp8266 port listener
"""

from time import sleep
from re import search
import signal
import sys
from ..wrappers.uart import Uart
from ..wrappers.gpio import Gpio, OUTPUT, LOW, HIGH
from ..wrappers.interrupt import Interrupt

UART_PIN = "UART1"
EXTERNAL_LED_PIN = 'P8_19'
INTERRUPT_PIN = 'P9_12'

BAUDRATE = 115200
PORT = 5000


def enum(**enums):
    """Function for creatign enums"""
    return type('Enum', (), enums)


Status = enum(ERR=['ERROR', 'Fail'],
              OK=['OK', 'ready', 'no change', 'SEND OK', 'CONNECT'])


def main():
    """Main program where we are defining and configuring
    wifi module, external led diod and interrupt gpio.
    Wifi module will be listening on specific port and
    upon receiving request it will change led diod state
    """

    uart_pin = sys.argv[1] if len(sys.argv) >= 2 else UART_PIN

    uart = Uart(uart_pin, BAUDRATE)
    setup_wifi(uart)

    # external_led_gpio = Gpio(EXTERNAL_LED_PIN, OUTPUT)
    external_led_gpio = None

    # interrupt_gpio = Interrupt(INTERRUPT_PIN)
    interrupt_gpio = None

    def signal_handler(signum, frame):
        """Handler for ctrl+c interrupt - program ends"""
        external_led_gpio.free()
        uart.free()
        interrupt_gpio.free()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    while True:
        (external_led_gpio, interrupt_gpio) = process_request(
            uart, interrupt_gpio, external_led_gpio)
        sleep(0.3)


def setup_wifi(uart):
    """Function for sending commands to wifi module,
    and thus configuring it"""

    send_cmd(uart, 'AT')  # Test if AT system works
    send_cmd(uart, 'AT+CWMODE=1')  # Client mode
    send_cmd(uart, 'AT+RST')  # Reset the module

    # send_cmd(uart, 'AT+CWLAP', 10)  # List available AP

    ssid = sys.argv[2] if len(sys.argv) >= 3 else raw_input(
        'Enter ssid of network you want to connect:')
    password = sys.argv[3] if len(sys.argv) >= 4 else raw_input(
        'Enter password for specific network:')
    con_command = 'AT+CWJAP=\"' + ssid + '\",\"' + password + '\"'

    send_cmd(uart, con_command, 5)  # Connect to SSID with supplied password
    send_cmd(uart, 'AT+CIFSR', 5)  # Get local IP address

    send_cmd(uart, 'AT+CIPMUX=1')  # Enable multiplex mode
    port = sys.argv[4] if len(sys.argv) >= 5 else str(PORT)
    send_cmd(uart, 'AT+CIPSERVER=1,' + port)  # Server mode


def send_cmd(uart, command, wait_time=1, retry=5):
    """Function for sending a command to wifi module"""

    time = 0
    status = ''

    print 'Sending command: ' + command

    for i in range(retry):
        uart.write(command)
        status = uart.read()
        sleep(0.2)
        while time < wait_time or 'busy' in status:
            while uart.is_busy():
                status = uart.read()
                print status
                time = 0
            if status in Status.OK or status == Status.ERR:
                break
            sleep(1)
            time += 1
        sleep(1)
        if status in Status.OK:
            break


def process_request(uart, interrupt_gpio, led_gpio):
    """Function for processing request from client"""

    has_request = False

    while uart.is_busy():
        request = uart.read()
        print request
        ipd_str = '+IPD,'
        if ipd_str in request:
            has_request = True
            data = search(':(.*)0,CLOSED', request).group(1)
            (pin_type, pin_id, pin_value) = [
                t(s) for t, s in zip((str, str, int), data.split())]

    if has_request:
        if pin_type == 'l':
            led_gpio = Gpio(pin_id, OUTPUT) if led_gpio is None else led_gpio
            led_gpio.write(pin_value)
        elif pin_type == 'i' and led_gpio is not None:
            interrupt_gpio = Interrupt(pin_id)
            interrupt_gpio.attach_interrupt(external_led_handler, led_gpio)
            print 'Interrupt attached on pin ' + pin_id

    return (led_gpio, interrupt_gpio)


def send_message(uart, message, client_id='0'):
    """Function for sending response to client"""

    uart.write('AT+CIPSEND=' + client_id + ',' + str(len(message)))
    sleep(0.3)
    uart.write(message)
    sleep(0.3)

    res_sent = False

    for i in range(100):
        while uart.is_busy():
            status = uart.read()
            print status
            if status == Status.OK[3]:
                res_sent = True
        if res_sent:
            break
        sleep(0.1)

    sleep(0.3)
    uart.write('AT+CIPCLOSE=' + client_id)
    sleep(0.3)


def external_led_handler(*args):
    """Method which is triggered when event is detected on interrupt gpio pin
    """
    led_gpio = args[0]

    state = HIGH if led_gpio.state is LOW else LOW
    led_gpio.write(state)

    print 'Interrupt detected... Led value is ' + str(state) + ' now'


if __name__ == "__main__":
    main()
