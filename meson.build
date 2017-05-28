project('kernel_experiments', 'c')
add_project_arguments(['-Wall', '-Werror', '-Wextra', '-I/usr/include/libevdev-1.0'], language: 'c')
dep_libevdev = dependency('libevdev')
dep_udev = dependency('libudev')

subdir('userspace')
#subdir('drivers')
