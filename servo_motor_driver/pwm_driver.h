#ifndef _pwm_driver_h_
#define _pwm_driver_h_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/pwm.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>

#define  DEVICE_NAME "bbpwm"
#define  CLASS_NAME  "bb"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stevan Milic");
MODULE_DESCRIPTION("A pwm driver for the BBB");
MODULE_VERSION("0.1");

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

struct PwmDevice {
	int major_number;
	struct class *class;
	struct device *device;
};

struct PwmChip {
	unsigned int id;
	int duty_cycle;
	int period;
	struct pwm_device *device;
};

static struct PwmDevice pwm_device;

static struct PwmChip pwm_chip = {
	.id = 3,
	.duty_cycle = 500000,
	.period = 16666667,
	.device = NULL
};

static bool is_read = 0;

static DEFINE_MUTEX(dev_mutex);
#endif
