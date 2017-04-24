"""Module for using sg90 servo motor with pwm pins on the bbb"""

from time import sleep
from beaglepy.wrappers.pwm import Pwm

SERVO_PWM_PIN = 'P9_14'
FREQUENCY = 60  # Hz

DUTY_MIN = 0.03  # %
DUTY_MAX = 0.115  # %
INCREMENT = 0.1


def main():
    """Main function for using pwm pin and thus controlling servo motor"""

    servo_pwm = Pwm(SERVO_PWM_PIN)

    position = 0
    while position <= 1:
        duty_percent = (position*DUTY_MAX) + DUTY_MIN
        servo_pwm.write(duty_percent, FREQUENCY)
        sleep(0.2)
        position += INCREMENT

    servo_pwm.free()


if __name__ == "__main__":
    main()
