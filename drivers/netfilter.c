#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/module.h>

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/textsearch.h>

#define LINUX "linux"
static struct ts_config *ts;

static unsigned int hook_func(void *priv, struct sk_buff *skb
				, const struct nf_hook_state *state)
{
	struct tcphdr *thdr;
	unsigned int ret;

	if (!skb)
		return NF_ACCEPT;

	thdr = tcp_hdr(skb);

	/* filter just for port 80 (HTTP) */
	if (ntohs(thdr->source) != 80)
		return NF_ACCEPT;

	ret = skb_find_text(skb, tcp_hdrlen(skb), skb->len, ts);
	if (ret != UINT_MAX)
		pr_info("Found linux in this page\n");

	return NF_ACCEPT;
}

static struct nf_hook_ops hook = {
	.hook = &hook_func,
	.pf = PF_INET,
	.hooknum = NF_INET_PRE_ROUTING,
	.priority = NF_IP_PRI_FIRST,
};

static int __init nf_enter(void)
{
	pr_info("adding a nf hook\n");
	ts = textsearch_prepare("kmp", LINUX, strlen(LINUX), GFP_KERNEL
			, TS_AUTOLOAD | TS_IGNORECASE);
	if (IS_ERR(ts))
		return PTR_ERR(ts);

	return nf_register_hook(&hook);
}

static void __exit nf_exit(void)
{
	pr_info("removing a nf hook\n");
	textsearch_destroy(ts);
	nf_unregister_hook(&hook);
}

module_init(nf_enter);
module_exit(nf_exit);
