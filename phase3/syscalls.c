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
         



void Mutex(int bob)
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
         : "g" (bob));   
}




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
