project('kernel_experiments', 'c'
	, default_options: ['prefix=/usr'])
add_project_arguments(['-Wall', '-Werror', '-Wextra'
		, '-Og', '-fstack-protector-strong', '-D_FORTIFY_SOURCE=2'
		, '-I/usr/include/libevdev-1.0'], language: 'c')
add_project_link_arguments('-fstack-protector-strong'
		, '-Wl,-z,relro,-z,now', language: 'c')

incdir = include_directories('userspace')

subdir('userspace/libinput')
subdir('userspace/misc')
subdir('userspace/namespaces')
subdir('userspace/uinput')
