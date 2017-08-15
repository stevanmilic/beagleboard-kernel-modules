#include "pwm_driver.h"
#include "device.h"

static ssize_t pwm_chip_init(struct PwmChip *pwm_chip)
{
	int rc;

	pwm_chip->device = pwm_request(pwm_chip->id, "sysfs");
	if (IS_ERR(pwm_chip->device)) {
		return PTR_ERR(pwm_chip->device);
	}

	rc = pwm_enable(pwm_chip->device);
	if (rc) {
		return rc;
	}

	printk(KERN_INFO "BBPWM: PWM CHIP with Id: %d Period: %d Duty Cycle: %d configured\n", pwm_chip->id, pwm_get_period(pwm_chip->device), pwm_get_duty_cycle(pwm_chip->device));

	return 0;
}

static void pwm_chip_exit(struct PwmChip *pwm_chip)
{
	pwm_disable(pwm_chip->device);
	pwm_free(pwm_chip->device);
	printk(KERN_INFO "BBPWM: PWM CHIP with Id: %d freed\n", pwm_chip->id);
}

static ssize_t dev_init(void)
{
	device.major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (device.major_number < 0) {
		return device.major_number;
	}

	device.class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(device.class)) {
		unregister_chrdev(device.major_number, DEVICE_NAME);
		return PTR_ERR(device.class);
	}

	device.device = device_create(device.class, NULL, MKDEV(device.major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(device.device)) {
		class_destroy(device.class);
		unregister_chrdev(device.major_number, DEVICE_NAME);
		return PTR_ERR(device.device);
	}

	printk(KERN_INFO "BBPWM: device has been successfully configured\n");

	mutex_init(&dev_mutex);

	return 0;
}

static void dev_exit(void)
{
	device_destroy(device.class, MKDEV(device.major_number, 0));
	class_destroy(device.class);
	unregister_chrdev(device.major_number, DEVICE_NAME);
	mutex_destroy(&dev_mutex);
	printk(KERN_INFO "BBPWM: device has been unregistered\n");
}

static ssize_t __init pwm_driver_init(void)
{
	int i, rc;

	rc = dev_init();
	if (rc) {
		return rc;
	}

	for (i = 0; i < PWM_CHIPS_LEN; i++) {
		pwm_chips[i].device = NULL;
	}

	mutex_init(&dev_mutex);

	return 0;
}

static void __exit pwm_driver_exit(void)
{
	int i;
	for (i = 0; i < PWM_CHIPS_LEN; i++) {
		if (pwm_chips[i].device != NULL) {
			pwm_chip_exit(&pwm_chips[i]);
		}
	}
	dev_exit();
	mutex_destroy(&dev_mutex);
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	if (!mutex_trylock(&dev_mutex)) {
		printk(KERN_ALERT "BBPWM: Device in use by another process");
		return -EBUSY;
	}
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error_count, size_of_message;
	char message[256] = {0};

	sprintf(message, "%u %u", pwm_chips[pwm_id].duty_cycle, pwm_chips[pwm_id].period);
	size_of_message = strlen(message);

	if (*offset >= size_of_message) {
		return 0;
	}

	error_count = copy_to_user(buffer, message, size_of_message);
	if (error_count != 0) {
		printk(KERN_INFO "BBPWM: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}

	*offset += size_of_message;
	return size_of_message;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int rc, error_count;
	unsigned int id, duty_cycle, period;
	char message[256] = {0}, choice;

	error_count = copy_from_user(message, buffer, len);
	if (error_count != 0) {
		printk(KERN_INFO "BBPWM: Failed to receive %d characters from the user\n", error_count);
		return -EFAULT;
	}

	sscanf(message, "%c %d %u %u", &choice, &id, &duty_cycle, &period);

	if (id >= 8) {
		return -EINVAL;
	}

	switch (choice) {
	case 'i':
		pwm_chips[id].id = id;
		if (pwm_chips[id].device == NULL) {
			rc = pwm_chip_init(&pwm_chips[id]);
			if (rc) {
				return rc;
			}
		}
		break;
	case 'r':
		if (pwm_chips[id].device != NULL) {
			pwm_id = id;
		}
		break;
	case 'w':
		if (pwm_chips[id].device != NULL) {
			pwm_chips[id].duty_cycle = duty_cycle;
			pwm_chips[id].period = period;
			rc = pwm_config(pwm_chips[id].device, pwm_chips[id].duty_cycle, pwm_chips[id].period);
			if (rc) {
				return rc;
			}
		}
		break;
	case 'f':
		if (pwm_chips[id].device != NULL) {
			pwm_chip_exit(&pwm_chips[id]);
			pwm_chips[id].device = NULL;
		}
		break;
	}

	return len;
}

static unsigned int dev_poll(struct file *filep, struct poll_table_struct *wait)
{
	/* not implemented */
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&dev_mutex);
	return 0;
}

module_init(pwm_driver_init);
module_exit(pwm_driver_exit);
