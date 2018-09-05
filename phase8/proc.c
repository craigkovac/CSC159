// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"      // cons_xxx below needs
#include "data.h"       // current_pid needed below
#include "proc.h"       // prototypes of processes
#include "syscalls.h"
#include "tools.h"

void SystemProc(void) {
   while(1)
   {
      asm("inb $0x80");
      //printf("goodbye");
   }
   //printf("hi");
}

void ShellProc(void) {
   int term, my_pid, forked_pid;
   char my_str[] = " ",
        my_msg[] = ": TheQuacken Shell> ",
        get_str[100];
   
   Signal(SIGINT, (int)Ouch);   

   while(1) {
      my_pid = GetPid();
      my_str[0] = my_pid + '0';               // id str
      term = (my_pid%2 == 1)? TERM1 : TERM2;  // which term to use

      PutStr(term, my_str);
      PutStr(term, my_msg);
      GetStr(term, get_str, 100); // syscall will add null
      if(MyStrcmp(get_str, "fork") == 1) { // 1 mean they're the same
         forked_pid = Fork();
         if(forked_pid == -1)
            PutStr(term, "ShellProc: cannot fork!\n\r");
      }
   }
}

void Wrapper(func_p_t sig_handler){
   asm("pusha");        //save regs
   sig_handler();       //call user's handler
   asm("popa");         //pop back regs
}

void Ouch(void){
   int term = (GetPid()%2)? TERM1: TERM2;          //which terminal?
   PutStr(term, "Ouch, that stings, Cowboy! ");    //just a message
}
