// proc.h, 159

#ifndef _PROC_H_
#define _PROC_H_

void SystemProc(void);      // PID 0, never preempted
void ShellProc(void);
void Wrapper(func_p_t);
void Ouch(void);

#endif
