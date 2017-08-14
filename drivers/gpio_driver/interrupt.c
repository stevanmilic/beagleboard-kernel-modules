#include "interrupt.h"

struct InterruptInfo interrupts[NUMBER_OF_INTERRUPTS];

irq_handler_t  dev_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	int pid, rc;
	unsigned int id;

	printk(KERN_INFO "BBGPIO: IRQ received: %d\n", irq);

	pid = interrupts[irq].signal_pid;
	id = interrupts[irq].int_id;

	rc = send_signal(pid, id);
	if(rc) {
		return IRQ_NONE;
	}

	return (irq_handler_t) IRQ_HANDLED;
}

ssize_t send_signal(int signal_pid, int int_id)
{
	struct siginfo info;
	struct task_struct *t;

	memset(&info, 0, sizeof(struct siginfo));

	info.si_signo = SIGIO;
	info.si_int = int_id;
	info.si_code = SI_QUEUE;

	printk(KERN_INFO "BBGPIO: Searching for task id: %d\n", signal_pid);

	rcu_read_lock();
	t = pid_task(find_vpid(signal_pid), PIDTYPE_PID);
	rcu_read_unlock();

	if (t == NULL) {
		printk(KERN_INFO "BBGPIO: No such pid, cannot send signal\n");
		return -ENODEV;
	}

	printk(KERN_INFO "BBGPIO: Found the task, sending signal");
	send_sig_info(SIGIO, &info, t);
	return 0;
}

EXPORT_SYMBOL(interrupts);
EXPORT_SYMBOL(dev_irq_handler);
