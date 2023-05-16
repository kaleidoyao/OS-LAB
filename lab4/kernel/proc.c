
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				p->ticks = p->priority;
			}
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
	disp_str("clolor");
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

PUBLIC void sys_p(char* s) {
	disp_str(s);
}

PUBLIC void sys_v() {

}
