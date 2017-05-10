add_project_arguments(['-Wall', '-Werror', '-Wextra', '-I/usr/include/libevdev-1.0'], language: 'c')
dep_libevdev = dependency('libevdev')
dep_udev = dependency('libudev')

executable('uinput_kb', 'uinput_kb.c')
executable('uinput_mouse', 'uinput_mouse.c')
executable('event_dev_info', 'event_dev_info.c')
executable('hid_dev_info', 'hid_dev_info.c')
executable('uinput_old_style', 'uinput_old_style.c')
executable('joystick_interface', 'joystick_interface.c')
executable('evdev', 'evdev.c'
	, dependencies: dep_libevdev)
executable('evdev_libudev', 'evdev_libudev.c'
	, dependencies: [dep_libevdev, dep_udev])
