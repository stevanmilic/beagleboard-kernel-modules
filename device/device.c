#define "device.h"

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

	printk(KERN_INFO "BB: device has been successfully configured\n");

	mutex_init(&dev_mutex);

	return 0;
}

static void dev_exit(void)
{
	device_destroy(device.class, MKDEV(device.major_number, 0));
	class_destroy(device.class);
	unregister_chrdev(device.major_number, DEVICE_NAME);
	mutex_destroy(&dev_mutex);
	printk(KERN_INFO "BB: device has been unregistered\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	if (!mutex_trylock(&dev_mutex)) {
		printk(KERN_ALERT "BB: Device in use by another process");
		return -EBUSY;
	}
	printk(KERN_INFO "BB: Device successfully opened\n");
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&dev_mutex);
	printk(KERN_INFO "BB: Device successfully closed\n");
	return 0;
}
