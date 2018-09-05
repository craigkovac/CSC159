// main.c, 159
// OS bootstrap and kernel code for OS phase 1
//
// Team Name: TheQuacken  (Members: Craig Kovac and Christopher Martin)

#include "spede.h"      // given SPEDE stuff
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve
#include "tools.h"      // small functions for handlers
#include "proc.h"       // process names such as SystemProc()
#include "handlers.h"   // handler code
#include "syscalls.h"   //kill all humans

// kernel data are all declared here:
int run_pid;            // currently running PID; if -1, none selected
q_t ready_q, run_q;     // processes ready to be created and runables
pcb_t pcb[PROC_NUM];    // Process Control Blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
int timer_ticks;
mutex_t mutex;
int pies;


void ProcScheduler(void) {              // choose run_pid to load/run
   
   //printf("run_pid : %i  ",run_pid);
   if(run_pid>0)
   {
      return; // no need if PID is a user proc
   }

   if(run_q.size==0)
   {
      run_pid = 0;
   } 
   else
   {
      //may need to perform queue dequeue function
      run_pid = DeQ(&run_q);
   }
   
   pcb[run_pid].life_time += pcb[run_pid].run_time; //<-increments? ++?
   //accumulate its life_time by adding its run_time
   pcb[run_pid].run_time = 0;
   //and then reset its run_time to zero
}



int main(void) {  // OS bootstraps
   int i;
   struct i386_gate *IDT_p; // DRAM location where IDT is
   timer_ticks = 0;
   
   run_pid = -1; // needs to find a runable PID

   pies = 0;
   mutex.wait_q.size = 0;
   mutex.lock = UNLOCK;
   MyBzero((char *)&mutex.wait_q,Q_SIZE);
 //  use your tool function MyBzero to clear the two queues
   MyBzero((char *)&ready_q, Q_SIZE);
   MyBzero((char *)&run_q,Q_SIZE);//making it 20 because of below enqueue line

   //create enqueue function?
   for(i=0;i<PROC_NUM;i++)
   { 
      EnQ(i, &ready_q);
   }
   //enqueue 0~19 to ready_q (all PID's are ready)


   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n",IDT_p);

   fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[SYSCALL_EVENT], (int)SyscallEvent, get_cs(), ACC_INTR_GATE, 0);
   outportb(0x21, ~1);
   
  
//   fill out IDT entry #32 like in the timer lab exercise look up
//   set the PIC mask to open up for timer event signal (IRQ0) only
   //printf("\r\nmain");
   NewProcHandler(SystemProc);// to create the 1st process
   ProcScheduler(); // to select the run_pid
   //printf("\r\nrun_pid:%d", run_pid); 
   ProcLoader(pcb[run_pid].proc_frame_p);// with the proc_frame_p of the selected run_pid
   //check that line^ it is probably wrong
   
   return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(proc_frame_t *proc_frame_p) {   // kernel code runs (100 times/second)
   char key;
   
   pcb[run_pid].proc_frame_p = proc_frame_p;
   //save the proc_frame_p to the PCB of run_pid
   //printf("%d ",(int)&proc_frame_p);
   //call the timer even handler routine to handle the timer interrupt event
   if( proc_frame_p->event_type == TIMER_EVENT )
   {
      TimerHandler();
   }
   else if( proc_frame_p->event_type == SYSCALL_EVENT )
   {
      switch(pcb[run_pid].proc_frame_p->EAX )
      {
         case GETPID:
            GetPidHandler();
         break;

         case WRITE:
            WriteHandler();
         break;

         case SLEEP:
            SleepHandler();
         break;
         
         case MUTEX:
            if(pcb[run_pid].proc_frame_p->ECX)
            {
               MutexUnlockHandler();
            }else
            {
               MutexLockHandler();
            }
         break;
      }
   }  

   if (cons_kbhit())
   {
      key = cons_getchar();
      if(key == 'n')
      {
         NewProcHandler(UserProc);
      }else if(key == 'b')
      {
         breakpoint();
      }else if(key == 'c')
      {
         NewProcHandler(CookerProc);
      } else if(key == 'e')
      {
         NewProcHandler(EaterProc);
      }
   }
   //arintf("                \r");
   //cons_printf("fuck this!!!\n");
   ProcScheduler();// to select run_pid (if needed)
   //printf("h\n");
   ProcLoader(pcb[run_pid].proc_frame_p);//given the proc_frame_p of the run_pid
}
// |


