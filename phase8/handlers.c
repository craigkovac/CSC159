// handlers.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"
#include "tools.h"
#include "proc.h"
#include "handlers.h"
#include "syscalls.h"

//unsigned static int timer_ticks = 0;
// to create process, 1st alloc PID, PCB, and process stack space
// build process frame, initialize PCB, record PID to run_q (if not 0)
void NewProcHandler(func_ptr_t *p) {  // arg: where process code starts
  int pid;

  //printf("\r\nNewProcHandler Entry");

  if(ready_q.size == 0)  
  {
    cons_printf("Kernel Panic: cannot create more process!\n");
    return;                   // alternative: breakpoint() into GDB
  }

  //printf("\r\nNewProcHandler DeQ");

  pid = DeQ(&ready_q);
  MyBzero((char *)&pcb[pid], sizeof(pcb_t));
  MyBzero((char *)&proc_stack[pid], PROC_STACK_SIZE);
  pcb[pid].state = RUN;
  if( pid != 0)
    EnQ(pid,&run_q);
  pcb[pid].proc_frame_p = (proc_frame_t *)&proc_stack[ pid ][PROC_STACK_SIZE-sizeof(proc_frame_t)];
  pcb[pid].proc_frame_p->EFL = EF_DEFAULT_VALUE | EF_INTR;
  pcb[pid].proc_frame_p->EIP =(unsigned int)p;
  pcb[pid].proc_frame_p->CS = get_cs();

}

// count run_time of running process and preempt it if reaching time slice
void TimerHandler(void) {
  int i;
  timer_ticks++;  

  if(timer_ticks%75 ==0)
  {
    //   cons_printf(". ");
  }

  //wake needed sleeping process'
  for(i=0;i<PROC_NUM;i++)
  {
    if(pcb[i].state==SLEEPING)
    {
      if(pcb[i].wake_time == timer_ticks)
      {
        pcb[i].state=RUN;
        EnQ(i, &run_q);
      }
    }
  }




  outportb(0x20, 0x60); // dismiss timer event (IRQ0)
  

  if (run_pid == 0){
    return;
  }

  pcb[run_pid].run_time++;

  if(pcb[run_pid].run_time >= TIME_SLICE){
    EnQ(run_pid, &run_q);
    run_pid = -1;
  }

}


void GetPidHandler(void)
{
  pcb[run_pid].proc_frame_p->EAX = run_pid;
}

void WriteHandler()
{ 
  //printf here?
  int i;
  int fileno = pcb[run_pid].proc_frame_p->EBX;
  char *p = (char *)pcb[run_pid].proc_frame_p->ECX;

  //call outportb
  //inportb -> inport byte`

  switch(fileno){
    case STDOUT:
      //cons_printf(p);
      cons_printf("%s", pcb[run_pid].proc_frame_p->ECX);
      break;

    default:

//      cons_printf(p);
      while(*p)
      {
        outportb(fileno + DATA, *p);
        for(i=0;i<12000;i++)
        {
          asm("inb $0x80");
        }
        p++;
      }
      //printf("\r\nWriteHandler Error");
      break;

  }  
}

void SleepHandler(void)
{
  //set current process wait time
  pcb[run_pid].wake_time = timer_ticks + 100 * pcb[run_pid].proc_frame_p->EBX;
  //set current process as sleeping
  pcb[run_pid].state = SLEEPING; 
  //put cur proc_frame_t in eax
  //EnQ(run_pid, &run_q);  
  //change current pid
  run_pid = -1;
}

void MutexLockHandler(void)
{
  if(mutex.lock==LOCK)//locked and locked
  {
    EnQ(run_pid,&mutex.wait_q);
    pcb[run_pid].state = WAIT;
    run_pid = -1;
  }else
  {
    //EnQ(run_pid,&run_q);
    mutex.lock = LOCK;
  }

  //printf("run_pidLock: %i  ",run_pid);
  return;
}

void MutexUnlockHandler(void)
{
  int pid;
  //printf("runpidunlock1; %i\n",run_pid);
  if(mutex.wait_q.size ==0)
  {
    mutex.lock = UNLOCK;

  }else
  {  
    pid = DeQ(&mutex.wait_q);
    pcb[pid].state = RUN;
    EnQ(pid,&run_q);
    //add to run_q? or set as ready and add to ready q?

  }
  //printf("run_pidUnlock: %i\n",run_pid);
  return;
}

void TerminalHandler(int port)
{
   int i;
   char ch;
   int temppid;
   int indicator;
   if(port == TERM1)
   {
      i = 0;
      outportb(0x20,0x63);
   }else
   {
      i=1;
      outportb(0x20,0x64);
   }
   indicator = inportb(port+IIR);
   if(indicator == IIR_RXRDY)
   {
      ch =  inportb(port+DATA);
      //printf(" %i%c ",i, ch);
      //breakpoint();
      if(term_kb_wait_q[i].size==0)
      {
     
         EnQ(ch,&term_kb_wait_q[i]);
      } else
      { 
         //this i think is fine o its not do it again
        
         temppid = DeQ(&term_kb_wait_q[i]);
         pcb[ temppid ].state = RUN;
         EnQ(temppid,&run_q);
         pcb[temppid].proc_frame_p->ECX = ch;
         if((ch == (char)3) && ( pcb[temppid].sigint_handler !=0))
         {
            InsertWrapper(temppid, pcb[temppid].sigint_handler);
            printf("InsertWrapper\r\n");
            printf("here1");
         }
      }
   }else
   {
      if(term_screen_wait_q[i].size != 0)
      {
         temppid = DeQ(&term_screen_wait_q[i]);
         EnQ(temppid,&run_q);
         pcb[temppid].state = RUN;
      }
   }
}


