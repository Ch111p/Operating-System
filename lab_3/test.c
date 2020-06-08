#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

void printAllProcess(void)
{ 
    struct task_struct* tempTask;
    int i = 0;
    for_each_process(tempTask)
    {
        if(!tempTask->mm)
        {
            printk("pid:%d prio:%d ppid:%d name:%s\n", tempTask->pid, \
            tempTask->prio, \
            tempTask->parent->pid, \
            tempTask->comm);
            i++;
        }
    }
    printk("Sum num is %d\n", i);
}

static int __init hello_init(void)
{
    printk(KERN_ALERT"hello, world!!\n");
    printAllProcess();
    return 0;
}

static void __exit temp_exit(void)
{
    printk(KERN_ALERT"Bye!!\n");
}

module_init(hello_init);
module_exit(temp_exit);