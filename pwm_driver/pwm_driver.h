#ifndef _pwm_driver_h_
#define _pwm_driver_h_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pwm.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "bbpwm"
#define CLASS_NAME  "bb"

#define PWM_CHIPS_LEN 8

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stevan Milic");
MODULE_DESCRIPTION("A pwm driver for the BBB");
MODULE_VERSION("0.1");

struct PwmChip {
	unsigned int id;
	unsigned int duty_cycle;
	unsigned int period;
	struct pwm_device *device;
};

static ssize_t pwm_chip_init(struct PwmChip *);
static void pwm_chip_exit(struct PwmChip *);

static struct PwmChip pwm_chips[PWM_CHIPS_LEN];

static unsigned int pwm_id;

#endif
