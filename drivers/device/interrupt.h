#ifndef _interrupt_h_
#define _interrupt_h_

#include <linux/interrupt.h>
#include <asm/siginfo.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>

#define NUMBER_OF_INTERRUPTS 256

struct InterruptInfo {
	unsigned int int_id;
	int signal_pid;
};

//using only 32-127, 128-238 -> External interrupts(IRQs)
extern struct InterruptInfo interrupts[NUMBER_OF_INTERRUPTS];

extern irq_handler_t  dev_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

extern ssize_t send_signal(int signal_pid, int int_id);

#endif
