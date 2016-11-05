Device Driver Notes
===================
`These are just personal notes while studying Linux Device Drivers`

1. When the **init function** returns < 0, insmod call **fails** and don't load the specified module
2. If **readv**/**writev** are not specified, **read**/**write** are called multiple times
   - When **read**/**write** returns a positive number, it checks if the number is smaller to the *count* parameter, if yes, then **read**/**write** will be called again, but this time will have a new value (older count - the returned value from last read/write call), and this will repeat until count reaches zero
3. To manipulate *proc* directories (linux/proc_fs.h):
   - struct proc_dir_entry *proc_mkdir(char *dirname, struct proc_dir_entry *parent);
   - struct proc_dir_entry *proc_create(const char *name, umode_t mode, struct proc_dir_entry *parent,
                                          const struct file_operations *proc_fops);
   - void remove_proc_entry(const char *, struct proc_dir_entry *);
   
4. Semaphores (linux/semaphore.h):
   - **down_interruptible** locks the sempahore, but available to be interrupted by signal
   - **up** unlocks the semaphore

5. Misc Device (linux/miscdevice.h)
   - Easy way to setup a simple device driver, like scull, that does nothing special
   - **misc_register** and **misc_deregister** does all the hard work.
   - If *probe* and *exit* functions just *register*/*deregister* a misc device, use **module_misc_device(struct miscdevice \*)**
   instead

Input subsystem tests
=====================

1. To check if your touchpad or input is working, or it's a WM problem. First of all, get the event file of your device:
   - Type cat /proc/bus/input/devices
   - Get the output and check for your desired device. In the output below we can see the input4
   - I: Bus=0011 Vendor=0002 Product=000e Version=0000
   - N: Name="ETPS/2 Elantech Touchpad"
   - P: Phys=isa0060/serio1/input0
   - S: Sysfs=/devices/platform/i8042/serio1/input/input5
   - U: Uniq=
   - H: Handlers=mouse0 event4
   - B: PROP=5
   - B: EV=b
   - B: KEY=e420 10000 0 0 0 0
   - B: ABS=661800011000003

2. Now execute the evtest program like the following:
   - sudo evtest /dev/input/event4

3. From now on, after each interaction you have with the device, a log will be printed in your terminal, like:
   - Event: time 1478365968.419065, -------------- SYN_REPORT ------------
   - Event: time 1478365968.431823, type 3 (EV_ABS), code 58 (ABS_MT_PRESSURE), value 3
   - Event: time 1478365968.431823, type 3 (EV_ABS), code 48 (ABS_MT_TOUCH_MAJOR), value 326
   - Event: time 1478365968.431823, type 3 (EV_ABS), code 28 (ABS_TOOL_WIDTH), value 2
   - Event: time 1478365968.431823, type 3 (EV_ABS), code 24 (ABS_PRESSURE), value 3
   - Event: time 1478365968.431823, -------------- SYN_REPORT ------------
   - Event: time 1478365968.444058, type 3 (EV_ABS), code 57 (ABS_MT_TRACKING_ID), value -1
   - Event: time 1478365968.444058, type 1 (EV_KEY), code 330 (BTN_TOUCH), value 0
   - Event: time 1478365968.444058, type 1 (EV_KEY), code 325 (BTN_TOOL_FINGER), value 0
   - Event: time 1478365968.444058, type 3 (EV_ABS), code 24 (ABS_PRESSURE), value 0
   - Event: time 1478365968.444058, -------------- SYN_REPORT ------------
