
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

PUBLIC	TASK	task_table[NR_TASKS] = {
    {NormalA,   STACK_SIZE_NORMALA,   "NormalA"},
	{ReaderB,   STACK_SIZE_READERB,   "ReaderB"},
	{ReaderC,   STACK_SIZE_READERC,   "ReaderC"},
	{ReaderD,   STACK_SIZE_READERD,   "ReaderD"},
	{WriterE,   STACK_SIZE_WRITERE,   "WriterE"},
	{WriterF,   STACK_SIZE_WRITERF,   "WriterF"},
};

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks,
													   sys_my_sleep,
													   sys_my_print,
													   sys_p,
													   sys_v,
													  };

PUBLIC SEMAPHORE rw_mutex = {1};
PUBLIC SEMAPHORE reader_mutex = {1};
PUBLIC SEMAPHORE writer_mutex = {1};
PUBLIC SEMAPHORE reader_allow_mutex = {1};
PUBLIC SEMAPHORE reader_count_mutex = {MAX_READERS};
