
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

PUBLIC	PROCESS			proc_table[NR_TASKS];

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC    TASK    task_table[NR_TASKS] = {
    {NormalA,     STACK_SIZE_NORMALA,     "NormalA"},
    {ProducerB,   STACK_SIZE_PRODUCERB,   "ProducerB"},
    {ProducerC,   STACK_SIZE_PRODUCERC,   "ProducerC"},
    {ConsumerD,   STACK_SIZE_CONSUMERD,   "ConsumerD"},
    {ConsumerE,   STACK_SIZE_CONSUMERE,   "ConsumerE"},
    {ConsumerF,   STACK_SIZE_CONSUMERF,   "ConsumerF"},
};

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks,
													   sys_my_sleep,
													   sys_my_print,
													   sys_p,
													   sys_v,
													  };

PUBLIC SEMAPHORE mutex = {1};
PUBLIC SEMAPHORE product1 = {0};
PUBLIC SEMAPHORE product2 = {0};
PUBLIC SEMAPHORE warehouse = {CAPACITY};
