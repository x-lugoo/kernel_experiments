#include <linux/uaccess.h>

#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "scull.h"

int scull_major;
int scull_count = 1;
int scull_quantum = 5;
module_param(scull_quantum, int, S_IRUGO);

int scull_qset = 10;
module_param(scull_qset, int, S_IRUGO);

struct _qset {
	void **data;
	struct _qset *next;
};

struct scull_dev {
	struct _qset *qset_data; /// Pointer to the first quantum set
	int quantum; /// current quantum size
	int qset; /// current array size
	unsigned long size; /// amount of data stored
	unsigned int access_key; /// used by sculluid and scullpriv
	struct semaphore sem; /// mutex semaphore
	struct cdev cdev; /// char device struct
};

struct scull_dev *scull_dev;
static struct proc_dir_entry *scull_proc_dir;

int scull_trim(struct scull_dev *dev)
{
	struct _qset *next, *dptr;
	int qset = dev->qset; // dev is not null
	int i;

	for (dptr = dev->qset_data; dptr; dptr = next) {
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->qset_data = NULL;
	dev->quantum = scull_quantum;
	dev->qset = scull_qset;
	dev->size = 0;
	return 0;
}

struct _qset *scull_follow(struct scull_dev *dev, int item)
{
	struct _qset *dptr = dev->qset_data;

	if (!dptr) {
		dptr = dev->qset_data = kzalloc(sizeof(struct _qset)
						, GFP_KERNEL);
		if (!dptr) {
			pr_err("Could not allocate scull_qset1\n");
			return NULL;
		}
	}

	while (item--) {
		if (!dptr->next) {
			dptr->next = kzalloc(sizeof(struct _qset)
						, GFP_KERNEL);
			if (!dptr->next)
				return NULL;
		}
		dptr = dptr->next;
	}
	return dptr;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count
			, loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct _qset *dptr; /* first listitem */
	int quantum = dev->quantum;
	int qset = dev->qset;
	int itemsize = quantum * qset; /* how many bites in listitem*/
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (*f_pos > dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	/* find lisitem, qset and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = scull_follow(dev, item);
	if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
		goto out;  /* don't fill holes */

	/* realy only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;
out:
	up(&dev->sem);
	return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count
			, loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct _qset *dptr;
	int quantum = dev->quantum;
	int qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	int retval = -ENOMEM;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	dptr = scull_follow(dev, item);
	if (dptr == NULL) {
		pr_info("write dptr NULL\n");
		goto out;
	}
	if (!dptr->data) {
		dptr->data = kcalloc(qset, sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
	}

	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kzalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}

	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;

	if (dev->size < *f_pos)
		dev->size = *f_pos;

out:
	up(&dev->sem);
	return retval;
}

int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;

	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;

	// now trim if device if open was write only
	if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
		scull_trim(dev); //FIXME: check errors
	return 0;
}

static int scull_proc_show(struct seq_file *sf, void *v)
{
	seq_printf(sf, "%d\n", scull_major);
	return 0;
}

static int scull_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, scull_proc_show, NULL);
}

static const struct file_operations scull_proc_ops = {
	.owner = THIS_MODULE,
	.open = scull_proc_open,
	.read = seq_read,
};

static long scull_ioctl(struct file *filp, unsigned int cmd
			,unsigned long arg)
{
	switch(cmd) {
	case SCULL_IOCGQUANTUM:
		pr_info("GQUANTUM\n");
		return put_user(scull_quantum, (unsigned long *)arg);
	case SCULL_IOCGQSET:
		pr_info("GQSET\n");
		return put_user(scull_qset, (unsigned long *)arg);
	}
	return -1;
}

const struct file_operations scull_fops = {
	.owner =   THIS_MODULE,
	.read =    scull_read,
	.write =   scull_write,
	.open =    scull_open,
	.release = scull_release,
	.unlocked_ioctl = scull_ioctl,
};

static int scull_proc_init(void)
{
	scull_proc_dir = proc_mkdir("scull", NULL);
	if (!scull_proc_dir)
		return -ENOMEM;

	if (!proc_create("major", 0, scull_proc_dir, &scull_proc_ops))
		goto fail;

	return 0;

fail:
	remove_proc_entry("scull", NULL);
	return -ENOMEM;
}

static void scull_proc_exit(void)
{
	remove_proc_entry("major", scull_proc_dir);
	remove_proc_entry("scull", NULL);
}

static int __init scull_init(void)
{
	dev_t dev;
	int ret = -ENOMEM;
	char devt_fmt[15];

	scull_dev = kzalloc(sizeof(struct scull_dev), GFP_KERNEL);
	if (!scull_dev)
		goto err;

	scull_dev->quantum = scull_quantum;
	scull_dev->qset = scull_qset;
	sema_init(&scull_dev->sem, 1);

	ret = alloc_chrdev_region(&dev, 0, scull_count, "scull");
	if (ret) {
		pr_err("Could not allocate major/minor: %d\n", ret);
		goto free_dev;
	}
	scull_major = MAJOR(dev);

	cdev_init(&scull_dev->cdev, &scull_fops);
	scull_dev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&scull_dev->cdev, MKDEV(scull_major, 0), scull_count);
	if (ret) {
		pr_notice("Error %d adding scull0\n", ret);
		goto unregister;
	}

	if (scull_proc_init())
		goto proc_err;

	pr_info("%s load success. dev=%s, qset=%d and quantum=%d\n", __func__
		, format_dev_t(devt_fmt, dev), scull_qset, scull_quantum);
	return 0;

proc_err:
	scull_proc_exit();
unregister:
	cdev_del(&scull_dev->cdev);
	unregister_chrdev_region(dev, scull_count);
free_dev:
	kfree(scull_dev);
err:
	return ret;
}

static void __exit scull_exit(void)
{
	scull_proc_exit();
	cdev_del(&scull_dev->cdev);
	unregister_chrdev_region(MKDEV(scull_major, 0), scull_count);
	pr_info("%s called\n", __func__);
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_INFO(About, "Just a 'hello world' module");
MODULE_AUTHOR("Marcos Paulo de Souza <marcos.souza.org@gmail.com>");
MODULE_ALIAS("scull");
