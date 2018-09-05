//syscalls.c
//API calls to kernel system servi
//GetPID() call
int GetPid(void) {          // no input, has return
   int pid;

   asm("pushl %%EAX;        
        movl $100, %%EAX;   
        int $128;           
        movl %%EAX, %0;     
        popl %%EAX"         // restore original EAX
       : "=g" (pid)         // output syntax, for output argument
       :                    // no input items
    );

   return pid;
}

// Write() call
void Write(int fileno, char *p) {
   asm(" pushl %%EAX;
         pushl %%EBX;
         pushl %%ECX;
         movl $4, %%EAX; 
         movl %0, %%EBX;
         movl %1, %%ECX;
         int $128;
         popl %%ECX;
         popl %%EBX;
         popl %%EAX"
      :
      : "g" (fileno), "g"((int)p)
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
         pushl %%eax;
         pushl %%ebx;
         movl $101, %%eax;
         movl %0, %%ebx;
         int $128;
         popl %%ebx;
         popl %%eax"
         :
         : "g" (number));
}


void Mutex(int lck)
{
   asm("
         pushl %%eax;
         pushl %%ebx;
         movl $102, %%eax;
         movl %0, %%ebx;
         int $128;
         popl %%ebx;
         popl %%eax"
         :
         : "g" (lck));
}

char GetChar(int fileno){
   int ch;
   asm("
         pushl %%eax;
         pushl %%ebx;
         pushl %%ecx;
         movl $103, %%eax;
         movl %1, %%ebx;
         int $128;
         movl %%ecx, %0;
         popl %%ecx;
         popl %%ebx;
         popl %%eax"
         : "=g" (ch)
         : "g" (fileno));
   return (char) ch;
}

void PutChar(int fileno, char ch)
{
   asm("
         pushl %%eax;
         pushl %%ebx;
         pushl %%ecx;
         movl $104, %%eax;
         movl %0, %%ebx;
         movl %1, %%ecx;
         int $128;
         popl %%ecx;
         popl %%ebx;
         popl %%eax"
         :
         : "g" (fileno), "g"((char)ch));


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
   int i;
   for(i = 0;i<size;i++)
   {
      
      *p = GetChar(fileno);
      if((*p) == ((char) 13))
      {
         break;
      }else if((*p) == ((char) 10))
      {
         break;
      }
      p++;
   }
   
   p[i+1] = '\0';
}


















