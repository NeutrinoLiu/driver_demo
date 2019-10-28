/*
    neutrino 2019 WICIL
    accelerator driver
    organized as a linux kernel module
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h> 
#include <linux/uaccess.h>
#include <asm/io.h>


// physical addr
#define ACC_PHY_BASE_ADDR 	0x60040000
#define ACC_PHY_ADDR_RANGE  0x200

#define REG_START 	(0ul) // read and write // set to one automatically 
#define REG_DONE	(1ul) // read and write
#define REG_MODE	(2ul) // read and write
#define REG_ADDR	(3ul) // read and write
#define REG_WD0		(4ul) // read and write 
#define REG_WD1		(5ul) 
#define REG_WD2		(6ul)
#define REG_WD3		(7ul)
#define REG_RD0     (8ul) //read only
#define REG_RD1     (9ul)
#define REG_RD2     (10ul)
#define REG_RD3     (11ul)

// unit defines
#define UNIT_NULL        0x00    //write type: reserved for test
#define UNIT_RESET       0x01    //write type: reset
#define UNIT_SET         0x02    //write type: send write commands
#define UNIT_GET	     0x03    //write type: send read commands
#define UNIT_RESP        0x04    // read type: check status

// device major number
#define ACCELERATOR_MAJOR 361
int accelerator_major = ACCELERATOR_MAJOR;

struct file_access_unit {
    uint64_t addr;
    uint64_t data[4];
    uint64_t parameter;
    uint8_t  type;
};                              // not used currently, will be used for the real acc 

struct accelerator_device {
    struct cdev cdev;
    volatile uint64_t *virtual_addr_base;
};                               // pointers to the control registers

struct accelerator_device *acc_device_inst; 
                                // an instance device

// ----- file operations provided by driver ---------------------------------------------
static int accelerator_open(struct inode *inode, struct file *file_p)
{
    file_p->private_data = acc_device_inst;
    return 0; 
}

static int accelerator_release(struct inode *inode, struct file *file_p)
{
    return 0;
}

static ssize_t accelerator_read(struct file *file_p, char __user *buf, size_t size, loff_t *ppos)
{
    int ret = 0;
    struct accelerator_device *dev = file_p->private_data;
    volatile uint64_t *reg_base = dev->virtual_addr_base;

    uint64_t i;
    struct file_access_unit fa_unit;

    
    if (size < sizeof(struct file_access_unit)) {
        ret = -EFAULT;
        printk(KERN_NOTICE "[Acc Driver] Not enough space in user buffer\n");
    }

    fa_unit.type = UNIT_RESP;
    //
    //printk(KERN_NOTICE "[Acc Driver] gona to read regs \n");
    //
    fa_unit.parameter = *(reg_base + REG_DONE);
    for (i = 0; i < 4; i++)
        fa_unit.data[i] = *(reg_base + REG_RD0 + i);
    printk(KERN_NOTICE "[Acc Driver] doing visit done - %d; data - %d, %d, %d, %d \n", fa_unit.parameter, fa_unit.data[0], fa_unit.data[1], fa_unit.data[2], fa_unit.data[3]);


    if (copy_to_user(buf, &fa_unit, size)) {
        ret = -EFAULT;
        printk(KERN_NOTICE "[Acc Driver] Fail to reply with a valid reading\n");
    }
    return ret;
}

static ssize_t accelerator_write(struct file *file_p, const char __user *buf, size_t size, loff_t *ppos)
{
    struct accelerator_device *dev = file_p->private_data;
    volatile uint64_t *reg_base = dev->virtual_addr_base;

    int i;
    struct file_access_unit fa_unit;

    // check size 
    if (size < sizeof(struct file_access_unit)) {
        printk(KERN_NOTICE "[Acc Driver] Not enough length of user access unit\n");
        return -EFAULT;
    }

    // get the cmd package from user
    if (copy_from_user(&fa_unit, buf, size)) {
        printk(KERN_NOTICE "[Acc Driver] Fail to provide a valid file write command\n");
        return -EFAULT;
    } else {
        //printk(KERN_NOTICE "[Acc Driver] receive your write request\n");
    }


    // excute cmd -------------------------------------------------------------------------- to be accomplish
    switch (fa_unit.type)
    {
        case UNIT_NULL:      // 0x00
            // doing nothing
            break;
        case UNIT_RESET:    // 0x01
            // reset the device throught your virtual addr pointer
            printk(KERN_NOTICE "[Acc Driver] doing reset\n");
        	for (i = 0; i < 8; i++)
        		*(reg_base + i) = 0;
            //printk(KERN_NOTICE "[Acc Driver] reset done\n");
            break;
        case UNIT_SET:      // 0x02
            printk(KERN_NOTICE "[Acc Driver] doing set, 0x%x, %d, %d, %d, %d\n", fa_unit.addr, fa_unit.data[0], fa_unit.data[1], fa_unit.data[2], fa_unit.data[3]);
        	*(reg_base + REG_DONE) = 0;                                
        	*(reg_base + REG_ADDR) = fa_unit.addr;                     
        	*(reg_base + REG_MODE) = 1;                     		   
        	for (i = 0; i < 4; i++)
        		*(reg_base + REG_WD0 + i) = fa_unit.data[i];           
        	*(reg_base + REG_START) = 1;                               
            //printk(KERN_NOTICE "[Acc Driver] set done\n");
            // set the device throught your virtual addr pointer
            break;
        case UNIT_GET:      // 0x03
            printk(KERN_NOTICE "[Acc Driver] doing visit, 0x%x\n", fa_unit.addr);
        	*(reg_base + REG_MODE) = 0;                                
        	*(reg_base + REG_ADDR) = fa_unit.addr;                     
        	*(reg_base + REG_DONE) = 0;                                
        	*(reg_base + REG_START) = 1;                               
            //printk(KERN_NOTICE "[Acc Driver] visit done\n");
            // set the device throught your virtual addr pointer
            break;
        default:
            printk(KERN_NOTICE "[Acc Driver] Unrecognizable write command\n");
            break;
    }

    return 0;
}

static const struct file_operations accelerator_fops = {
    .owner = THIS_MODULE,
    .read = accelerator_read,
    .write = accelerator_write,
    .open = accelerator_open,
    .release = accelerator_release,
};

// ----- init the linux kernel module ---------------------------------------------------

static int __init accelerator_init(void)
{
    int ret;

    dev_t devno = MKDEV(accelerator_major, 0);
    ret = register_chrdev_region(devno, 1, "accelerator");
    
    // a - get a dev number
    if (ret < 0)
        return ret;
    
    // b - ask for kernel mem space
    acc_device_inst = kzalloc(sizeof(struct accelerator_device), GFP_KERNEL);
    if (!acc_device_inst) {
        printk(KERN_NOTICE "[Acc Driver] Fail to malloc memory \n");

        unregister_chrdev_region(devno, 0);
        return -ENOMEM;
    }

    // c - setup 
    cdev_init(&acc_device_inst->cdev, &accelerator_fops);
    acc_device_inst->cdev.owner = THIS_MODULE;
    if (cdev_add(&acc_device_inst->cdev, devno, 1)) {
        printk(KERN_NOTICE "[Acc Driver] Fail to add char device accelerator\n");
        
        kfree(acc_device_inst);
        unregister_chrdev_region(MKDEV(accelerator_major, 0), 1);
        return -EINVAL;
    }

    // d - mem map to virtual addr
    uint64_t *virtual_base_addr = ioremap(ACC_PHY_BASE_ADDR, ACC_PHY_ADDR_RANGE);
    if (virtual_base_addr) {
        acc_device_inst->virtual_addr_base  = virtual_base_addr;
        printk(" - virtual_addr_base @ 0x%x \n", (uint32_t) acc_device_inst->virtual_addr_base);
    } else {
        printk("[Acc Driver] Fail to map physical addr into virtual addr \n");
        
        cdev_del(&acc_device_inst->cdev);
        kfree(acc_device_inst);
        unregister_chrdev_region(MKDEV(accelerator_major, 0), 1);
        return -EINVAL;
    }
    
    return 0;
}
module_init(accelerator_init);

static void __exit accelerator_exit(void)
{
    iounmap(acc_device_inst->virtual_addr_base);
    cdev_del(&acc_device_inst->cdev);
    kfree(acc_device_inst);
    unregister_chrdev_region(MKDEV(accelerator_major, 0), 1);
}
module_exit(accelerator_exit);

MODULE_AUTHOR("WICIL.Neutrino");
MODULE_DESCRIPTION("accelerator Driver");
MODULE_LICENSE("GPL v2");