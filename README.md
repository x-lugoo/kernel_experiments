`These are just personal notes while studying Linux Device Drivers`

Problem solving
===============

1. When you try to load a module and it shows some *Unkonwn symbols*, like *Unknown symbol debugfs_remove_recursive*, you need to add a license in your module, like **MODULE_LICENSE("GPL")**.

2. When starting kernel build process with KBUILD_VERBOSE set as 1, it shows the exactly gcc command to compile each file, useful for debugging.

Device Driver Notes and Useful Functions
========================================

1. When the **init function** returns < 0, insmod call **fails** and don't load the specified module
2. If **readv**/**writev** are not specified, **read**/**write** are called multiple times
   - When **read**/**write** returns a positive number, it checks if the number is smaller to the *count* parameter, if yes, then **read**/**write** will be called again, but this time will have a new value (older count - the returned value from last read/write call), and this will repeat until count reaches zero
3. To manipulate *proc* directories (linux/proc_fs.h):
   ```C
   struct proc_dir_entry *proc_mkdir(char *dirname, struct proc_dir_entry *parent);
   struct proc_dir_entry *proc_create(const char *name, umode_t mode, struct proc_dir_entry *parent,
                                          const struct file_operations *proc_fops);
   void remove_proc_entry(const char *, struct proc_dir_entry *);
   ```
   
4. Semaphores and mutexes (linux/semaphore.h, linux/mutex.h):
   ```C
   /* to lock/unlock an semaphore */
   int down_interruptible(struct semaphore *sem); /* locks but available to be interrupted by signal */
   void up(struct semaphore *sem); /* unlocks the up */
   /* to lock/unlock mutexes */
   mutex_lock(struct mutex *m);
   mutex_unlock(struct mutex *m);
   ```

5. Misc Device (linux/miscdevice.h)
   - Easy way to setup a simple device driver, like scull, that does nothing special
   - **misc_register** and **misc_deregister** does all the hard work.
   - If *probe* and *exit* functions just *register*/*deregister* a misc device, use **module_misc_device(struct miscdevice \*)**
   instead

6. Create Device (linux/device.h)
   - This registers a device, and tell userspace (aka udev or systemd) to create a file under /dev dir.
   - **device_create** and **device_destroy**, does this job. To call **device_create**, you need to pass a class pointer, like below:
   ```C
   dev_t dev = MKDEV(Major,0);
   struct class *c = class_create(THIS_MODULE, "name_of_file_in_dev_dir_to_be_created");
   class_destroy(c);
   struct device *d = device_create(c, NULL /* parent */, dev, NULL /* no additional data */, "same name as before");
   device_destroy(c, dev);
   ```

7. Simple write/read from user (linux/fs.h)
   - To handle simple user readers or writes from userspace, you can use the functions **simple_{read,write}_from_buffer**. These functions already takes care of all possible errors while reading/writing, like invalid position and **copy_{from,to}_user** errors.
   ```C
   ssize_t simple_read_from_buffer(void __user *to, size_t count, loff_t *ppos
				, const void *from, size_t available);
   ssize_t simple_write_to_buffer(void *to, size_t available, loff_t *ppos
   				, const void __user *from, size_t count);
   ```

8. Debugfs creation routines (linux/debugfs.h):
   ```C
   /* to create a new directory under debugfs */
   struct dentry *debugfs_create_dir(const char *name, struct dentry *parent);
   /* to create a new file in debugfs */
   struct dentry *debufs_create_file(const char *name, umode_t mode, struct dentry *parent
   				, void *data /* is passed in every call */
				, const struct file_operations *fops);
   /* to remove all debugfs files below a dir */
   void debugfs_remove_recursive(struct dentry *dentry);
   ```

9. Credential manipulations routines (linux/cred.h, linux/uidgid.h):
   ```C
   /* to get the current effective user (uid) */
   current_uid();
   /* get the current cred structure */
   current_cred();
   /* compare the UIDs */
   bool uid_eq();
   /* definition of root user */
   GLOBAL_ROOT_UID
   ```
10. Sysfs manipulation (linux/sysfs.h, linux/kobject.h)
   ```C
   /* create root sysfs dir to your aplication */
   struct kobject *kobj = kobject_create_and_add("directory_name", parent_kobj);

   /* create file within this directory and add methods to access it */
   char *foo;
   size_t foo_show(struct kobject *kobj, struct kobj_attribute *attr
			, char *buf) { ... }
   size_t foo_store(struct kobject *kobj, struct kobj_attribute *attr
			, char *buf, size_t count) { ... }

   /* implies S_IRUSR | S_IWUSR, foo_show and foo_store */
   struct kobj_attribute foo_attr = __ATTR(foo);

   struct attribute *attrs[] = { &foo_attr.attr, NULL, };

   struct attribute_group attr_group = { .attrs = attrs; };

   /* to add more files, create more #_show, #_store, and new #_attrs */
   struct kobject \*child = sysfs_create_group(kobj, &attr_group);
   ```
11. ACPI object descriptions
```sh
OSPM	Operating Systems-directed configuration and Power Management
_HID	supply OSPM with a device's Plug and Play Hardware ID
_CID	supply OSPM with a device's Plug and Play Device ID
_UID	Unique ID who doesn't change across reboots. This should be different from _HIDs and _CIDs.
_CLS	Class Code, PCI-defined class, subclass or programming interface of a device.
```

12. Debug ACPI methods
```sh
Extract your ACPI data, extract the ACPI tables, decompile it and run the acpiexec:
sudo acpidump >acpi.dat
acpixtract acpi.dat
iasl *.dat; rm acpi.dat
acpiexec -vi -b "methods" *.dsl # this will prompt all methods available
acpiexec -vi *.dsl # this command alone will load all dsl files and prompt you a command to be issued

