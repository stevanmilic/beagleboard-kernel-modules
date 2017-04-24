# beaglebone-io
School(open source) project for using beagleboard pins with drivers(software) and sensors(hardware).

# drivers - kernel modules written in c
In the folder drivers are trouved implementations of using gpio and pwm pins on bbb. Using kernel modules, and representing them as a devices, you can easily change the states of pwm and gpio pins.

# beaglepy - python interface for bbb 
Python implementation of using the above mentioned drivers, in the package wrappers are modules for using pwm and gpio pins. In the package examples there are several programs which are using electrical components and sensors as a clear introduction how the modules are supposed to be used.
