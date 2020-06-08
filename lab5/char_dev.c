#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/io.h>

#define NAME "CHARDEV"
#define MEMLENGTH 4096
#define SETMEMCLEAN 0
#define GETMEMBYPOINT 1
#define SETMEMBYPOINT 2

MODULE_LICENSE("GPL");

typedef struct argType
{
    int start;
    int length;
    char *mem;
}arg, *pArg;

static struct cdev my_cdev;
static int majorNumber;
static int sum = 0;
static struct device *charDevice;
static struct class *charClass;
static char mem[MEMLENGTH];

int charDev_open(struct inode *inodep, struct file *filp)
{
    sum++;
    printk("Hello, %d\n", sum);
    return 0;
}

ssize_t charDev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int ret = 0;
    if(*f_pos >= MEMLENGTH)
    {
        goto out;
    }
    if(*f_pos + count > MEMLENGTH)
    {
        count = MEMLENGTH - *f_pos;
    }
    if(copy_to_user(buf, &mem[*f_pos], count))
    {
        ret = -EFAULT;
    }
    else
    {
        *f_pos += count;
        ret = count;
        printk(KERN_INFO"read %d byte from device\n");
    }
out:
    return ret;
}

ssize_t charDev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    int ret = 0;
    printk(KERN_INFO"here!!!\n");
    if(*f_pos >= MEMLENGTH)
    {
        goto out;
    }
    if(*f_pos + count > MEMLENGTH)
    {
        count = MEMLENGTH - *f_pos;
    }
    if(copy_from_user(&mem[*f_pos], buf, count))
    {
        ret = -EFAULT;
    }
    else
    {
        *f_pos += count;
        ret = count;
        printk(KERN_INFO"write %d bytes to device\n", count);
    }
out:
    return ret;
}

int charDev_release(struct inode *inode, struct file *filp)
{
    return 0;
}

long charDev_ioctl(struct file *filp, unsigned int cmd,unsigned long arg)
{
    int ret = 0;
    if(cmd >= 3)
    {
        return -ENOTTY;
    }
    if(!access_ok(VERIFY_READ, (void __user *)arg, sizeof(struct argType)))
    {
        return -ENOTTY;
    }
    struct argType tempArg;
    copy_from_user(&tempArg, (void*)arg, sizeof(struct argType));
    switch(cmd)
    {
        case SETMEMCLEAN:
            memset(mem, 0, MEMLENGTH);
            break;
        case GETMEMBYPOINT:
            if(!access_ok(VERIFY_READ, tempArg.mem, tempArg.length) || tempArg.start >= MEMLENGTH)
            {
                return -ENOTTY;
            }
            ret = copy_to_user(tempArg.mem, &mem[tempArg.start], (tempArg.length+tempArg.start) > MEMLENGTH?((tempArg.length+tempArg.start)%MEMLENGTH):(tempArg.length));
            break;
        case SETMEMBYPOINT:
            if(!access_ok(VERIFY_READ, tempArg.mem, tempArg.length) || (tempArg.start >= MEMLENGTH))
            {
                return -ENOTTY;
            }
            ret = copy_from_user(&mem[tempArg.start], tempArg.mem, (tempArg.length+tempArg.start) > MEMLENGTH?((tempArg.length+tempArg.start)%MEMLENGTH):(tempArg.length));
            break;            
    }
    printk(KERN_INFO"ioctl success!!!!\n");
    return ret;
}

static void charDev_exit(void)
{
    unregister_chrdev_region(MKDEV(majorNumber, 0), 1);
    // class_destroy(charClass);
    cdev_del(&my_cdev);
}

static struct file_operations fops = \
{
    .owner = THIS_MODULE,
    .open = charDev_open,
    .read = charDev_read,
    .write = charDev_write,
    .release = charDev_release,
    .unlocked_ioctl = charDev_ioctl,
};

int __init charDev_init(void)
{
    printk(KERN_INFO"now the game is start!\n");
    int result = alloc_chrdev_region(&majorNumber, 0, 1, NAME);
    if (result < 0)
    {
        printk(KERN_INFO"REGISTER_CHRDEV FAILED!\n");
        return majorNumber;
    }
    printk(KERN_INFO"register number is %d %d\n", MAJOR(majorNumber), MINOR(majorNumber));
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    cdev_add(&my_cdev, majorNumber, 1);
    printk("cdev_add successed!\n");
    // charClass = class_create(THIS_MODULE, NAME);
    // charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, "char_device");
    // if(IS_ERR(charDevice))
    // {
    //     unregister_chrdev_region(MKDEV(majorNumber, 0), 2);
    //     class_destroy(charClass);
    //     printk(KERN_INFO"device_create FAILED!\n");
    //     return charDevice;
    // }
    return 0;
}

module_init(charDev_init);
module_exit(charDev_exit);