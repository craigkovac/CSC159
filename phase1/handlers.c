// handlers.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"
#include "tools.h"
#include "proc.h"
#include "handlers.h"


unsigned static int tick_count = 0;
// to create process, 1st alloc PID, PCB, and process stack space
// build process frame, initialize PCB, record PID to run_q (if not 0)
void NewProcHandler(func_ptr_t *p) {  // arg: where process code starts
   int pid;

   printf("\r\nNewProcHandler Entry");
   
   if(ready_q.size == 0)  
   {
      cons_printf("Kernel Panic: cannot create more process!\n");
      return;                   // alternative: breakpoint() into GDB
   }

   printf("\r\nNewProcHandler DeQ");

   pid = DeQ(&ready_q);
   MyBzero((char *)&pcb[pid], sizeof(pcb_t));
   pcb[pid].state = RUN;
   EnQ(pid,&run_q);

   pcb[pid].proc_frame_p = (proc_frame_t *)proc_stack[pid];
   pcb[pid].proc_frame_p->EFL = EF_DEFAULT_VALUE | EF_INTR;
   pcb[pid].proc_frame_p->EIP =(int)p;
   pcb[pid].proc_frame_p->CS = get_cs();
}

// count run_time of running process and preempt it if reaching time slice
void TimerHandler(void) {
   tick_count++;   
   
   if(tick_count%75 ==0)
   {
      cons_printf(". ");
   }
   
   outportb(0x20, 0x60); // dismiss timer event (IRQ0)


   if (run_pid == 0){
      return;
   }
   pcb[run_pid].run_time++;
   if(pcb[run_pid].run_time == TIME_SLICE){
      EnQ(run_pid, &run_q);
      run_pid = -1;
   }
}
