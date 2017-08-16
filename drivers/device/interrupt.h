#ifndef _interrupt_h_
#define _interrupt_h_

#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/wait.h>

#define NUMBER_OF_INTERRUPTS 256

static unsigned int gpio_irq_data = 0;
static unsigned int gpio_last_mask = 0;
static unsigned int interrupts[NUMBER_OF_INTERRUPTS];

static DECLARE_WAIT_QUEUE_HEAD(gpio_wait);

static irq_handler_t  dev_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

#endif
