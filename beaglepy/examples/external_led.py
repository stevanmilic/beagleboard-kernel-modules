""""Module for using external led diode with gpio pins on the bbb

Args:
    param1 (str): external led gpio pin
    param2 (str): blinking interval
    param3 (str): timeout interval
"""

from sys import argv
from signal import signal, alarm, SIGALRM
from time import sleep
from beaglepy.wrappers.gpio import Gpio, OUTPUT, LOW, HIGH

EXTERNAL_LED_PIN = 'P8_19'
BLINKING_INTERVAL = 1  # second
TIMEOUT_INTERVAL = 10  # seconds


def main():
    """Main function for init led gpio and making a timeout function"""
    external_led_pin = argv[1] if len(argv) >= 2 else EXTERNAL_LED_PIN
    external_led_gpio = Gpio(external_led_pin, OUTPUT)

    timeout_interval = int(argv[3]) if len(argv) >= 4 else TIMEOUT_INTERVAL
    signal(SIGALRM, handler)
    alarm(timeout_interval)

    try:
        blinking_led(external_led_gpio)
    except Exception, exception:
        external_led_gpio.free()


def blinking_led(led_gpio):
    """Function for turning the led on and off"""

    blinking_interval = float(argv[2]) if len(argv) >= 3 else BLINKING_INTERVAL
    state = LOW

    while True:
        state = HIGH if state is LOW else LOW
        led_gpio.write(state)
        sleep(blinking_interval)


def handler(signum, frame):
    """Handler function for signal from the system"""
    print "Blinking is done"
    raise Exception()


if __name__ == "__main__":
    main()
