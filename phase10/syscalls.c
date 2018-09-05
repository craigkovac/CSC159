//syscalls.c
//API calls to kernel system servi
//GetPID() call
#include "types.h"
int GetPid(void) {          // no input, has return
   int pid;

   asm("   
        movl $100, %%eax;   
        int $128;           
        movl %%eax, %0;"
       : "=g" (pid)         // output syntax, for output argument
       :                    // no input items
       :"eax"
    );

   return pid;
}

// Write() call
void Write(int fileno, char *p) {
   asm(" 
         movl $4, %%eax; 
         movl %0, %%ebx;
         movl %1, %%ecx;
         int $128;"
      :
      : "g" (fileno), "g"((int)p)
      : "eax", "ebx", "ecx"
   );
} 
         
    // save registers that will be used here
    //   send in service #, fileno, and p via
    //   three suitable registers
    //   issue syscall
    //   recover those saved registers
      //:          // no outputs, otherwise, use "=g" (...)
     // : "g" (fileno), "g" ((int)p)  // inputs, %0 and %1
    // " );


void Sleep(int number) {
      
    asm("
         movl $101, %%eax;
         movl %0, %%ebx;
         int $128;"
         :
         : "g" (number)
         :"eax", "ebx"
   );
}


void Mutex(int mutexid, int lck)
{
   asm("
         movl $102, %%eax;
         movl %0, %%ebx;
         movl %1, %%ecx;
         int $128;"
         :
         : "g" (lck), "g"(mutexid)
         : "eax", "ebx" ,"ecx"
   );
}

char GetChar(int fileno){
   int ch;
   asm("
         movl $103, %%eax;
         movl %1, %%ebx;
         int $128;
         movl %%ecx, %0;"
         : "=g" (ch)
         : "g" (fileno)
         :"eax", "ebx", "ecx"
   );
   return (char) ch;
}

void PutChar(int fileno, char ch)
{
   asm("
         movl $104, %%eax;
         movl %0, %%ebx;
         movl %1, %%ecx;
         int $128;"
         :
         : "g" (fileno), "g"((char)ch)
         : "eax", "ebx", "ecx"
   );


}

void PutStr(int fileno, char *p)
{  
   while(*p){
      PutChar(fileno,*p);
      p++;
   }
}


void GetStr(int fileno, char *p,int size)
{
   char ch;
   int i;
   for(i=0;i<size-1;i++)
   {  
      ch = GetChar(fileno);
      PutChar(fileno, ch); 
      if(ch == '\r') PutChar(fileno, '\n');

      if(ch == '\r' || ch == '\n') break;
      *p = ch;
      p++;     
   }

   *p = (char)0;
}

int Fork(void)
{
   int forkme;
   asm("
         movl $2, %%eax;
         int $128;
         movl %%ebx, %0"
         : "=g"(forkme)
         :
         :"eax", "ebx"
      );
   return forkme;
}

void Signal(int quack, func_p_t p){
   asm("
         movl $48, %%eax;
         movl %0, %%ebx;
         movl %1, %%ecx;
         int $128"
         :
         : "g" (quack), "g"((int) p)
         : "eax", "ebx", "ecx"
      );
}

void Exit(int exit_num)
{
   asm("
         movl $1, %%eax;
         movl %0, %%ebx;
         int $128"
         :
         : "g"(exit_num)
         : "eax", "ebx"
      );
}

int WaitPid(int *exit_num_p)
{
   int child_pid;

   asm("
         movl $7, %%eax;
         movl %1, %%ebx;
         int $128;
         movl %%ecx, %0"
         : "=g"(child_pid)
         : "g"(exit_num_p)
         : "eax", "ebx", "ecx"
      );
   return child_pid;
}


void Exec(func_p_t point)
{
   asm("
         movl $11, %%eax;
         movl %0, %%ebx;
         int $128;"
         :
         : "g"((int)point)
         : "eax", "ebx"
      );

}