void GetCharHandler()
{
   int i;
   char bob;
   int fileno;

   fileno = pcb[run_pid].proc_frame_p->EBX;

   if(fileno==TERM1)
   {
      i = 0;
   } else
   {
      i = 1;
   }
   //printf(" 4 ");
   //breakpoint();
   //bob = DeQ(&terminal_buffer[i]);

   //pcb[run_pid].proc_frame_p->ECX = 'a';

   if(term_kb_wait_q[i].size == 0)
   {
     // printf(" 5 ");
      //looks right
      EnQ(run_pid,&term_kb_wait_q[i]);
      pcb[run_pid].state = WAIT;
      run_pid = -1;

      //may be wrong
      //bob = DeQ(&terminal_buffer[i]);
      //pcb[run_pid].proc_frame_p->ECX = bob;
   }else
   {
      //printf(" 6 ");
      //also looks right
      
      bob = DeQ(&term_kb_wait_q[i]);
      pcb[run_pid].proc_frame_p->ECX = bob;
   }
   
}


void PutCharHandler()
{
   int fileno;
   int i;
   char ch;
   
   fileno =  pcb[run_pid].proc_frame_p->EBX;

   if(fileno == TERM1)
   {
      i = 0;
   } else
   {
      i = 1;
   }
   
   ch = pcb[run_pid].proc_frame_p->ECX;
   
   //call outportb to send ch to data register based on fileno
   //outportb(fileno+DATA,ch);
   outportb(fileno,ch);

   EnQ(run_pid,&term_screen_wait_q[i]);
   pcb[run_pid].state = WAIT;
   run_pid = -1;
}

void ForkHandler(proc_frame_t *parent_frame_p)
{
   int child_pid, delta, *bp;
   proc_frame_t *child_frame_p;
   if(ready_q.size ==0)
   {
      cons_printf("Kernel Panic: cannot create more process!\n");
      pcb[run_pid].proc_frame_p->EBX = -1;
      return;
   }
   child_pid = DeQ(&ready_q);


   //pcb[child_pid].proc_frame_p = (proc_frame_t *)&proc_stack[child_pid ][PROC_STACK_SIZE-sizeof(proc_frame_t)];


   EnQ(child_pid,&run_q);
   MyBzero((char *)&pcb[child_pid], sizeof(pcb_t));
   //MyBzero((char *)&proc_stack[child_pid], PROC_STACK_SIZE);
   pcb[child_pid].state = RUN;
   pcb[child_pid].ppid = run_pid;
   MyMemcpy((char *)&proc_stack[child_pid], (char *)&proc_stack[run_pid], PROC_STACK_SIZE);
   
   //a
   delta = proc_stack[child_pid]-proc_stack[run_pid];
   
   //printf("c1:%04x ",(int)child_frame_p);
   //printf("p2:%04x ",(int)parent_frame_p);
   //b
   (long)child_frame_p =(long)parent_frame_p + delta;
   
   //c
   child_frame_p->ESP = parent_frame_p->ESP + delta;   
   
   child_frame_p->EBP = parent_frame_p->EBP + delta;   
   
   child_frame_p->ESI = parent_frame_p->ESI + delta;   
   
   child_frame_p->EDI = parent_frame_p->EDI + delta;   

   
   
   //printf("c3:%04x ",(int)child_frame_p);
   //printf("p4:%04x ",(int)parent_frame_p);
   //d
   pcb[run_pid].proc_frame_p->EBX = child_pid;
   child_frame_p->EBX = 0;

   //e
   //breakpoint();
   
   (int) bp = child_frame_p->EBP;
   while(*bp)
   {
      *bp +=  delta;
      (int)bp = *bp;
   }
   //breakpoint();
   //pcb[run_pid].proc_frame_p = parent_frame_p;
   pcb[child_pid].proc_frame_p = child_frame_p;
   pcb[child_pid].sigint_handler = pcb[run_pid].sigint_handler;
}

void SignalHandler(proc_frame_t *proc_frame_p){
   if(proc_frame_p->EBX == SIGINT)
   {
      pcb[run_pid].sigint_handler =(func_p_t)proc_frame_p -> ECX;
   }   
}

void InsertWrapper(int pid, func_p_t handler){
   int *p;
   proc_frame_t temp_frame;

   temp_frame = *pcb[pid].proc_frame_p;

   p = (int *)&pcb[pid].proc_frame_p->EFL;
   *p = (int)handler;
   p--;
   *p = (int)temp_frame.EIP;

   pcb[pid].proc_frame_p = (proc_frame_t *)((int)pcb[pid].proc_frame_p - sizeof(int [2]));
   MyMemcpy((char *) pcb[pid].proc_frame_p,(char *)&temp_frame, sizeof(proc_frame_t));

   temp_frame = *pcb[pid].proc_frame_p;
   printf("here");
   pcb[pid].proc_frame_p->EIP=(unsigned int)Wrapper;

}
