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
		perror("err1");
		goto out;
	}

	printf("Allow some syscalls like write\n");

	ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
	if (ret != 0) {
		perror("err2");
		goto out;
	}

	ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
	if (ret != 0) {
		perror("err3");
		goto out;
	}

	ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
	if (ret != 0) {
		perror("err4");
		goto out;
	}

	ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0);
	if (ret != 0) { 
		perror("err6");
		goto out;
	}

	ret = seccomp_load(ctx);
	if (ret != 0) {
		perror("err7");
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
