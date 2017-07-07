#include <linux/kprobes.h>
#include <linux/module.h>

static bool capable_proxy(int cap)
{
	pr_info("Proxied cap value: %d\n", cap);
	jprobe_return();
	return 0;
}

static struct jprobe jp = {
	.entry = capable_proxy,
	.kp = {
		.symbol_name = "_do_fork",
	},
};

static int __init mod_init(void)
{
	int ret;

	ret = register_jprobe(&jp);
	if (ret < 0) {
		pr_info("jprobe err: %d\n", ret);
		return -1;
	}

	pr_info("Jprobe placed at %p, handler at %p\n", jp.kp.addr, jp.entry);
	return 0;
}

static void __exit mod_exit(void)
{
	unregister_jprobe(&jp);
	pr_info("jprobe at %p unregistered\n", jp.kp.addr);
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");
