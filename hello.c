#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
	printk(KERN_ALERT "Hello, world!\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_ALERT "Good bye, cruel world!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_INFO(About, "Just a 'hello world' module");
MODULE_AUTHOR("Marcos Paulo de Souza <marcos.souza.org@gmail.com>");
MODULE_ALIAS("hello-world");
