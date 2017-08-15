#ifndef _device_h_
#define _device_h_

#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/device.h>

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static unsigned int dev_poll(struct file *, struct poll_table_struct *);

static ssize_t dev_init(void);
static void dev_exit(void);

static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
	.poll = dev_poll
};

struct Device {
	int major_number;
	struct class *class;
	struct device *device;
};

static struct Device device;

static DEFINE_MUTEX(dev_mutex);
#endif
