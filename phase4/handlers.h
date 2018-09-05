// handlers.h, 159

#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include "types.h"   // need definition of 'func_p_t' below

#define func_ptr_t void 

void NewProcHandler(func_ptr_t *);
void TimerHandler(void);
void GetPidHandler(void);
void WriteHandler();
void SleepHandler(void);
void MutexLockHandler(void);
void MutexUnlockHandler(void);

#endif
