// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"      // cons_xxx below needs
#include "data.h"       // current_pid needed below
#include "proc.h"       // prototypes of processes
#include "syscalls.h"

void SystemProc(void) {
   while(1)
   {
      asm("inb $0x80");
      //printf("goodbye");
   }
   //printf("hi");
}


void UserProc(void) {
   char my_str[] = "  ";
   while(1)
   {
      //printf("hello");
      my_str[0] =GetPid() + '0';
      Write(STDOUT, my_str);
      //cons_printf("%d",run_pid);
      Sleep( GetPid() % 5 );
      //while(1){
      //   asm("inb $0x80");
     // }
   }
}
