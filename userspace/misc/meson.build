dep_libseccomp = dependency('libseccomp')

executable('syscall_getcpu', 'syscall_getcpu.c')
executable('seccomp', 'seccomp.c')
executable('seccomp_filter', 'seccomp_filter.c'
	, dependencies: [dep_libseccomp])

executable('fs_fat', 'fs_fat.c')
executable('netlink_route', 'netlink_route.c')
executable('netlink_kobject_uevent', 'netlink_kobject_uevent.c')


