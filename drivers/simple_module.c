#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>

#define MODNAME "simple_module"

static dev_t devno;
static struct class *simple_class;
static int count = 0;

static int __init init_mod(void)
{
	int ret;
	struct device *dev;

	ret = alloc_chrdev_region(&devno, 0 /* first minor */, count, MODNAME);
	if (ret < 0) {
		pr_err("Could not allocate chrdev region\n");
		return ret;
	}

	simple_class = class_create(THIS_MODULE, MODNAME);
	if (IS_ERR(simple_class)) {
		pr_err("Could not create class\n");
		return PTR_ERR(simple_class);
	}

	dev = device_create(simple_class, NULL, devno, NULL, MODNAME);
	if (IS_ERR(dev)) {
		pr_err("Coukd not create_device\n");
		return PTR_ERR(dev);
	}

	pr_info("%s loaded\n", MODNAME);
	return 0;
}

static void __exit exit_mod(void)
{
	device_destroy(simple_class, devno);
	class_destroy(simple_class);
	unregister_chrdev_region(devno, count);
	pr_info("%s unloaded\n", MODNAME);
}

module_init(init_mod);
module_exit(exit_mod);
/* without it, insmod shows "Unknown symbol class_destroy" and don't load the module */
MODULE_LICENSE("GPL"); 
