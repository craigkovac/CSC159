// events.h of events.S
// prototypes those coded in events.S

#ifndef __EVENTS_H__
#define __EVENTS_H__

// #include <spede/machine/pic.h> // shouldn't need this anymore

#define K_CODE 0x08         // kernel code segment # (register)
#define K_DATA 0x10         // kernel data segment # (register)
#define K_STACK_SIZE 16384  // kernel runtime stack byte size

#define TIMER_EVENT 32      // IDT entry #32, aka IRQ0, for timer device
#define GETPID_EVENT 100
#define SLEEP_EVENT 101

#define SEMWAIT_EVENT 103
#define SEMALLOC_EVENT 102
#define SEMPOST_EVENT 104
#define SYSPRINT_EVENT 105

#define PORT_EVENT 35
#define PORTALLOC_EVENT 106
#define PORTWRITE_EVENT 107
#define PORTREAD_EVENT 108
#define FSFIND_EVENT 109
#define FSOPEN_EVENT 110
#define FSREAD_EVENT 111
#define FSCLOSE_EVENT 112

#define FORK_EVENT 113
#define WAIT_EVENT 114
#define EXIT_EVENT 115

#ifndef ASSEMBLER  // skip below if ASSEMBLER defined (from an assembly code)
                   // since below is not in assembler syntax
__BEGIN_DECLS

#include "types.h"          // for 'TF_t' below

void TimerEvent(void);      // coded in events.S, assembler won't like this syntax
void Loader(TF_t *);        // coded in events.S
void GetPidEvent(void);
void SleepEvent(void);
void SemAllocEvent(void);
void SemWaitEvent(void);
void SemPostEvent(void);
void SysPrintEvent(void);
void PortEvent(void);
void PortAllocEvent(void);
void PortReadEvent(void);
void PortWriteEvent(void);
void FSfindEvent(void);
void FSopenEvent(void);
void FSreadEvent(void);
void FScloseEvent(void);
void ForkEvent(void);
void WaitEvent(void);
void ExitEvent(void);
__END_DECLS

#endif // ifndef ASSEMBLER

#endif // ifndef __EVENTS_H__

