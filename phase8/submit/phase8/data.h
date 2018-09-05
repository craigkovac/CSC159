// data.h, 159
// kernel data are all declared in main.c during bootstrap, but
// other kernel .c code must reference them as 'extern' (already declared)

#ifndef __DATA_H__                  // 'name-mangling' prevention
#define __DATA_H__                  // 'name-mangling' prevention

#include "types.h"                  // defines q_t, pcb_t, PROC_NUM, PROC_STACK_SIZE

extern int current_pid;             // PID of current selected process to run, 0 means none
extern q_t ready_q, free_q;         // ready-to-run PID's, and un-used PID's
extern pcb_t pcb[PROC_NUM];         // 20 Process Control Blocks
extern char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // 20 process runtime stacks
extern unsigned int current_time;
extern sem_t sem[Q_SIZE];
extern unsigned short *ch_p;
extern port_t port[PORT_NUM];
extern char help_txt_data[];
extern char note_txt_date[];
extern char index_html_data[];
extern char hello_html_data[];
extern dir_t bin_dir[4];
extern dir_t www_dir[8];
extern dir_t root_dir[11];
extern fd_t fd_array[FD_NUM];
extern mem_page_t mem_page[MEM_PAGE_NUM];
extern int kernel_MMU;
#endif // ifndef __DATA_H__         ('name-mangling' prevention)
