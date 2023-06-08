
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

char* int2str(int num, char* str, int radix) {
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";  // 索引表
    unsigned unum;    // 存放要转换的整数的绝对值，转换的整数可能是负数
    int i = 0, j, k;  // i用来指示设置字符串相应位，转换之后i其实就是字符串的长度；转换后顺序是逆序的，有正负的情况，k用来指示调整顺序的开始位置；j用来指示调整顺序时的交换
 
    // 获取要转换的整数的绝对值
    if(radix == 10 && num < 0) {  // 要转换成十进制数并且是负数
        unum = (unsigned) -num;   // 将num的绝对值赋给unum
        str[i++] = '-';           // 在字符串最前面设置为'-'号，并且索引加1
    }
    else unum = (unsigned) num;   // 若是num为正，直接赋值给unum
 
    // 转换部分，注意转换后是逆序的
    do {
        str[i++] = index[unum % (unsigned)radix];  // 取unum的最后一位，并设置为str对应位，指示索引加1
        unum /= radix;  // unum去掉最后一位
 
    }while(unum);  // 直至unum为0退出循环
 
    str[i] = '\0'; // 在字符串最后添加'\0'字符，c语言字符串以'\0'结束
 
    // 将顺序调整过来
    if(str[0] == '-') k = 1;  // 如果是负数，符号不用调整，从符号后面开始调整
    else k = 0;               // 不是负数，全部都要调整
 
    char temp;  // 临时变量，交换两个值时用到
    for(j=k; j<=(i-1)/2; j++) {  // 头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
        temp = str[j];           // 头部赋值给临时变量
        str[j] = str[i-1+k-j];   // 尾部赋值给头部
        str[i-1+k-j] = temp;     // 将临时变量的值(其实就是之前的头部值)赋给尾部
    }
 
    return str;  // 返回转换后的字符串
}

void init_main() {
    // 初始化进程表
    for(int i=0; i<NR_TASKS; i++) {
		proc_table[i].ticks = 1;
	}

    // 初始化信号量
    product1.value = 0;
    product2.value = 0;

    // 初始化仓库
    for(int i=0; i<CAPACITY; i++) {
        w[i] = 0;
    }
    putptr = 0;
    getptr = 0;
}

void clean_screen() {  // 清屏
	disp_pos = 0;
	for(int i=0; i<80*25; i++) {
		disp_str(" ");
	}
	disp_pos = 0;
}


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

    init_main(); // 初始化

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	clean_screen();
	restart();

	while(1){}
}

void producing() {
    // 生产货物
    milli_delay(TIME_SLICE);
}

void consuming() {
    // 消费货物
    milli_delay(TIME_SLICE);
}

void produce1() {
    producing();     // 生产货物
    p(&warehouse);

    p(&mutex);       // 将货物放入仓库
    w[putptr] = 1;
    putptr = (putptr + 1) % CAPACITY;
    p_proc_ready->total++;
    v(&mutex);

    v(&product1);
}

void consume1() {
    p(&product1);

    p(&mutex);       // 从仓库取出货物
    w[getptr] = 0;
    getptr = (getptr + 1) % CAPACITY;
    p_proc_ready->total++;
    v(&mutex);

    v(&warehouse);
    consuming();     // 消费货物
}

void produce2() {
    producing();     // 生产货物
    p(&warehouse);

    p(&mutex);       // 将货物放入仓库
    w[putptr] = 2;
    putptr = (putptr + 1) % CAPACITY;
    p_proc_ready->total++;
    v(&mutex);

    v(&product2);
}

void consume2() {
    p(&product2);

    p(&mutex);       // 从仓库取出货物
    w[getptr] = 0;
    getptr = (getptr + 1) % CAPACITY;
    p_proc_ready->total++;
    v(&mutex);

    v(&warehouse);
    consuming();     // 消费货物
}

produce_function produce_funcs[2] = {produce1, produce2};
consume_function consume_funcs[2] = {consume1, consume2};

void NormalA() {
    int sequence = 0;
    char string[5] = {'\0'};
    while(++sequence <= 20) {
        int2str(sequence / 10, string, 10);  // 打印序列号
        my_print(string, TEXT_DEFAULT);
        int2str(sequence % 10, string, 10);
        my_print(string, TEXT_DEFAULT);
        my_print(": ", TEXT_DEFAULT);
        for(int i=1; i<NR_TASKS; i++) {
            int num = (proc_table + i)->total;
            int2str(num, string, 10);
            my_print(string, TEXT_DEFAULT);
            my_print(" ", TEXT_DEFAULT);
        }
        my_print("\n", TEXT_DEFAULT);
        my_sleep(TIME_SLICE);
    }
    while(1);
}

void ProducerB() {
    while(1) {
        produce_funcs[0]();
    }
}

void ProducerC() {
    while(1) {
        produce_funcs[1]();
    }
}

void ConsumerD() {
    while(1) {
        consume_funcs[0]();
    }
}

void ConsumerE() {
    while(1) {
        consume_funcs[1]();
    }
}

void ConsumerF() {
    while(1) {
        consume_funcs[1]();
    }
}
