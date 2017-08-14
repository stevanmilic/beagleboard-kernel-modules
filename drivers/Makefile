obj-m := gpio_driver.o pwm_driver/pwm_driver.o
gpio_driver-objs = gpio_driver/gpio_driver.o gpio_driver/interrupt.o

ccflags-y += -I$(src)/device

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
loadpwm:
	insmod pwm_driver/pwm_driver.ko
unloadpwm:
	rmmod pwm_driver/pwm_driver.ko
loadgpio:
	insmod gpio_driver.ko
unloadgpio:
	rmmod gpio_driver.ko
