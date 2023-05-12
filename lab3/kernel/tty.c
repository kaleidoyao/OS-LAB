
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty() {
	TTY* p_tty;

	init_keyboard();
	mode = NORMAL_MODE;  // 初始化为普通模式

	for(p_tty=TTY_FIRST; p_tty<TTY_END; p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);

	int time = get_ticks();
	while(1) {
		for(p_tty=TTY_FIRST; p_tty<TTY_END; p_tty++) {  // 使用循环处理每一个tty的读和写操作
			if(mode == NORMAL_MODE) {
				if(get_ticks() - time > 100 * HZ) {  // 约为20s
					for(p_tty=TTY_FIRST; p_tty<TTY_END; p_tty++) {
						clean_screen(p_tty->p_console);
					}
					time = get_ticks();
				}
			}
			else {
				time = get_ticks();  // 防止退出查找模式时瞬间清屏
			}
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}


/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key) {
    char output[2] = {'\0', '\0'};
	int raw_code = key & MASK_RAW;

	if(mode == MASK_MODE) {  // 屏蔽模式下
		if(raw_code == ESC) {
			mode = NORMAL_MODE;
			exit_search(p_tty->p_console);
		}
		return;
	}

    if(!(key & FLAG_EXT)) {  // 表明当前字符是一个可打印字符
		if((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) {  // 判断是不是control + z
			char ch = key;
			if(ch == 'z' || ch == 'Z') {
				int prev_mode = mode;
				mode = UNDO_MODE;           // 切换到撤回模式
				undo(p_tty->p_console);
				mode = prev_mode;           // 切换回先前模式
			}
		}
		else put_key(p_tty, key);
    }
    else {
        int raw_code = key & MASK_RAW;
        switch(raw_code) {
			case ESC:
				if(mode == NORMAL_MODE) {
					mode = SEARCH_MODE;
					p_tty->p_console->search_pos = p_tty->p_console->cursor;
					p_tty->p_console->search_pos_idx = p_tty->p_console->pos_stack.index;        // 记录搜索开始时的光标栈index
					p_tty->p_console->search_ch_idx = p_tty->p_console->ch_stack.index;          // 记录搜索开始时的字符栈index  
					p_tty->p_console->search_action_idx = p_tty->p_console->action_stack.index;  // 记录搜索开始时的操作栈index
				}
				else if(mode == SEARCH_MODE) {
					mode = NORMAL_MODE;
					exit_search(p_tty->p_console);
				}
				break;
			case TAB:
				put_key(p_tty, '\t');
				break;
            case ENTER:
				if(mode == NORMAL_MODE) {
					put_key(p_tty, '\n');
				}
				else if(mode == SEARCH_MODE) {
					mode = MASK_MODE;
					search(p_tty->p_console);
				}
				break;
            case BACKSPACE:
				put_key(p_tty, '\b');
				break;
            case UP:
                if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_DN);
                }
				break;
			case DOWN:
				if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_UP);
				}
				break;
			case F1:
			case F2:
			case F3:
			case F4:
			case F5:
			case F6:
			case F7:
			case F8:
			case F9:
			case F10:
			case F11:
			case F12:
				/* Alt + F1~F12 */
				if((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
					select_console(raw_code - F1);
				}
				break;
            default:
                break;
        }
    }
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key) {  // 将key写入TTY的缓冲区
	if(p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}

