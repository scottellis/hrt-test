/*
  Checking the effective resolution of a Linux hrtimer on an OMAP3 kernel.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>

// this is pin 19 on the TOBI/SUMMIT Gumstix expansion boards
#define TOGGLE_PIN 170

#define USEC_TO_NSEC(x)	((x) * 1000)

struct hrt_dev {
	dev_t devt;
	struct cdev cdev;
	struct semaphore sem;
	struct class *class;
	struct hrtimer timer;
	ktime_t delay;
	int val;
	int count;
};

static struct hrt_dev hrt;

static enum hrtimer_restart hrt_timer_callback(struct hrtimer *timer)
{
	hrt.count++;
	
	if (hrt.count > 199) {
		// leave at zero for next run
		gpio_set_value(TOGGLE_PIN, 0);
		return HRTIMER_NORESTART;
	}

	hrt.val = !hrt.val;
	gpio_set_value(TOGGLE_PIN, hrt.val);

	hrtimer_forward_now(&hrt.timer, hrt.delay);

	return HRTIMER_RESTART;
}

static void do_toggle_test(void)
{
	hrt.val = 1;
	hrt.count = 0;
	gpio_set_value(TOGGLE_PIN, 1);

	hrtimer_start(&hrt.timer, hrt.delay, HRTIMER_MODE_REL);
}

static ssize_t hrt_write(struct file *filp, const char __user *buff,
		size_t count, loff_t *f_pos)
{
	char str[16];
	ssize_t status;
	unsigned long usecs;

	if (count == 0)
		return 0;

	if (down_interruptible(&hrt.sem))
		return -ERESTARTSYS;

	if (count > 8)
		count = 8; 

	str[count] = 0;

	if (copy_from_user(str, buff, count)) {
		printk(KERN_ALERT "Error copy_from_user\n");
		status = -EFAULT;
		goto hrt_write_done;
	}

	usecs = simple_strtoul(str, NULL, 10);

	if (usecs < 1 || usecs > 500000) {
		printk(KERN_ALERT "invalid delay %lu\n", usecs);
		status = -EINVAL;
		goto hrt_write_done;
	}
	
	hrt.delay = ktime_set(0, USEC_TO_NSEC(usecs));

	do_toggle_test();	

	status = count;
	*f_pos += count;

hrt_write_done:

	up(&hrt.sem);

	return status;
}

static const struct file_operations hrt_fops = {
	.owner = THIS_MODULE,
	.write = hrt_write,
};

static int __init hrt_init_cdev(void)
{
	int error;

	hrt.devt = MKDEV(0, 0);

	error = alloc_chrdev_region(&hrt.devt, 0, 1, "hrt");
	if (error < 0) {
		printk(KERN_ALERT 
			"alloc_chrdev_region() failed: error = %d \n", 
			error);
		
		return -1;
	}

	cdev_init(&hrt.cdev, &hrt_fops);
	hrt.cdev.owner = THIS_MODULE;

	error = cdev_add(&hrt.cdev, hrt.devt, 1);
	if (error) {
		printk(KERN_ALERT "cdev_add() failed: error = %d\n", error);
		unregister_chrdev_region(hrt.devt, 1);
		return -1;
	}	

	return 0;
}

static int __init hrt_init_class(void)
{
	hrt.class = class_create(THIS_MODULE, "hrt");

	if (!hrt.class) {
		printk(KERN_ALERT "class_create(hrt) failed\n");
		return -1;
	}

	if (!device_create(hrt.class, NULL, hrt.devt, NULL, "hrt")) {
		class_destroy(hrt.class);
		return -1;
	}

	return 0;
}

static int __init hrt_init_pins(void)
{
	if (gpio_request(TOGGLE_PIN, "togglepin")) {
		printk(KERN_ALERT "gpio_request failed\n");
		return -1;
	}

	if (gpio_direction_output(TOGGLE_PIN, 0)) {
		printk(KERN_ALERT "gpio_direction_output failed\n");
		gpio_free(TOGGLE_PIN);
		return -1;
	}

	return 0;


init_pins_fail_2:
	gpio_free(TOGGLE_PIN);

init_pins_fail_1:

	return -1;
}

static int __init hrt_init(void)
{
	sema_init(&hrt.sem, 1);
	
	if (hrt_init_cdev())
		goto init_fail_1;

	if (hrt_init_class())
		goto init_fail_2;

	if (hrt_init_pins() < 0)
		goto init_fail_3;

	hrtimer_init(&hrt.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrt.timer.function = hrt_timer_callback;

	return 0;

init_fail_3:
	device_destroy(hrt.class, hrt.devt);
  	class_destroy(hrt.class);

init_fail_2:
	cdev_del(&hrt.cdev);
	unregister_chrdev_region(hrt.devt, 1);

init_fail_1:

	return -1;
}
module_init(hrt_init);

static void __exit hrt_exit(void)
{
	hrtimer_cancel(&hrt.timer);

	gpio_free(TOGGLE_PIN);

	device_destroy(hrt.class, hrt.devt);
  	class_destroy(hrt.class);

	cdev_del(&hrt.cdev);
	unregister_chrdev_region(hrt.devt, 1);
}
module_exit(hrt_exit);


MODULE_AUTHOR("Scott Ellis");
MODULE_DESCRIPTION("Test the hrtimer resolution on an OMAP3");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

