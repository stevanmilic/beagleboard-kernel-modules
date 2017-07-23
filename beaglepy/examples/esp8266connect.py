"""Module for using esp8266 wifi module with uart pins on the bbb"""

from time import sleep
from beaglepy.wrappers.uart import Uart
from beaglepy.wrappers.gpio import Gpio, OUTPUT, LOW, HIGH

UART_PIN = "UART1"
EXTERNAL_LED_PIN = 'P8_19'

BAUDRATE = 115200
PORT = 80


def enum(**enums):
    """Function for creatign enums"""
    return type('Enum', (), enums)


Status = enum(ERR=['ERROR', 'Fail'], OK=['OK', 'ready', 'no change', 'SEND OK'])


def main():
    """Main program where we are defining and configuring
    wifi module and external led diod. Wifi module will be
    listening on specific port and upon receiving request
    it will change led diod state"""

    uart = Uart(UART_PIN, BAUDRATE)
    setup_wifi(uart)

    external_led_gpio = Gpio(EXTERNAL_LED_PIN, OUTPUT)
    state = LOW

    while True:
        state = process_request(uart, external_led_gpio, state)
        sleep(0.3)

    external_led_gpio.free()
    uart.free()


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


def process_request(uart, led_gpio, state):
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

    if has_request:
        state = HIGH if state is LOW else LOW
        led_gpio.write(state)
        response = 'You\'ve changed led\'s state, now it\'s ' + str(state)
        send_response(uart, response, client_id)

    return state


def send_response(uart, response, client_id='0'):
    """Function for sending response to client"""

    uart.write('AT+CIPSEND=' + client_id + ',' + str(len(response)))
    sleep(0.3)
    uart.write(response)
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


if __name__ == "__main__":
    main()
