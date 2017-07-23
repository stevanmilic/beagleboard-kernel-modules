# beaglebone-io
School(open source) project for using beagleboard pins by creating an interface to the gpio, pwm and uart pins.

# drivers - kernel modules written in c
In the folder drivers are implementations of using gpio and pwm pins on bbb. 
Using kernel modules and representing them as a devices, you can easily change the states of pwm and gpio pins.

# beaglepy - python interface for bbb 
Python implementation of using the above mentioned drivers. 
In the package wrappers are modules for using pwm, gpio and uart pins. 
In the package examples there are several programs which are using electrical components and sensors.
