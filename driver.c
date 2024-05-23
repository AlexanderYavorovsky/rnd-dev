#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yavorovsky Alexander");
MODULE_DESCRIPTION("Random byte generator character device");

#include "poly.h"
#include "gf.h"

static int __init rnddev_init(void);
static void __exit rnddev_exit(void);
static void init_crs(void);
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

static size_t crs_len = 0;
static uint8_t crs_coeffs[MAX_LEN];
static uint8_t crs_init_elems[MAX_LEN];
static uint8_t crs_c = 0;
static gf_elem_t gf_crs_coeffs[MAX_LEN];
static gf_elem_t gf_crs_elems[MAX_LEN];
static gf_elem_t gf_crs_constant;

module_param(crs_len, ulong, 0); /* todo: 0? */
MODULE_PARM_DESC(crs_len, "CRS length");

module_param_array(crs_coeffs, byte, NULL, 0);
MODULE_PARM_DESC(crs_coeffs, "An array of CRS coefficients a_0..a_{k-1}");

module_param_array(crs_init_elems, byte, NULL, 0);
MODULE_PARM_DESC(crs_init_elems, "An array of CRS elements x_0..x_{k-1}");

module_param(crs_c, byte, 0);
MODULE_PARM_DESC(crs_c, "CRS constant");

static int __init rnddev_init(void)
{
	size_t i;

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
	printk(KERN_INFO "crs_len: %lu\n", crs_len);
	printk(KERN_INFO "crs_c: %d\n", crs_c);
	for (i = 0; i < crs_len; i++)
		printk(KERN_INFO "crs_coeffs[%lu] = %d\n", i, crs_coeffs[i]);
	for (i = 0; i < crs_len; i++)
		printk(KERN_INFO "crs_init_elems[%lu] = %d\n", i,
		       crs_init_elems[i]);

	init_crs();

	return 0;
}

static void __exit rnddev_exit(void)
{
	size_t i;

	unregister_chrdev(Major, DEVICE_NAME);

	/* free up memory */
	gf_elem_free(gf_crs_constant);
	for (i = 0; i < crs_len; i++) {
		gf_elem_free(gf_crs_coeffs[i]);
		gf_elem_free(gf_crs_elems[i]);
	}

	printk(KERN_INFO "Rnd dev exit.\n");
}

/* initialize sequence */
static void init_crs(void)
{
	size_t i;

	gf_crs_constant = uint8_to_gf_elem(crs_c);
	for (i = 0; i < crs_len; i++) {
		gf_crs_elems[i] = uint8_to_gf_elem(crs_init_elems[i]);
		gf_crs_coeffs[i] = uint8_to_gf_elem(crs_coeffs[i]);
	}
}

static int rnddev_open(struct inode *inode, struct file *file)
{
	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	try_module_get(THIS_MODULE);

	printk(KERN_INFO "OPENED\n");

	return 0;
}

static int rnddev_release(struct inode *, struct file *)
{
	Device_Open--;
	module_put(THIS_MODULE);

	printk(KERN_INFO "released\n");

	return 0;
}

static ssize_t rnddev_read(struct file *filp, char *buffer, size_t length,
			   loff_t *offset)
{
	/* give 1 rnd byte */
	uint8_t res = 4;

#if 0
	gf_elem_t x = gf_elem_copy(gf_crs_constant);
	gf_elem_t tmp;
	gf_elem_t prod;
	size_t i;


	for (i = 0; i < crs_len; i++) {
		prod = gf_multiply(gf_crs_coeffs[i], gf_crs_elems[i]);
		tmp = gf_sum(x, prod);
		gf_elem_free(x);
		gf_elem_free(prod);
		x = tmp;
	}

	res = gf_elem_to_uint8(x);

	if (copy_to_user(buffer, &res, sizeof(res))) {
		printk(KERN_ALERT "COPY ERROR\n");
	}
#endif

	if (*offset != 0)
		return 0;

	printk(KERN_INFO "COPYING\n");
	put_user(res, buffer);
	*offset += 1;
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
