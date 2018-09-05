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
       run_pid = DeQ(&run_q);
   }
   
   pcb[run_pid].life_time += pcb[run_pid].run_time;
 
   pcb[run_pid].run_time = 0;
   
   //if(pcb[run_pid].life_time == : 
}



int main(void) {  // OS bootstraps
   int i;
   struct i386_gate *IDT_p; // DRAM location where IDT is
   timer_ticks = 0;
   run_pid = -1; 
   pies = 0;
   //need to do this
   //init muz and seta ll values to zero
   //step 3
   mutex.lock = UNLOCK;//do you need to set all parts of mutex lock to UNLOCK 
   MyBzero((char *)&mutex.wait_q, sizeof(mutex_t));
   //mux queue size = 100 need to know for my Bzero
   MyBzero((char *)&ready_q, Q_SIZE);
   MyBzero((char *)&run_q,Q_SIZE);
   
   for(i=0;i<PROC_NUM;i++)
   { 
      EnQ(i, &ready_q);
   }
   


   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n",IDT_p);

   fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[SYSCALL_EVENT], (int)SyscallEvent, get_cs(), ACC_INTR_GATE, 0);
   outportb(0x21, ~1);
   
  

   NewProcHandler(SystemProc);// to create the 1st process
   ProcScheduler(); // to select the run_pid
   ProcLoader(pcb[run_pid].proc_frame_p);// with the proc_frame_p of the selected run_pid
   return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(proc_frame_t *proc_frame_p) {   // kernel code runs (100 times/second)
   char key;
   
   pcb[run_pid].proc_frame_p = proc_frame_p;
   if( proc_frame_p->event_type == TIMER_EVENT )
   {
      TimerHandler();
   }
   else if( proc_frame_p->event_type == SYSCALL_EVENT )
   {
      switch( proc_frame_p->EAX )
      {
         case GETPID:
            GetPidHandler();
         break;

         case WRITE:
            WriteHandler(*proc_frame_p);
         break;

         case SLEEP:
            SleepHandler();
         case MUTEX:
            printf("\r\nMUTEX");
            if(proc_frame_p->EBX)
            {
               printf("\r\nEBX:1");
                MutexUnlockHandler();
              //cons_printf("Quack\n");
            } else//==lock
            {
               printf("\r\nEBX:0");
               MutexLockHandler();
              //cons_printf("lawl\n");
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
      }else if(key == 'e')
      {
         NewProcHandler(EaterProc);
      }   
   }


   ProcScheduler();
 
   ProcLoader(pcb[run_pid].proc_frame_p);
}


