// events.S, initial handling sequence of an event, build TF mainly

#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#include "events.h"                 // for K_DATA, K_STACK_SIZE below

// set CPU DS and ES registers to K_DATA (data segment # registers)
#define SET_K_SEGS movw $(K_DATA), %ax; mov %ax, %ds; mov %ax, %es

.comm k_stack, K_STACK_SIZE, 1      // declare kernel stack space (global)
.text

// loads the trapframe of the selected process in order to run it
ENTRY(Loader)
   // copy what's above stack pointer to eax
   movl 4(%esp), %eax
   // copy eax to esp (point esp to TF)
   movl %eax, %esp
   // 1st pop gs (see TF_t in types.h)
   popl %gs 
   // then fs
   popl %fs
   // then es
   popl %es
   // then ds
   popl %ds
   // pop all: e-d/s-i, e-b/s-p, e-b/d/c/a-x
   popa
   // skip 4 bytes (event_num)
   addl $4, %esp
   // interrupt return (pop eip, cs, eflags)
   iret


// push event_num then go into kernel
ENTRY(TimerEvent)           // push eflags, cs, eip (by circuit)
   // push event_num
   pushl $TIMER_EVENT
   jmp CommonPart

ENTRY(SleepEvent)
   pushl $SLEEP_EVENT
   jmp CommonPart

ENTRY(GetPidEvent)
   pushl $GETPID_EVENT
   jmp CommonPart

ENTRY(SemAllocEvent)
   pushl $SEMALLOC_EVENT
   jmp CommonPart
ENTRY(SemWaitEvent)
   pushl $SEMWAIT_EVENT
   jmp CommonPart
ENTRY(SemPostEvent)
   pushl $SEMPOST_EVENT
   jmp CommonPart
ENTRY(SysPrintEvent)
   pushl $SYSPRINT_EVENT
   jmp CommonPart

ENTRY(PortEvent)
   pushl $PORT_EVENT
   jmp CommonPart

ENTRY(PortAllocEvent)
   pushl $PORTALLOC_EVENT
   jmp CommonPart

ENTRY(PortWriteEvent)
   pushl $PORTWRITE_EVENT
   jmp CommonPart

ENTRY(PortReadEvent)
   pushl $PORTREAD_EVENT
   jmp CommonPart

ENTRY(FSfindEvent)
   pushl $FSFIND_EVENT
   jmp CommonPart
ENTRY(FSopenEvent)
   pushl $FSOPEN_EVENT
   jmp CommonPart
ENTRY(FSreadEvent)
   pushl $FSREAD_EVENT
   jmp CommonPart
ENTRY(FScloseEvent)
   pushl $FSCLOSE_EVENT
   jmp CommonPart

ENTRY(ForkEvent)
   pushl $FORK_EVENT
   jmp CommonPart

ENTRY(WaitEvent)
   pushl $WAIT_EVENT
   jmp CommonPart

ENTRY(ExitEvent)
   pushl $EXIT_EVENT
   jmp CommonPart
CommonPart:
// push all: e-a/c/d/b-x, e-s/b-p, e-s/d-i
   pusha
   // push 4 data seg registers
   pushl %ds
   pushl %es
   pushl %fs
   pushl %gs
   // copy TF addr (esp) to ecx
   movl %esp, %ecx
   // clear direction flag bit
   cld
   SET_K_SEGS               // set kernel data segment registers
   leal k_stack + K_STACK_SIZE, %esp  // point esp to kernel stack
   // push ecx (TF addr) there
   pushl %ecx
   call CNAME(Kernel)

