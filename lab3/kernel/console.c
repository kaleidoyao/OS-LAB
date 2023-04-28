
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PUBLIC  void clean_screen(CONSOLE* p_con);
PRIVATE void push_pos(CONSOLE* p_con, int pos);
PRIVATE int  pop_pos(CONSOLE* p_con);
PRIVATE void push_ch(CONSOLE* p_con, char ch);
PRIVATE char pop_ch(CONSOLE* p_con);
PRIVATE void push_action(CONSOLE* p_con, char action, char ch);
PRIVATE char pop_action(CONSOLE* p_con);
PUBLIC  void exit_search(CONSOLE* p_con);

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty) {
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	clean_screen(p_tty->p_console);
	p_tty->p_console->pos_stack.index = 0;
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch) {
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
		case '\t':
			if(p_con->cursor < p_con->original_addr + p_con->v_mem_limit - TAB_WIDTH) {
				for(int i=0; i<TAB_WIDTH; i++) {
					*p_vmem++ = ' ';
					*p_vmem++ = DEFAULT_CHAR_COLOR;
				}
				push_pos(p_con, p_con->cursor);
				p_con->cursor += TAB_WIDTH;  // 调整光标位置
			}
			break;
		case '\n':
			if(p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) {
				push_pos(p_con, p_con->cursor);
				p_con->cursor = p_con->original_addr + SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
			}
			break;
		case '\b':
		{
			if(p_con->cursor > p_con->original_addr) {
				int prev_pos = p_con->cursor;    // 原先光标的位置
				p_con->cursor = pop_pos(p_con);  // 删除后光标位置
				for(int i=1; i<prev_pos-p_con->cursor+1; i++) { // 使用空格填充
					*(p_vmem-2*i) = ' ';
					*(p_vmem-2*i+1) = DEFAULT_CHAR_COLOR;
				}
				char ch = pop_ch(p_con);
				if(mode != UNDO_MODE) push_action(p_con, ch, ch);
			}
			break;
		}
		default:
			if(p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {
				*p_vmem++ = ch;
				if(mode == NORMAL_MODE) *p_vmem++ = DEFAULT_CHAR_COLOR;
				else if(mode == SEARCH_MODE) *p_vmem++ = RED_CHAR_COLOR;
				push_pos(p_con, p_con->cursor);
				push_ch(p_con, ch);
				push_action(p_con, '\b', '\b');
				p_con->cursor++;
			}
			break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con) {
    set_cursor(p_con->cursor);
    set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}


/*======================================================================*
			   clean_screen
 *======================================================================*/
PUBLIC void clean_screen(CONSOLE* p_con) {
	u8 *p_vmem = (u8 *)(V_MEM_BASE + p_con->original_addr * 2);
	for(int i=p_con->original_addr; i<p_con->cursor; i++) {
		*p_vmem++ = ' ';
		*p_vmem++ = DEFAULT_CHAR_COLOR;
	}
	// 当前光标的位置 = 当前显示到的位置 = 当前显存的位置
	p_con->cursor = p_con->current_start_addr = p_con->original_addr;
	flush(p_con);
}


PRIVATE void push_pos(CONSOLE* p_con, int pos) {
	p_con->pos_stack.data[p_con->pos_stack.index++] = pos;
}

PRIVATE int pop_pos(CONSOLE* p_con) {
	return p_con->pos_stack.data[--p_con->pos_stack.index];
}

PRIVATE void push_ch(CONSOLE* p_con, char ch) {
	p_con->ch_stack.data[p_con->ch_stack.index++] = ch;
}

PRIVATE char pop_ch(CONSOLE* p_con) {
	return p_con->ch_stack.data[--p_con->ch_stack.index];
}

PRIVATE void push_action(CONSOLE* p_con, char action, char ch) {
	p_con->action_stack.data[p_con->action_stack.index] = action;
	p_con->action_stack.ch[p_con->action_stack.index] = ch;
	p_con->action_stack.index++;
}

PRIVATE char pop_action(CONSOLE* p_con) {
	return p_con->action_stack.data[--p_con->action_stack.index];
}


/*======================================================================*
			   					search
 *======================================================================*/
PUBLIC void search(CONSOLE* p_con) {
	int key_len = p_con->cursor - p_con->search_pos;
	int begin, end;
	for(int i=p_con->original_addr*2; i<p_con->search_pos*2-key_len; i+=2) {
		int found = 1;
		begin = end = i;
		for(int j=p_con->search_pos*2; j<p_con->cursor*2; j+=2) {
			if(*(u8*)(V_MEM_BASE + end) == *(u8*)(V_MEM_BASE + j)) {
				end += 2;
			}
			else {
				found = 0;
				break;
			}
		}
		if(found) {
			for(int i=begin; i<end; i+=2) {
				*(u8*)(V_MEM_BASE + i + 1) = RED_CHAR_COLOR;
			}
		}
	}
}

PUBLIC void exit_search(CONSOLE* p_con) {
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->original_addr * 2);
    // 删除关键词
    for(int i=p_con->search_pos*2; i<p_con->cursor*2; i+=2) { 
        *(u8*)(V_MEM_BASE + i) = ' ';
        *(u8*)(V_MEM_BASE + i + 1) = DEFAULT_CHAR_COLOR;
    }
    // 文本恢复白色
    for(int i=p_con->original_addr*2; i<p_con->search_pos*2; i+=2) {
        *(u8*)(V_MEM_BASE + i + 1) = DEFAULT_CHAR_COLOR;
    }
    // 更新光标位置
    p_con->cursor = p_con->search_pos;
    flush(p_con);
}


/*======================================================================*
			   					undo
 *======================================================================*/
PUBLIC void undo(CONSOLE* p_con) {
	if(p_con->action_stack.index == 0) return;
	char action = pop_action(p_con);
	out_char(p_con, action);
	// switch(action) {
	// 	case '\b':
	// 		out_char(p_con, );
	// 		break;
	// 	default:
	// 		out_char(p_con, '\b');
	// 		break;
	// }
}