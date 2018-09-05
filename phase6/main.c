// main.c, 159
// OS bootstrap and kernel code for OS phase 4
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
q_t terminal_buffer[2], terminal_wait_queue[2];


void InitTerms(int port)
{
   int i;

   // set baud, Control Format Control Register 7-E-1 (data- parity-stop bits)
   // raise DTR, RTS of the serial port to start read/write
   // outportb(TERM1 + BAUDLO, LOBYTE(115200/9600)); // period of each of 9600 bauds
   outportb(port + CFCR, CFCR_DLAB);             // CFCR_DLAB is 0x80
   outportb(port + BAUDLO, LOBYTE(115200/9600));
   outportb(port + BAUDHI, HIBYTE(115200/9600));
   outportb(port + CFCR, CFCR_PEVEN | CFCR_PENAB | CFCR_7BITS);

   outportb(port + IER, 0);
   outportb(port + MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   outportb(port + IER, IER_ERXRDY|IER_ETXRDY);

   for(i=0;i<LOOP;i++) asm("inb $0x80");   

   // terminal H/W reset time
   /*

   // repeat above code for the other terminal
   outportb(TERM2 + BAUDHI, HIBYTE(115200/9600));
   outportb(TERM2 + CFCR, CFCR_DLAB);             // CFCR_DLAB is 0x80
   outportb(TERM2 + CFCR, CFCR_PEVEN | CFCR_PENAB | CFCR_7BITS);
   for(i=0;i<LOOP;i++) asm("inb $0x80");   
   */  
}

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
   MyBzero((char *)&mutex.wait_q,sizeof(mutex_t));
   //  use your tool function MyBzero to clear the two queues
   MyBzero((char *)&ready_q, Q_SIZE);
   MyBzero((char *)&run_q,Q_SIZE);//making it 20 because of below enqueue line

   MyBzero((char *)&terminal_wait_queue[0],Q_SIZE);
   MyBzero((char *)&terminal_wait_queue[1],Q_SIZE);
   MyBzero((char *)&terminal_buffer[0], Q_SIZE);
   MyBzero((char *)&terminal_buffer[1], Q_SIZE);

   //create enqueue function?
   for(i=0;i<PROC_NUM;i++)
   { 
      EnQ(i, &ready_q);
   }
   //enqueue 0~19 to ready_q (all PID's are ready)

   InitTerms(TERM1);
   InitTerms(TERM2);

   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n",IDT_p);

   fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[SYSCALL_EVENT], (int)SyscallEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[TERM1_EVENT], (int)Term1Event, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[TERM2_EVENT], (int)Term2Event, get_cs(), ACC_INTR_GATE, 0);
   outportb(0x21, ~0x19);


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
            if(pcb[run_pid].proc_frame_p->EBX)
            {
               MutexUnlockHandler();
            }else
            {
               MutexLockHandler();
            }
            break;

         case GETCHAR:
            
            GetCharHandler();
         break;

         case PUTCHAR:
            PutCharHandler();
         break;
      }
   }else if (proc_frame_p->event_type == TERM1_EVENT)
   {
      TerminalHandler(TERM1);
   }else if (proc_frame_p->event_type == TERM2_EVENT)
   {
      TerminalHandler(TERM2);
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

   ProcScheduler();// to select run_pid (if needed)
   //printf("h\n");
   ProcLoader(pcb[run_pid].proc_frame_p);//given the proc_frame_p of the run_pid
}
