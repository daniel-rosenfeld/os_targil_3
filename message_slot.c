// Declare what kind of code we want
// from the header files. Defining __KERNEL__
// and MODULE allows us to access kernel-level
// code not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE
#define MAX_NUM_OF_SLOTS 256
#define MY_MAJOR 235
#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "simple_message_slot"
#define SUCCESS 0
#define MESSAGE_LEN 128

#include <linux/kernel.h>  /* We're doing kernel work */
#include <linux/module.h>  /* Specifically, a module */
#include <linux/fs.h>      /* for register_chrdev */
#include <linux/uaccess.h> /* for get_user and put_user */
#include <linux/string.h>  /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>
#include "message_slot.h"

MODULE_LICENSE("GPL");

struct message
{
    char *content;
    size_t size;
    struct message *next;
    int channel;
};

struct message_slot
{
    struct message *my_message;
};

struct message *open_channel(int channel_num);
struct message *is_valid(struct file *file);

static struct message_slot slots[MAX_NUM_OF_SLOTS];

//================== DEVICE FUNCTIONS ===========================
static int device_open(struct inode *inode,
                       struct file *file)
{
    file->private_data = (void *)0;
    return 0;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read(struct file *file,
                           char __user *buffer,
                           size_t length,
                           loff_t *offset)
{
    struct message *relevant_channel = is_valid(file);
    int i;

    if (relevant_channel == NULL)
    {
        return -EWOULDBLOCK;
    }

    if (relevant_channel->size > length)
    {
        return -ENOSPC;
    }

    for (i = 0; i < length && i < MESSAGE_LEN; i++)
    {
        put_user(relevant_channel->content[i], &buffer[i]);
    }

    return relevant_channel->size;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write(struct file *file,
                            const char __user *buffer,
                            size_t length,
                            loff_t *offset)
{
    struct message *relevant_channel = is_valid(file);
    int i;

    printk("valid\n");

    if (relevant_channel == NULL)
    {
        return -EINVAL;
    }

    if (MESSAGE_LEN < length || length <= 0)
    {
        return -EMSGSIZE;
    }
    printk("going to free and malloc\n");
    if (relevant_channel->content != NULL)
    {
        kfree(relevant_channel->content);
        relevant_channel->content = kmalloc(length, GFP_KERNEL);
        printk("freed and malloced\n");
    }
    else{
        printk("didn't need to\n");
    }

    printk("going to get user for length\n");
    for (i = 0; i < length && i < MESSAGE_LEN; i++)
    {
        get_user(relevant_channel->content[i], &buffer[i]);
    }
    printk("got user for length\n");

    relevant_channel->size = length;
    return length;
}

//----------------------------------------------------------------
static long device_ioctl(struct file *file,
                         unsigned int ioctl_command_id,
                         unsigned long ioctl_param)
{
    bool exists;
    if ((MSG_SLOT_CHANNEL != ioctl_command_id) || (ioctl_param == 0))
    {
        return -EINVAL;
    }

    exists = false;
    struct message *relevant_channel = slots[iminor(file->f_inode)].my_message;

    if (relevant_channel == NULL)
    {
        slots[iminor(file->f_inode)].my_message = open_channel(ioctl_param);
        file->private_data = (void *)ioctl_param;
        return SUCCESS;
    }

    while (relevant_channel->next != NULL)
    {
        if (relevant_channel->next->channel == ioctl_param)
        {
            exists = true;
            break;
        }
        relevant_channel = relevant_channel->next;
    }

    if (exists)
    {
        file->private_data = (void *)ioctl_param;
        return SUCCESS;
    }

    relevant_channel->next = open_channel(ioctl_param);
    file->private_data = (void *)ioctl_param;
    return SUCCESS;
}

struct message *open_channel(int channel_num)
{
    struct message *new_channel = kmalloc(sizeof(struct message), GFP_KERNEL);
    new_channel->channel = channel_num;
    new_channel->content = NULL;
    new_channel->size = 0;
    new_channel->next = NULL;
    return new_channel;
}

struct message *is_valid(struct file *file)
{
    struct message *relevant_channel = slots[iminor(file->f_inode)].my_message;

    if (relevant_channel == NULL)
    {
        return NULL;
    }

    while (relevant_channel != NULL)
    {
        if (relevant_channel->channel == (int)file->private_data)
        {
            break;
        }
        relevant_channel = relevant_channel->next;
    }

    return relevant_channel;
}

//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .unlocked_ioctl = device_ioctl,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void)
{
    int rc = -1;
    int i;

    // Register driver capabilities. Obtain major num
    rc = register_chrdev(MY_MAJOR, DEVICE_RANGE_NAME, &Fops);

    // Negative values signify an error
    if (rc < 0)
    {
        printk(KERN_ALERT "%s registraion failed for  %d\n",
               DEVICE_FILE_NAME, MY_MAJOR);
        return rc;
    }

    for (i = 0; i < MAX_NUM_OF_SLOTS; i++)
    {
        slots[i].my_message = NULL;
    }

    return 0;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void)
{
    struct message *relevant_channel;
    struct message *next;
    int i;
    for (i = 0; i < MAX_NUM_OF_SLOTS; i++)
    {
        relevant_channel = slots[i].my_message;
        next = relevant_channel;
        while (next != NULL)
        {
            relevant_channel = next;
            if (relevant_channel->content != NULL)
                kfree(relevant_channel->content);
            next = relevant_channel->next;
            kfree(relevant_channel);
        }
    }

    // Unregister the device
    // Should always succeed
    unregister_chrdev(MY_MAJOR, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================
