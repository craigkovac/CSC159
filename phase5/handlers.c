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
      //cons_printf("%s", pcb[run_pid].proc_frame_p->ECX);
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
   if(port == TERM1)
   {
      i = 0;
      outportb(0x20,0x63);
   }else
   {
      i=1;
      outportb(0x20,0x64);
   }
   ch =  inportb(port+DATA);
   //printf(" %i%c ",i, ch);
   //breakpoint();
   if(terminal_wait_queue[i].size==0)
   {
     
      EnQ(ch,&terminal_buffer[i]);
   } else
   { 
      //this i think is fine o its not do it again
      //printf(" 3 ");
      temppid = DeQ(&terminal_wait_queue[i]);
      pcb[ temppid ].state = RUN;
      EnQ(ch,&terminal_buffer[i]);
      EnQ(temppid,&run_q);
      pcb[temppid].proc_frame_p->ECX = DeQ(&terminal_buffer[i]);
   }

}


void GetCharHandler(int fileno)
{
   int i;
   char bob;
   //int ran;

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

   if(terminal_buffer[i].size == 0)
   {
     // printf(" 5 ");
      //looks right
      EnQ(run_pid,&terminal_wait_queue[i]);
      pcb[run_pid].state = WAIT;
      run_pid = -1;

      //may be wrong
      //bob = DeQ(&terminal_buffer[i]);
      //pcb[run_pid].proc_frame_p->ECX = bob;
   }else
   {
      //printf(" 6 ");
      //also looks right
      
      bob = DeQ(&terminal_buffer[i]);
      pcb[run_pid].proc_frame_p->ECX = bob;
   }
}
