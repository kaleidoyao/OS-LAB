
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE	 (80 * 25)
#define SCREEN_WIDTH  80
#define TAB_WIDTH     4

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */
#define RED_CHAR_COLOR		0x04	/* 0000 0100 黑底红字 */

/* 记录光标位置 */
typedef struct cursor_pos_stack {
    int index;
    int data[SCREEN_SIZE];
}POSSTACK;

/* 记录字符栈 */
typedef struct ch_stack {
	int index;
	char data[SCREEN_SIZE];
}CHSTACK;

/* 记录操作栈 */
typedef struct action_stack {
	int index;
	char data[SCREEN_SIZE];
}ACTIONSTACK;

/* CONSOLE */
typedef struct s_console {
	unsigned int current_start_addr;  /* 当前显示到了什么位置 */
	unsigned int original_addr;	      /* 当前控制台对应显存位置 */
	unsigned int v_mem_limit;         /* 当前控制台占的显存大小 */
	unsigned int cursor;              /* 当前光标位置 */
	unsigned int search_pos;          /* 搜索开始的位置 */
	int search_pos_idx;               /* 搜索开始时的位置栈index */
	int search_ch_idx;                /* 搜索开始时的字符栈index */
	int search_action_idx;            /* 搜索开始时的操作栈index */
	POSSTACK pos_stack;               /* 光标位置栈 */
	CHSTACK ch_stack;                 /* 字符栈 */
	ACTIONSTACK action_stack;         /* 操作栈 */
}CONSOLE;

#endif /* _ORANGES_CONSOLE_H_ */
