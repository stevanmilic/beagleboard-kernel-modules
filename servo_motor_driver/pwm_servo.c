#include <linux/init.h> // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h> // Core header for loading LKMs into the kernel
#include <linux/device.h> // Header to support the kernel Driver Model
#include <linux/kernel.h> // Contains types, macros, functions for the kernel
#include <linux/pwm.h> // Required for the PWM functions
#include <linux/fs.h> // Header for the Linux file system support
#include <linux/mutex.h> // Required for the mutex functionality
#include <asm/uaccess.h> // Required for the copy to user function

#define  DEVICE_NAME "ebbservo" // The device will appear at /dev/ebbservo using this value
#define  CLASS_NAME  "ebb" // The device class -- this is a character device driver

MODULE_LICENSE("GPL"); // The license type -- this affects available functionality
MODULE_AUTHOR("Stevan Milic"); // The author -- visible when you use modinfo
MODULE_DESCRIPTION("A Servo Motor driver for the BBB"); // The description -- see modinfo
MODULE_VERSION("0.1"); // A version number to inform users

static int    majorNumber;          // Stores the device number -- determined automatically
static struct class*  ebbservoClass  = NULL; // The device-driver class struct pointer
static struct device* ebbservoDevice = NULL; // The device-driver device struct pointer
static char   message[256] = {0};          // Buffer for sending and receiving messages from user space
static bool     devRead = 0;               // Flag for defining if reading is done

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

static unsigned int pwmServoId = 4;  // hard coding the Servo pwm for this example to PWM1A
static struct pwm_device *pwmServoDevice = NULL;
static int dutyCycleServo = 1458333; // 0.5ms pulse -> position "0"
static int periodServo = 16666667; //16.66ms period -> 60hz

static DEFINE_MUTEX(ebbservo_mutex); // A macro that is used to declare a new mutex that is visible in this file
// results in a semaphore variable ebbservo_mutex with value 1 (unlocked)
// DEFINE_MUTEX_LOCKED() results in a variable with value 0 (locked)

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init servogpio_init(void)
{
	printk(KERN_INFO "GPIO_TEST: Initializing the PWM_TEST LKM\n");

	mutex_init(&ebbservo_mutex); // Initialize the mutex lock dynamically at runtime

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber < 0) {
		printk(KERN_ALERT "Ebbservo failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "Ebbservo: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	ebbservoClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(ebbservoClass)) {               // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(ebbservoClass);          // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "Ebbservo: device class registered correctly\n");

	// Register the device driver
	ebbservoDevice = device_create(ebbservoClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(ebbservoDevice)) {              // Clean up if there is an error
		class_destroy(ebbservoClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(ebbservoDevice);
	}

	printk(KERN_INFO "Ebbservo: %d device class created correctly\n", pwmServoId); // Made it! device was initialized

	pwmServoDevice = pwm_request(pwmServoId, "sysfs"); //pwm_id is hardcoded to 2, request it
	if (IS_ERR(pwmServoDevice)) {
		printk(KERN_ALERT "Ebbservo failed to get pwm servo device\n");
		return PTR_ERR(pwmServoDevice);
	}

	pwm_config(pwmServoDevice, dutyCycleServo, periodServo);

	printk(KERN_INFO "Ebbservo: Period: %d Duty Cycle: %d\n", pwm_get_period(pwmServoDevice), pwm_get_duty_cycle(pwmServoDevice));

	pwm_enable(pwmServoDevice);

	printk(KERN_INFO "Ebbservo: Servo enabled successfully\n");

	return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit servogpio_exit(void)
{
	pwm_disable(pwmServoDevice);
	pwm_free(pwmServoDevice);
	device_destroy(ebbservoClass, MKDEV(majorNumber, 0)); // remove the device
	class_unregister(ebbservoClass); // unregister the device class
	class_destroy(ebbservoClass); // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME); // unregister the major number
	mutex_destroy(&ebbservo_mutex); // destroy the dynamically-allocated mutex
	printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
*  This will only increment the numberOpens counter in this case.
*  @param inodep A pointer to an inode object (defined in linux / fs.h)
*  @param filep A pointer to a file object (defined in linux / fs.h)
*/
static int dev_open(struct inode *inodep, struct file *filep)
{
	if (!mutex_trylock(&ebbservo_mutex)) {
		// Try to acquire the mutex (i.e., put the lock on/down)
		// returns 1 if successful and 0 if there is contention
		printk(KERN_ALERT "ebbservo: Device in use by another process");
		return -EBUSY;
	}
	printk(KERN_INFO "ebbservo: Device has been opened\n");
	return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error_count, size_of_message;

	if (devRead == 1) {
		return (devRead = 0);
	}

	sprintf(message, "%d", pwm_get_duty_cycle(pwmServoDevice));
	size_of_message = strlen(message) + 1;

	error_count = copy_to_user(buffer, message, size_of_message);

	if (error_count == 0) {         // if true then have success
		printk(KERN_INFO "ebbservo: Sent %d character to the user\n", size_of_message);
		devRead = 1;
		return size_of_message;
	} else {
		printk(KERN_INFO "ebbservo: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int error_count = copy_from_user(message, buffer, len);

	if (error_count == 0) {
		printk(KERN_INFO "ebbservo: Received %zu characters from the user\n", len);
		sscanf(message, "%d", &dutyCycleServo);
		pwm_config(pwmServoDevice, dutyCycleServo, periodServo);
	} else {
		printk(KERN_INFO "ebbservo: Failed to receive %d characters from the user\n", error_count);
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}

	return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep)
{
	mutex_unlock(&ebbservo_mutex); // Releases the mutex (i.e., the lock goes up)
	printk(KERN_INFO "ebbservo: Device successfully closed\n");
	return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(servogpio_init);
module_exit(servogpio_exit);
