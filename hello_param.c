#include <linux/init.h>
#include <linux/module.h>

static char *whom = "world";
module_param(whom, charp, S_IRUGO);

static int howmany = 1;
module_param(howmany, int, S_IRUGO);

static int __init hello_init(void)
{
	int i;

	for (i = 0; i < howmany; i++)
		pr_alert("Hello, %s!\n", whom);

	return 0;
}

static void __exit hello_exit(void)
{
	pr_alert("Good bye, %s!\n", whom);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_INFO(About, "Just a 'hello world' module, with parameters");
MODULE_AUTHOR("Marcos Paulo de Souza <marcos.souza.org@gmail.com>");
MODULE_ALIAS("hello-world-param");
