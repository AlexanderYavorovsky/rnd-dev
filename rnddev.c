#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yavorovsky Alexander");
MODULE_DESCRIPTION("Random byte generator character device");

static int __init rnddev_init(void)
{
    printk(KERN_INFO "Rnd dev start\n");

    return 0;
}

static void __exit rnddev_exit(void)
{
    printk(KERN_INFO "Rnd dev exit\n");
}

module_init(rnddev_init);
module_exit(rnddev_exit);
