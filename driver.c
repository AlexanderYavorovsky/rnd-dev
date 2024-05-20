#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yavorovsky Alexander");
MODULE_DESCRIPTION("Random byte generator character device");

#include "gf.h"
#include "poly.h"

static int __init rnddev_init(void);
static void __exit rnddev_exit(void);
static int rnddev_open(struct inode *, struct file *);
static int rnddev_release(struct inode *, struct file *);
static ssize_t rnddev_read(struct file *, char *, size_t, loff_t *);
static ssize_t rnddev_write(struct file *, const char *, size_t, loff_t *);

#define DEVICE_NAME "rnddev"
#define MAX_LEN 30

static int Major;
static int Device_Open = 0;

static struct file_operations fops = { .read = rnddev_read,
				       .write = rnddev_write,
				       .open = rnddev_open,
				       .release = rnddev_release };

static int crs_len = 0;
static int crs_coeffs[MAX_LEN];
static int crs_elems[MAX_LEN];
static int crs_c = 0;

module_param(crs_len, int, 0); /* todo: 0? */
MODULE_PARM_DESC(crs_len, "CRS length");

module_param_array(crs_coeffs, int, NULL, 0);
MODULE_PARM_DESC(crs_coeffs, "An array of CRS coefficients a_0..a_{k-1}");

module_param_array(crs_elems, int, NULL, 0);
MODULE_PARM_DESC(crs_elems, "An array of CRS elements x_0..x_{k-1}");

module_param(crs_c, int, 0);
MODULE_PARM_DESC(crs_c, "CRS constant `c`");

static int __init rnddev_init(void)
{
	printk(KERN_INFO "Rnd dev start.\n");

	if (crs_len == 0) {
		printk(KERN_ALERT
		       "EINVAL: crs_len is zero! Cannot make a sequence. Abort.\n");
		return -EINVAL;
	}

	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "Registration failed with error %d.\n",
		       Major);
		return Major;
	}

	printk(KERN_INFO "Major: %d\n", Major);
	printk(KERN_INFO "Create chrdev using: mknod /dev/%s c %d 0\n",
	       DEVICE_NAME, Major);

	/* debug info */
	printk(KERN_INFO "crs_len: %d\n", crs_len);
	printk(KERN_INFO "crs_c: %d\n", crs_c);
	for (int i = 0; i < crs_len; i++)
		printk(KERN_INFO "crs_coeffs[%d] = %d\n", i, crs_coeffs[i]);
	for (int i = 0; i < crs_len; i++)
		printk(KERN_INFO "crs_elems[%d] = %d\n", i, crs_elems[i]);

	/* init sequence */

	return 0;
}

static void __exit rnddev_exit(void)
{
	unregister_chrdev(Major, DEVICE_NAME);

	/* free memory */

	printk(KERN_INFO "Rnd dev exit.\n");
}

static int rnddev_open(struct inode *inode, struct file *file)
{
	if (Device_Open)
		return -EBUSY;

	try_module_get(THIS_MODULE);

	return 0;
}

static int rnddev_release(struct inode *, struct file *)
{
	Device_Open--;
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t rnddev_read(struct file *filp, char *buffer, size_t length,
			   loff_t *offset)
{
	/* give 1 rnd byte */

	return 1;
}

static ssize_t rnddev_write(struct file *filp, const char *buff, size_t length,
			    loff_t *offset)
{
	printk(KERN_ALERT "Write is not supported.\n");
	return -EINVAL;
}

module_init(rnddev_init);
module_exit(rnddev_exit);
