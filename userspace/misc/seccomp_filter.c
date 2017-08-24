#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include <seccomp.h>

int main(void)
{
	scmp_filter_ctx ctx = NULL;
	int ret;

	printf("Init seccomp ctx to KILL all syscalls\n");
	ctx = seccomp_init(SCMP_ACT_KILL);
	if (ctx == NULL) {
		perror("seccomp_init");
		goto out;
	}

	printf("Allow some syscalls like write\n");

	ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
	if (ret != 0) {
		perror("seccomp_rule_add write");
		goto out;
	}

	ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
	if (ret != 0) {
		perror("seccomp_rule_add exit");
		goto out;
	}

	ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
	if (ret != 0) {
		perror("seccomp_rule_add exit_group");
		goto out;
	}

	ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0);
	if (ret != 0) { 
		perror("seccomp_rule_add open");
		goto out;
	}

	ret = seccomp_load(ctx);
	if (ret != 0) {
		perror("seccomp_load");
		goto out;
	}

	printf("We now should die, as close syscall is not enabled...\n");

	close(2);

	printf("This message shoul not appear!!\n");

	ret = 0;

out:
	seccomp_release(ctx);

	return ret;
}
