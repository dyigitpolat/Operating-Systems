#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h> 
#include <linux/moduleparam.h>

static int param_pid = 0;

/* This function is called when the module is loaded. */
int simple_init(void)
{
	struct task_struct *cur;

	#define next_task(p) \
	list_entry_rcu((p)->tasks.next, struct task_struct, tasks)

	#define for_each_process(p) \
	for (p = &init_task ; (p = next_task(p)) != &init_task ; )

	rcu_read_lock();                                                    
	for_each_process(cur) {                                             
		task_lock(cur);                                             
		if((int) task_pid_nr(cur) == param_pid){
			printk(KERN_INFO "Loading Module\n");
			printk("ID: %d\n", (int) task_pid_nr(cur));
			printk("NAME: %s\n", cur->comm);
			printk("GROUP: %d\n", (int) task_tgid_nr(cur));
			printk("PRIO: %ld\n", (long int) cur->prio);
			printk("VID: %d\n", (int) task_pid_vnr(cur));
			printk("TTY %d\n", cur->signal->tty);
			printk("------------\n------------\n");
		}
		task_unlock(cur);                                           
	}                                                                    
	rcu_read_unlock();       

	return 0; //leave.
}

/* This function is called when the module is removed. */
void simple_exit(void) {
	printk(KERN_INFO "Removing Module, Bye\n");
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );
module_param( param_pid, int, 0);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("yigit");
