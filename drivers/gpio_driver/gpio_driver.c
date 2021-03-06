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
		gpios[i].value = 0;
		gpios[i].irq = 0;
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

	gpio_unexport(gpio->id);
	gpio_free(gpio->id);

	if (!gpio_is_valid(gpio->id)) {
		return -ENODEV;
	}

	rc  = gpio_request(gpio->id, "sysfs");
	if (rc) {
		return rc;
	}

	if (!gpio->direction) {
		rc = gpio_direction_input(gpio->id);
		/* gpio_set_debounce(gpio->id, 200); */
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

	if (!gpio->direction && gpio->mask) {

		int irq = gpio_to_irq(gpio->id);
		if (irq < 0) {
			return irq;
		}

		gpio->irq = irq;
		interrupts[gpio->irq] = gpio->id;

		rc = request_irq(gpio->irq, (irq_handler_t) dev_irq_handler, IRQF_TRIGGER_HIGH, "bbb_gpio_handler", NULL);
		if (rc) {
			return rc;
		}

		gpio_interrupt_counter++;
		gpio_poll_counter = gpio_interrupt_counter;

		printk(KERN_INFO "BBGPIO: The interrupt request result is: %d\n", rc);
	}

	printk(KERN_INFO "BBGPIO: GPIO with Id: %u Value: %u Direction: %u IRQ : %d configured\n", gpio->id, gpio->value, gpio->direction, gpio->irq);

	return 0;
}

static void gpio_exit(struct Gpio *gpio)
{
	gpio_set_value(gpio->id, 0);
	gpio_unexport(gpio->id);
	gpio_free(gpio->id);
	gpio->exported = 0;

	if (gpio->irq > 0) {

		free_irq(gpio->irq, NULL);
		gpio->irq = 0;
		interrupts[gpio->irq] = 0;

		gpio_irq_data |= gpio->mask;
		gpio_last_mask = gpio->mask;

		wake_up_interruptible_all(&gpio_wait);

		gpio_interrupt_counter--;
		gpio->mask = 0;

	}

	printk(KERN_INFO "BBGPIO: GPIO with Id: %u freed\n", gpio->id);
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	if (!mutex_trylock(&dev_mutex)) {
		printk(KERN_ALERT "BBGPIO: Device in use by another process");
		return -EBUSY;
	}
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error_count, size_of_message;
	char message[256] = {0};

	sprintf(message, "%u", gpio_value);
	size_of_message = strlen(message);

	if (*offset >= size_of_message) {
		return 0;
	}

	error_count = copy_to_user(buffer, message, size_of_message);
	if (error_count != 0) {
		printk(KERN_INFO "BBGPIO: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}

	*offset += size_of_message;
	return size_of_message;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int rc, error_count;
	char message[256] = {0}, choice;
	unsigned int id, param;
	unsigned int mask;

	error_count = copy_from_user(message, buffer, len);
	if (error_count != 0) {
		printk(KERN_INFO "BBGPIO: Failed to receive %d characters from the user\n", error_count);
		return -EFAULT;
	}

	sscanf(message, "%c %u %u %u", &choice, &id, &param, &mask);

	if (id >= 65) {
		return -EINVAL;
	}

	switch (choice) {
	case 'i':
		if (!gpios[id].exported) {
			gpios[id].direction = param;
			gpios[id].mask = mask;
			rc = gpio_init(&gpios[id]);
			if (rc) {
				return rc;
			}
		}
		break;
	case 'r':
		if (gpios[id].exported) {
			if (!gpios[id].direction) {
				gpio_value = gpio_get_value(gpios[id].id);
			} else {
				gpio_value = gpios[id].value;
			}
		}
		break;
	case 'w':
		if (gpios[id].exported) {
			gpios[id].value = param;
			gpio_set_value(gpios[id].id, gpios[id].value);
		}
		break;
	case 'f':
		if (gpios[id].exported) {
			gpio_exit(&gpios[id]);
			gpios[id].exported = 0;
		}
		break;
	}

	return len;
}

static unsigned int dev_poll(struct file *filep, struct poll_table_struct *wait)
{
	poll_wait(filep, &gpio_wait, wait);

	printk(KERN_INFO "BBGPIO: Poll function continued with data: %u\n", gpio_irq_data);
	if (gpio_irq_data) {
		printk(KERN_INFO "BBGPIO: Poll counter: %d\n", --gpio_poll_counter);
		if (gpio_poll_counter <= 0) {
			unsigned int l = gpio_irq_data;
			gpio_irq_data &= ~gpio_last_mask;
			gpio_poll_counter = gpio_interrupt_counter;
			return l;
		} else {
			return gpio_irq_data;
		}
	}

	return 0;
}

static irq_handler_t  dev_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int gpio_id;

	gpio_id = interrupts[irq];
	gpio_irq_data |= gpios[gpio_id].mask;
	gpio_last_mask = gpios[gpio_id].mask;

	wake_up_interruptible_all(&gpio_wait);

	return (irq_handler_t) IRQ_HANDLED;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&dev_mutex);
	return 0;
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);
