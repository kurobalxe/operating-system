#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

static int a = 0, b = 0, flag = 0;

static ssize_t sumdev_read(struct file *file, char __user *buf,
                           size_t lbuf, loff_t *ppos)
{
    int sum = a + b;
    char msg[32];
    int len;
    int ret;

    printk(KERN_INFO "sumdev_read called, a=%d, b=%d, flag=%d\n", a, b, flag);

    if (*ppos > 0) {
        return 0; // EOF
    }

    if (flag == 0 && a == 0 && b == 0) {
        len = snprintf(msg, sizeof(msg), "No data yet\n");
    } else if (flag == 0 && b == 0) {
        // 只有一个数
        len = snprintf(msg, sizeof(msg), "Only one number: %d\n", a);
    } else {
        len = snprintf(msg, sizeof(msg), "Sum = %d (a=%d, b=%d)\n", sum, a, b);
    }

    if (copy_to_user(buf, msg, len)) {
        return -EFAULT;
    }

    *ppos = len;
    return len;
}

static ssize_t sumdev_write(struct file *file, const char __user *buf,
                            size_t count, loff_t *f_pos)
{
    char kbuf[32];
    int num;

    if (count >= sizeof(kbuf)) {
        return -EFAULT;
    }

    if (copy_from_user(kbuf, buf, count)) {
        return -EFAULT;
    }

    kbuf[count] = '\0';
    
    if (kstrtoint(kbuf, 10, &num)) {
        printk(KERN_ERR "sumdev_write: invalid integer input: %s\n", kbuf);
        return -EINVAL;
    }

    if (flag == 0) {
        a = num;
        printk(KERN_INFO "sumdev_write: wrote %d to a\n", num);
    } else {
        b = num;
        printk(KERN_INFO "sumdev_write: wrote %d to b\n", num);
    }
    flag = !flag;

    return count;
}

static int sumdev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "sumdev_open called\n");
    return 0;
}

static int sumdev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "sumdev_release called\n");
    return 0;
}

static struct file_operations sumdev_fops = {
    .owner = THIS_MODULE,
    .open = sumdev_open,
    .release = sumdev_release,
    .read = sumdev_read,
    .write = sumdev_write,
};

static struct miscdevice sumdev_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "sumdev",
    .fops = &sumdev_fops,
    .mode = 0666,
};

static int __init sumdev_init(void)
{
    int ret;
    
    ret = misc_register(&sumdev_misc_device);
    if (ret) {
        printk(KERN_ERR "sumdev: failed to register misc device\n");
        return ret;
    }
    
    printk(KERN_INFO "sumdev driver loaded, device: /dev/sumdev\n");
    return 0;
}

static void __exit sumdev_exit(void)
{
    misc_deregister(&sumdev_misc_device);
    printk(KERN_INFO "sumdev driver unloaded\n");
}

module_init(sumdev_init);
module_exit(sumdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("Simple sum device driver");
MODULE_VERSION("1.0");
