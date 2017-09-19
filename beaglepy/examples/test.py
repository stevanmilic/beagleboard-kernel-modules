import signal
import sys
from ..wrappers.interrupt import Interrupt


def main():
    i1 = Interrupt('P9_12')
    i2 = Interrupt('P9_23')

    def signal_handler(signal, frame):
        """Handler for ctrl+c interrupt - program ends"""
        print ' handler'
        i1.free()
        i2.free()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    def m1(*args):
        print 'yo1'

    def m2(*args):
        print 'yo2'

    i1.attach_interrupt(m1)
    i2.attach_interrupt(m2)

    print 'test'

    while True:
        pass


if __name__ == "__main__":
    main()
