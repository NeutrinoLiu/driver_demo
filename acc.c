/*
    neutrino 2019 WICIL
    acc driver
    organized as a linux kernel module
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h> 
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <asm/io.h>


// physical addr
#define ACC_PHY_BASE_ADDR_1 	0x60040000
#define ACC_PHY_ADDR_RANGE_1  	0x10000
#define ACC_PHY_BASE_ADDR_2		0x60050000
#define ACC_PHY_ADDR_RANGE_2	0x10000

// device major number
#define   ACC_MAJOR 361
int acc_major = ACC_MAJOR;

// cmds
#define  READ_LITE0	0x01
#define WRITE_LITE0 0x02
#define  READ_LITE1 0x03
#define WRITE_LITE1 0x04

struct io_access {
    int index;
    uint64_t value;
};     


struct acc_device {
    struct cdev cdev;
    volatile uint64_t *vaddr_base_1, *vaddr_base_2;
};                               // pointers to the control registers

struct acc_device *acc_device_inst; 
                                // an instance device

// ----- file operations provided by driver ---------------------------------------------
static int acc_open(struct inode *inode, struct file *file_p)
{
    file_p->private_data = acc_device_inst;
    return 0; 
}

static int acc_release(struct inode *inode, struct file *file_p)
{
    return 0;
}

static ssize_t acc_read(struct file *file_p, char __user *buf, size_t size, loff_t *ppos)
{
    printk(KERN_NOTICE "[Acc Driver] use ioctl instead, please\n");
    return 0;
}

static ssize_t acc_write(struct file *file_p, const char __user *buf, size_t size, loff_t *ppos)
{
    printk(KERN_NOTICE "[Acc Driver] use ioctl instead, please\n");
    return 0;
}                         

long int acc_ioctl(struct file *file_p, unsigned int cmd, unsigned long arg) // arg is a pointer in usigned long
{
	struct acc_device *dev = file_p->private_data;
   	struct io_access *access_unit;

   	access_unit = kzalloc(sizeof(struct io_access), GFP_KERNEL); // add check point here
   	if (copy_from_user(access_unit, (char __user *) arg, sizeof(struct io_access))) {
   		printk(KERN_NOTICE "[Acc Driver] user mem read fail\n");
   		return -1;
   	}
   	
	switch (cmd) {

		case READ_LITE0:
			access_unit->value = *(dev->vaddr_base_1 + access_unit->index);
			if (copy_to_user((char __user *) arg, access_unit, sizeof(struct io_access))) {
   				printk(KERN_NOTICE "[Acc Driver] user mem read fail\n");
   				return -EFAULT;
   			}
			break;

		case WRITE_LITE0:
			*(dev->vaddr_base_1 + access_unit->index) = access_unit->value;
			break;

		case READ_LITE1:
			access_unit->value = *(dev->vaddr_base_2 + access_unit->index);
			if (copy_to_user((char __user *) arg, access_unit, sizeof(struct io_access))) {
   				printk(KERN_NOTICE "[Acc Driver] user mem read fail\n");
   				return -EFAULT;
   			}
			break;

		case WRITE_LITE1:
			*(dev->vaddr_base_2 + access_unit->index) = access_unit->value;
			break;

		default:
			printk(KERN_NOTICE "[Acc Driver] invalid cmd\n");
   			return -EFAULT;
	}
	
	kfree(access_unit);
	return 0;
}

static const struct file_operations acc_fops = {
    .owner = THIS_MODULE,
    .read = acc_read,
    .write = acc_write,
    .unlocked_ioctl = acc_ioctl,
    .open = acc_open,
    .release = acc_release,
};

// ----- init the linux kernel module ---------------------------------------------------

static int __init acc_init(void)
{
    int ret;

    dev_t devno = MKDEV(acc_major, 0);
    ret = register_chrdev_region(devno, 1, "acc");
    
    // a - get a dev number
    if (ret < 0)
        return ret;
    
    // b - ask for kernel mem space
    acc_device_inst = kzalloc(sizeof(struct acc_device), GFP_KERNEL);
    if (!acc_device_inst) {
        printk(KERN_NOTICE "[Acc Driver] Fail to malloc memory \n");
        unregister_chrdev_region(devno, 0);
        return -ENOMEM;
    }

    // c - setup 
    cdev_init(&acc_device_inst->cdev, &acc_fops);
    acc_device_inst->cdev.owner = THIS_MODULE;
    if (cdev_add(&acc_device_inst->cdev, devno, 1)) {
        printk(KERN_NOTICE "[Acc Driver] Fail to add char device acc\n");
        kfree(acc_device_inst);
        unregister_chrdev_region(MKDEV(acc_major, 0), 1);
        return -EINVAL;
    }

    // d - mem map to virtual addr
    uint64_t *vaddr_1 = ioremap(ACC_PHY_BASE_ADDR_1, ACC_PHY_ADDR_RANGE_1);
    if (vaddr_1) {
        acc_device_inst->vaddr_base_1  = vaddr_1;
        printk(" - vaddr_base @ 0x%x \n", (uint32_t) acc_device_inst->vaddr_base_1);
    } else {
        printk("[Acc Driver] Fail to map base physical addr 1 into virtual addr \n");
        cdev_del(&acc_device_inst->cdev);
        kfree(acc_device_inst);
        unregister_chrdev_region(MKDEV(acc_major, 0), 1);
        return -EINVAL;
    }

    uint64_t *vaddr_2 = ioremap(ACC_PHY_BASE_ADDR_2, ACC_PHY_ADDR_RANGE_2);
    if (vaddr_2) {
        acc_device_inst->vaddr_base_2  = vaddr_2;
        printk(" - vaddr_base @ 0x%x \n", (uint32_t) acc_device_inst->vaddr_base_2);
    } else {
        printk("[Acc Driver] Fail to map base physical addr 2 into virtual addr \n");
        cdev_del(&acc_device_inst->cdev);
        kfree(acc_device_inst);
        unregister_chrdev_region(MKDEV(acc_major, 0), 1);
        return -EINVAL;
    }
    
    return 0;
}
module_init(acc_init);

static void __exit acc_exit(void)
{
    iounmap(acc_device_inst->vaddr_base_1);
    iounmap(acc_device_inst->vaddr_base_2);
    cdev_del(&acc_device_inst->cdev);
    kfree(acc_device_inst);
    unregister_chrdev_region(MKDEV(acc_major, 0), 1);
}
module_exit(acc_exit);

MODULE_AUTHOR("WICIL.Neutrino");
MODULE_DESCRIPTION("Acc Driver");
MODULE_LICENSE("GPL v2");