#include "pwm_driver.h"

static ssize_t pwm_chip_init(void)
{
	int rc;

	pwm_chip.device = pwm_request(pwm_chip.id, "sysfs");
	if (IS_ERR(pwm_chip.device)) {
		return PTR_ERR(pwm_chip.device);
	}

	rc = pwm_config(pwm_chip.device, pwm_chip.duty_cycle, pwm_chip.period);
	if (rc) {
		return rc;
	}

	rc = pwm_enable(pwm_chip.device);
	if (rc) {
		return rc;
	}

	printk(KERN_INFO "Ebbservo: Period: %d Duty Cycle: %d\n", pwm_get_period(pwm_chip.device), pwm_get_duty_cycle(pwm_chip.device));

	return 0;
}

static void pwm_chip_exit(void)
{
	pwm_disable(pwm_chip.device);
	pwm_free(pwm_chip.device);
}

static ssize_t pwm_device_init(void)
{
	pwm_device.major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (pwm_device.major_number < 0) {
		return pwm_device.major_number;
	}

	pwm_device.class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(pwm_device.class)) {
		unregister_chrdev(pwm_device.major_number, DEVICE_NAME);
		return PTR_ERR(pwm_device.class);
	}

	pwm_device.device = device_create(pwm_device.class, NULL, MKDEV(pwm_device.major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(pwm_device.device)) {
		class_destroy(pwm_device.class);
		unregister_chrdev(pwm_device.major_number, DEVICE_NAME);
		return PTR_ERR(pwm_device.device);
	}

	return 0;
}

static void pwm_device_exit(void)
{
	device_destroy(pwm_device.class, MKDEV(pwm_device.major_number, 0));
	class_destroy(pwm_device.class);
	unregister_chrdev(pwm_device.major_number, DEVICE_NAME);
}

static int __init pwm_driver_init(void)
{
	int rc;

	rc = pwm_chip_init();
	if (rc) {
		return rc;
	}

	rc = pwm_device_init();
	if (rc) {
		pwm_chip_exit();
		return rc;
	}

	mutex_init(&dev_mutex);

	printk(KERN_INFO "Pwm driver has been successfully configured with id: %d\n", pwm_chip.id);

	return 0;
}

static void __exit pwm_driver_exit(void)
{
	pwm_chip_exit();
	pwm_device_exit();
	mutex_destroy(&dev_mutex);
	printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	if (!mutex_trylock(&dev_mutex)) {
		printk(KERN_ALERT "ebbservo: Device in use by another process");
		return -EBUSY;
	}
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error_count, size_of_message;
	char message[256] = {0};

	if (is_read == 1) {
		return (is_read = 0);
	}

	sprintf(message, "%d", pwm_get_duty_cycle(pwm_chip.device));
	size_of_message = strlen(message) + 1;

	error_count = copy_to_user(buffer, message, size_of_message);

	if (error_count == 0) {
		printk(KERN_INFO "ebbservo: Sent %d character to the user\n", size_of_message);
		is_read = 1;
		return size_of_message;
	} else {
		printk(KERN_INFO "ebbservo: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	char message[256] = {0};
	int error_count = copy_from_user(message, buffer, len);

	if (error_count != 0) {
		printk(KERN_INFO "ebbservo: Failed to receive %d characters from the user\n", error_count);
		return -EFAULT;
	}

	printk(KERN_INFO "ebbservo: Received %zu characters from the user\n", len);
	sscanf(message, "%d", &pwm_chip.duty_cycle);
	pwm_config(pwm_chip.device, pwm_chip.duty_cycle, pwm_chip.period);

	return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&dev_mutex);
	printk(KERN_INFO "ebbservo: Device successfully closed\n");
	return 0;
}

module_init(pwm_driver_init);
module_exit(pwm_driver_exit);
