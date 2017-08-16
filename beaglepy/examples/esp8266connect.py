"""Module for using esp8266 wifi module with uart pins on the bbb"""

from time import sleep
import signal
import sys
from ..wrappers.uart import Uart
from ..wrappers.gpio import Gpio, OUTPUT, LOW, HIGH
from ..wrappers.interrupt import Interrupt

UART_PIN = "UART1"
EXTERNAL_LED_PIN = 'P8_19'
INTERRUPT_PIN = 'P9_12'

BAUDRATE = 115200
PORT = 80


def enum(**enums):
    """Function for creatign enums"""
    return type('Enum', (), enums)


Status = enum(ERR=['ERROR', 'Fail'], OK=[
    'OK', 'ready', 'no change', 'SEND OK'])


def main():
    """Main program where we are defining and configuring
    wifi module and external led diod. Wifi module will be
    listening on specific port and upon receiving request
    it will change led diod state"""

    uart = Uart(UART_PIN, BAUDRATE)
    setup_wifi(uart)

    external_led_gpio = Gpio(EXTERNAL_LED_PIN, OUTPUT)

    interrupt_gpio = Interrupt(INTERRUPT_PIN)

    def signal_handler(signal, frame):
        """Handler for ctrl+c interrupt - program ends"""
        external_led_gpio.free()
        uart.free()
        interrupt_gpio.free()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    while True:
        process_request(uart, interrupt_gpio, external_led_gpio)
        sleep(0.3)


def setup_wifi(uart):
    """Function for sending commands to wifi module,
    and thus configuring it"""

    send_cmd(uart, 'AT')  # Test if AT system works
    send_cmd(uart, 'AT+CWMODE=1')  # Client mode
    send_cmd(uart, 'AT+RST')  # Reset the module

    send_cmd(uart, 'AT+CWLAP', 10)  # List available AP

    ssid = raw_input('Enter ssid of network you want to connect:')
    password = raw_input('Enter password for specific network:')
    con_command = 'AT+CWJAP=\"' + ssid + '\",\"' + password + '\"'

    send_cmd(uart, con_command, 5)  # Connect to SSID with supplied password
    send_cmd(uart, 'AT+CIFSR', 5)  # Get local IP address

    send_cmd(uart, 'AT+CIPMUX=1')  # Enable multiplex mode
    send_cmd(uart, 'AT+CIPSERVER=1,' + str(PORT))  # Server mode


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
    client_id = '0'

    while uart.is_busy():
        request = uart.read()
        print request
        ipd_str = '+IPD,'
        if ipd_str in request:
            has_request = True
            client_id = request[request.find(ipd_str) + len(ipd_str)]

    if has_request and not interrupt_gpio.attached:
        print 'Interrupt attached...'
        interrupt_gpio.attach_interrupt(
            external_led_handler, uart, led_gpio, client_id)


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
    uart = args[0]
    led_gpio = args[1]
    client_id = args[2]

    state = HIGH if led_gpio.state is LOW else LOW
    led_gpio.write(state)

    message = "HTTP/1.1 200 OK\n" + "Content-Type: text/html\n" + \
        "\n" + '<html><body>You\'ve changed led\'s state, now it\'s ' + \
        str(state) + '</body></html>\n'
    send_message(uart, message, client_id)


if __name__ == "__main__":
    main()
