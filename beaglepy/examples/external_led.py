""""Module for using external led diode with gpio pins on the bbb"""

from signal import signal, alarm, SIGALRM
from time import sleep
from beaglepy.wrappers.gpio import Gpio, OUTPUT, LOW, HIGH

EXTERNAL_LED_PIN = 'P8_19'
BLINKING_INTERVAL = 1  # second
TIMEOUT_INTERVAL = 10  # seconds


def main():
    """Main function for init led gpio and making a timeout function"""
    external_led_gpio = Gpio(EXTERNAL_LED_PIN, OUTPUT)

    signal(SIGALRM, handler)
    alarm(TIMEOUT_INTERVAL)

    try:
        blinking_led(external_led_gpio)
    except Exception, exception:
        external_led_gpio.free()


def blinking_led(led_gpio):
    """Function for turning the led on and off"""
    state = LOW
    while True:
        state = HIGH if state is LOW else LOW
        led_gpio.write(state)
        sleep(BLINKING_INTERVAL)


def handler(signum, frame):
    """Handler function for signal from the system"""
    print "Blinking is done"
    raise Exception()


if __name__ == "__main__":
    main()
