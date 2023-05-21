
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule() {
    PROCESS* p;
    
    while(1) {
        p_proc_ready++;
        if(p_proc_ready >= proc_table + NR_TASKS) {
            p_proc_ready = proc_table;
        }
        if(p_proc_ready->is_blocked == FALSE && p_proc_ready->sleep_time == 0) {
            break;
        }
    }
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC void sys_my_sleep(int milli_seconds) {
    int ticks = milli_seconds / 1000 * HZ;
    p_proc_ready->sleep_time = ticks;
    schedule();
}

PUBLIC void sys_my_print(char* s, int color) {
    // 打印字符
    disp_color_str(s, color);

    // 移动光标
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, ((disp_pos / 2) >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, (disp_pos / 2) & 0xFF);
    enable_int();
}

PUBLIC void sys_p(SEMAPHORE *s) {
    disable_int();
    s->value--;
    if(s->value < 0) {
        s->queue[-s->value - 1] = p_proc_ready;  // 放入队列
        p_proc_ready->is_blocked = TRUE;         // 阻塞
        schedule();
    }
    enable_int();
}

PUBLIC void sys_v(SEMAPHORE *s) {
    disable_int();
    s->value++;
    if(s->value <= 0) {
        s->queue[0]->is_blocked = FALSE;  // 唤醒进程
        for(int i=0; i<-s->value; i++) {
            s->queue[i] = s->queue[i+1];  // 队列前移
        }
    }
    enable_int();
}
