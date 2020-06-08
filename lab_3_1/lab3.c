#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

int nPid = 0;

MODULE_LICENSE("GPL");
module_param(nPid, int, 0);


static int __init tempInit(void)
{
    char* status[] = {"unrunnable", "runnable", "stopped"};
    struct task_struct* tempTask;
    printk("Hello! and the pid is %d\n", nPid);
    for_each_process(tempTask)
    {
        if(tempTask->pid == nPid)
        {
            struct task_struct* childTask = container_of(tempTask->children.next, struct task_struct, sibling);
            struct task_struct* sibTask = container_of(tempTask->parent->children.next, struct task_struct, sibling);
            struct task_struct* parentTask = tempTask->parent;
            list_for_each_entry_from(childTask, (void*)((char*)tempTask + offsetof(struct task_struct, children)), sibling)
            {
                printk("children pid:%d name:%s status:%s\n",\
                    childTask->pid, \
                    childTask->comm, \
                    status[childTask->state>1?(2):(childTask->state + 1)]
                );
            }
            list_for_each_entry_from(sibTask, (void*)((char*)tempTask->parent + offsetof(struct task_struct, children)), sibling)
            {
                if(sibTask == tempTask)
                {
                    continue;
                }
                printk("sib pid:%d name:%s status:%s\n",\
                    sibTask->pid, \
                    sibTask->comm, \
                    status[sibTask->state>1?(2):(sibTask->state + 1)]
                );
            }
            printk("parents pid:%d name:%s status:%s\n",\
                    parentTask->pid, \
                    parentTask->comm, \
                    status[parentTask->state>1?(2):(parentTask->state + 1)]
            );
            return 0;
        }
    }
    printk(KERN_ALERT"No such process!!!\n");
    return -1;
}

static void __exit tempFini(void)
{
    printk(KERN_ALERT"ByeBye!\n");
}

module_init(tempInit);
module_exit(tempFini);