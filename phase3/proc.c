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



void CookerProc(void) {   // will be created by pressing 'c' key
   int i;

   while(1) {
      Mutex(LOCK);
      if(pies == 99) {
         cons_printf("+++++ Cooker %i: too many pies!\n", GetPid());
      } else {
         cons_printf("Cooker %i: making pie # %i...\n", GetPid(), ++pies);
         for(i=0; i<LOOP; i++) asm("inb $0x80");      // pie-making time
      }
      Mutex(UNLOCK);
   }
}


void EaterProc(void) {   // will be created by pressing 'e' key
   int i;
   while(1)
   {
      Mutex(LOCK);
      if(pies == 0) {
         cons_printf("----- Eater %i: no pie to eat!\n", GetPid());
      } else {
         cons_printf("Eater %i: eating pie # %i...\n", GetPid(), pies--);
         for(i=0; i<LOOP; i++) asm("inb $0x80");
      }
      Mutex(UNLOCK);
   }
}











