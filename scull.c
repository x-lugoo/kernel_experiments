#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/semaphore.h>

struct scull_qset {
	void **data;
	struct scull_qset *next;
}

struct scull_dev {
	struct scull_qset *qset_data; /// Pointer to the first quantum set
	int quantum; /// current quantum size
	int qset; /// current array size
	unsigned long size; /// amount of data stored
	unsigned int access_key; /// used by sculluid and scullpriv
	struct semaphore sem; /// mutex semaphore
	struct cdev cdev; /// char device struct
};

int scrull_trim(struct scull_dev *dev)
{
	struct scull_qset *next, *dptr;
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

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
	int err;

	// FIXME: check errors
	dev->cdev = cdev_alloc();
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.fops = &scull_fops;
	err = cdev_add(dev->cdev, MKDEV(scull_major, scull_minor + 1));
	if (err)
		printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}

int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev; // dev info

	dev = container_of(node->i_cdev, struct scrull_dev, cdev);
	filp->private_data = dev;

	// now trim if device if open was write only
	if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
		scull_trim(dev); //FIXME: check errors
	return 0;
}



struct file_operations scull_fops = {
	.owner =   THIS_MODULE,
	.llseek =  scull_llseek,
	.read =    scull_read,
	.write =   scull_write,
	.ioctl =   scull_ioctl,
	.open =    scull_open,
	.release = scull_release,
};