# type execute to call an ACPI method from the dsl files:
- execute \_SB.PCI0.LPCB.PS2M._HID
Evaluating \_SB.PCI0.LPCB.PS2M._HID
Evaluation of \_SB.PCI0.LPCB.PS2M._HID returned object 0x55bd05ec2a60, external buffer length 18
  [Integer] = 00000000060A2E4F
```

Input subsystem tests
=====================

Using evtest
------------
      1. To check if your touchpad or input is working, or it's a WM problem. First of all, get the event file of your device:
         - Type cat /proc/bus/input/devices
      2. Get the output and check for your desired device. In the output below we can see the input4
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
      3. Now execute the evtest program like the following:
         - sudo evtest /dev/input/event4
      4. From now on, after each interaction you have with the device, a log will be printed in your terminal, like:
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

Using xev
---------
      1. xev creates a window and then asks the X server to send it events whenever anything happens to the
      window (such as it being moved, resized, typed in, clicked in, etc.). You can  also attach it to an existing
      window. It is useful for seeing what causes events to occur and to display the information that they contain; it
      is essentially a debugging and development tool, and should not be needed in normal usage. (Description grabbed
      from xev's manpage). Output example:
         -FocusIn event, serial 36, synthetic NO, window 0x2000001,
         -  mode NotifyWhileGrabbed, detail NotifyNonlinear
         -KeymapNotify event, serial 36, synthetic NO, window 0x0,
         -  keys:  0   0   4294967168 0   0   0   0   0   0   0   0   0   0   0   0   0   
         -         0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   
         -PropertyNotify event, serial 36, synthetic NO, window 0x2000001,
         -   atom 0x128 (_NET_WM_STATE), time 664328400, state PropertyNewValue
         -FocusIn event, serial 36, synthetic NO, window 0x2000001,
         -   mode NotifyUngrab, detail NotifyNonlinear
         -KeymapNotify event, serial 36, synthetic NO, window 0x0,
         -   keys:  3   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   
         -          0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   
