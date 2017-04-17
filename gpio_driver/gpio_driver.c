#include "gpio_driver.h"
#include "device.h"

static ssize_t __init gpio_driver_init(void)
{
	int i, rc;

	rc = dev_init();
	if (rc) {
		return rc;
	}

	for (i = 0; i < GPIOS_LEN; i++) {
		gpios[i].exported = 0;
		gpios[i].id = i;
	}

	return 0;
}

static void __exit gpio_driver_exit(void)
{
	int i;
	for (i = 0; i < GPIOS_LEN; i++) {
		if (gpios[i].exported) {
			gpio_exit(&gpios[i]);
		}
	}
	dev_exit();
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

	printk(KERN_INFO "BBGPIO: device has been successfully configured\n");

	mutex_init(&dev_mutex);

	return 0;
}

static void dev_exit(void)
{
	device_destroy(device.class, MKDEV(device.major_number, 0));
	class_destroy(device.class);
	unregister_chrdev(device.major_number, DEVICE_NAME);
	mutex_destroy(&dev_mutex);
	printk(KERN_INFO "BBGPIO: device has been unregistered\n");
}

static ssize_t gpio_init(struct Gpio *gpio)
{
	int rc;

	if (!gpio_is_valid(gpio->id)) {
		return -ENODEV;
	}

	rc  = gpio_request(gpio->id, "sysfs");
	if (rc) {
		return rc;
	}

	if (!gpio->direction) {
		rc = gpio_direction_input(gpio->id);
	} else {
		rc = gpio_direction_output(gpio->id, gpio->value);
	}
	if (rc) {
		return rc;
	}

	rc = gpio_export(gpio->id, 0);
	if (rc) {
		return rc;
	}

	gpio->exported = 1;

	printk(KERN_INFO "BBGPIO: GPIO with Id: %u Value: %u Direction: %u configured\n", gpio->id, gpio->value, gpio->direction);

	return 0;
}

static void gpio_exit(struct Gpio *gpio)
{
	gpio_set_value(gpio->id, 0);
	gpio_unexport(gpio->id);
	gpio_free(gpio->id);
	gpio->exported = 0;
	printk(KERN_INFO "BBGPIO: GPIO with Id: %u freed\n", gpio->id);
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	if (!mutex_trylock(&dev_mutex)) {
		printk(KERN_ALERT "BBGPIO: Device in use by another process");
		return -EBUSY;
	}
	printk(KERN_INFO "BBGPIO: Device successfully opened\n");
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error_count, size_of_message, i;
	char message[256] = {0};

	if (is_read == 1) {
		return (is_read = 0);
	}

	for (i = 0; i < GPIOS_LEN; i++) {
		if (gpios[i].exported) {
			sprintf(message + strlen(message), "%u %u %u\n", gpios[i].id, gpios[i].value, gpios[i].direction);
		}
	}
	size_of_message = strlen(message) + 1;

	error_count = copy_to_user(buffer, message, size_of_message);

	if (error_count == 0) {
		printk(KERN_INFO "BBGPIO: Sent %d character to the user\n", size_of_message);
		is_read = 1;
		return size_of_message;
	} else {
		printk(KERN_INFO "BBGPIO: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int rc, error_count;
	char message[256] = {0}, choice;
	unsigned int id, value, direction;

	error_count = copy_from_user(message, buffer, len);
	if (error_count != 0) {
		printk(KERN_INFO "BBGPIO: Failed to receive %d characters from the user\n", error_count);
		return -EFAULT;
	}

	sscanf(message, "%c %d %u %u", &choice, &id, &value, &direction);

	if (id >= 65) {
		return -EINVAL;
	}

	switch (choice) {
	case 's':
		gpios[id].id = id;
		gpios[id].value = value;
		gpios[id].direction = direction;
		if (!gpios[id].exported) {
			rc = gpio_init(&gpios[id]);
		} else {
			gpio_set_value(gpios[id].id, gpios[id].value);
		}
		if (rc) {
			return rc;
		}
		break;
	case 'd':
		if (gpios[id].exported) {
			gpio_exit(&gpios[id]);
			gpios[id].exported = 0;
		}
		break;
	}

	return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&dev_mutex);
	printk(KERN_INFO "BBGPIO: Device successfully closed\n");
	return 0;
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);
