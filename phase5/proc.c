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
   char hello[] = ":Hello World : ";
   char this[] = " ";
   char death[] = "\r\n"; 
   //my_str[0] = GetPid();
   my_str[0] =GetPid() + '0';
   while(1)
   {
      if(run_pid%2==1)
      {
         Write(TERM1, my_str);
         Write(TERM1, hello);
         //printf("here1");
         this[0] = GetChar(TERM1);
         //printf("%c\r\n",this);
         Write(TERM1, this);
         Write(TERM1, death);
         //add \r\n 
      }
      else
      {
         Write(TERM2, my_str);
         Write(TERM2, hello);
         //printf("here");
         this[0] = GetChar(TERM2);
       //  printf("2%c", this);
         Write(TERM2, this);
         Write(TERM2, death);
         //write term 2?
         //odd pid
      }
      
      //Write(STDOUT, my_str);
      Sleep( GetPid() % 5 );
      //while(1){
      //   asm("inb $0x80");
     // }
   }
}



void CookerProc(void) {   // will be created by pressing 'c' key
   int i;
   for(;;) {
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
   for(;;) {
      Mutex(LOCK);
      if(pies == 0){
         cons_printf("----- Eater %i: no pie to eat!\n", GetPid());
      } else {
         cons_printf("Eater %i: eating pie # %i...\n", GetPid(), pies--);
         for(i=0; i<LOOP; i++) asm("inb $0x80");
      }
      Mutex(UNLOCK);
   }
}


// |

