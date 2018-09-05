// main.c, 159
// this is kernel code for phase 8
//
// Team Name: offby1 (Members: Brandon Byrne)
// Notes: Couldn't track down "Somehow numbers 2 and 1 got queued into the write_q of the port data(and got sent to the terminal as non-ASCII symbols), more like PID #'s got queued in correctly somewhere (potentially fatal) -1"
//
// I did a 'grep "write_q" *' and traced backwards to the PortWrite() function, nowhere is PID getting queued into write_q
// 
// It's coming from the MyMemcpy's on line 398 of FSReadHandler.  I gave up trying to figure it out after that
//
//



#include "spede.h"      // given SPEDE stuff
#include "handlers.h"   // handler code
#include "tools.h"      // small functions for handlers
#include "proc.h"       // processes such as Init()
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve
#include "FSdata.h"


// B
// kernel's own data:
int current_pid;        // current selected PID; if 0, none selected
q_t ready_q, free_q;    // processes ready to run and not used
pcb_t pcb[PROC_NUM];    // process control blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
unsigned int current_time;
sem_t sem[Q_SIZE];
unsigned short *ch_p = (unsigned short *) 0xB8000;
port_t port[PORT_NUM];
mem_page_t mem_page[MEM_PAGE_NUM];
int kernel_MMU;

void Scheduler() {                 // choose a PID as current_pid to load/run
   if (current_pid != 0) return; // if continue below, find one for current_pid

   if (ready_q.size == 0) {
      cons_printf( "Kernel Panic: no process to run!\n"); // big problem!
      breakpoint();
   }

   //get next ready-to-run process as current_pid
   current_pid = DeQ(&ready_q);

   //update its state
   pcb[current_pid].state = RUN;
   pcb[current_pid].cpu_time = 0;
   ch_p[current_pid*80+43] = 0xf00 + 'R';
}

// OS bootstrap from main() which is process 0, so we do not use this PID
int main() {
   int i;
   struct i386_gate *IDT_p; 
   current_time = 0;
   kernel_MMU = get_cr3();
   //Zero data structures out
   for (i=0;i < PROC_NUM; i++) {
   MyBzero((char *)ready_q.q[i], sizeof(q_t));
   MyBzero((char *)free_q.q[i], sizeof(q_t));
   sem[i].owner = 0;
   pcb[i].MMU = 0;
   }  
   MyBzero((char*)&port[0], sizeof(port_t) * PORT_NUM);
   MyBzero((char *)&sem[0], sizeof(sem_t) * Q_SIZE);
	
   for (i=0;i<MEM_PAGE_NUM;i++)
   {
      mem_page[i].owner = 0;
      mem_page[i].addr = (char *)(MEM_BASE + MEM_PAGE_SIZE * i);		
   }

   for (i=1;i<20;i++)
   {
      EnQ(i, &free_q);
   }

   for(i=0;i<FD_NUM-1;i++)
   {
    fd_array[i].owner = 0;
   }

   root_dir[0].size = sizeof(root_dir);
   bin_dir[0].size = sizeof(bin_dir);
   bin_dir[1].size = root_dir[0].size;
   www_dir[0].size = sizeof(www_dir);
   www_dir[1].size = root_dir[0].size;


   //Load IDT
   IDT_p = get_idt_base();
   cons_printf("IDT located at DRAM addr %x (%d).\n", IDT_p, IDT_p);
   cons_printf("************************************\n");
   fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[SLEEP_EVENT], (int)SleepEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[GETPID_EVENT], (int)GetPidEvent, get_cs(), ACC_INTR_GATE, 0);

   fill_gate(&IDT_p[SEMALLOC_EVENT], (int)SemAllocEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[SEMWAIT_EVENT], (int)SemWaitEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[SEMPOST_EVENT], (int)SemPostEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[SYSPRINT_EVENT], (int)SysPrintEvent, get_cs(), ACC_INTR_GATE, 0);

   fill_gate(&IDT_p[PORT_EVENT], (int)PortEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[PORT_EVENT+1], (int)PortEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[PORTALLOC_EVENT], (int)PortAllocEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[PORTWRITE_EVENT], (int)PortWriteEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[PORTREAD_EVENT], (int)PortReadEvent, get_cs(), ACC_INTR_GATE, 0);

   fill_gate(&IDT_p[FSOPEN_EVENT], (int)FSopenEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[FSREAD_EVENT], (int)FSreadEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[FSCLOSE_EVENT], (int)FScloseEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[FSFIND_EVENT], (int)FSfindEvent, get_cs(), ACC_INTR_GATE, 0);
   
   fill_gate(&IDT_p[FORK_EVENT], (int)ForkEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[WAIT_EVENT], (int)WaitEvent, get_cs(), ACC_INTR_GATE, 0);
   fill_gate(&IDT_p[EXIT_EVENT], (int)ExitEvent, get_cs(), ACC_INTR_GATE, 0);



   //Set Timer and COM interrupts
   outportb(0x21, 0xE6);
   
   //Initialize processes
   NewProcHandler(Init);
   NewProcHandler(TermProc);
   NewProcHandler(TermProc);
   Scheduler();
  // set_cr3(pcb[current_pid].MMU);
   Loader(pcb[current_pid].TF_p);

   return 0;
}

void Kernel(TF_t *TF_p) {  

   pcb[current_pid].TF_p = TF_p;
   

   switch (TF_p->event_num) {
      case TIMER_EVENT:
         TimerHandler();
         break;
      case SLEEP_EVENT:
         SleepHandler();
         break;
      case GETPID_EVENT:
         GetPidHandler();
         break;
      case SEMALLOC_EVENT:
        SemAllocHandler(pcb[current_pid].TF_p->eax);
         break;
      case SEMWAIT_EVENT:
        SemWaitHandler(pcb[current_pid].TF_p->eax);
        break;
      case SEMPOST_EVENT:
        SemPostHandler(pcb[current_pid].TF_p->eax);
        break;
      case SYSPRINT_EVENT:
	      SysPrintHandler((char *)pcb[current_pid].TF_p->eax);
        break;
      case PORT_EVENT:
        PortHandler();
        break;
      case PORTALLOC_EVENT:
        PortAllocHandler(&TF_p->eax);
        break;
      case PORTWRITE_EVENT:
        PortWriteHandler((char)TF_p->eax, TF_p->ebx);
        break;
      case PORTREAD_EVENT:
        PortReadHandler((char *)TF_p->eax, TF_p->ebx);
        break;
      case FSFIND_EVENT:
        FSfindHandler();
        break;
      case FSOPEN_EVENT:
        FSopenHandler();
        break;
      case FSREAD_EVENT:
        FSreadHandler();
        break;
      case FSCLOSE_EVENT:
        FScloseHandler();
        break;
      case FORK_EVENT:
	      ForkHandler((char *)TF_p->eax, &TF_p->ebx);
	      break;
      case WAIT_EVENT:
	      WaitHandler(&TF_p->eax);
	      break;
      case EXIT_EVENT: 
	      ExitHandler(TF_p->eax);
	      break;
      default:
         cons_printf("Kernel Panic: unknown event_num %d!\n", TF_p->event_num);
         breakpoint();
   }


   Scheduler();
   if(pcb[current_pid].MMU != 0)
   {
    // cons_printf("Setting PID %d with MMU %x\n", current_pid, pcb[current_pid].MMU);

   }

   set_cr3(pcb[current_pid].MMU);
   Loader(pcb[current_pid].TF_p);
}

