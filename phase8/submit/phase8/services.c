// services.c, 159
// OS Services and their assembly calls are listed here
//
#include "services.h"
#include "types.h"
#include "data.h"
#include "tools.h"
int GetPid()
{
  int pid;

  asm("push %%eax; \
       int $0x64; \
       movl %%eax, %0; \
       popl %%eax; "
       : "=g" (pid) 
       :);
  return pid;
}

void Sleep(int seconds)
{
  asm("pushl %%eax; \
       movl %0, %%eax; \
       int $0x65; \
       popl %%eax;"
       :
       : "g" (seconds));
}

int SemAlloc(int passes)
{
  int sid;
  asm("pushl %%eax; \
       pushl %%ebx; \
       movl %1, %%eax; \
       int $0x66; \
       movl %%ebx, %0; \
       popl %%ebx; \
       popl %%eax;"
       : "=g" (sid)
       : "g" (passes));

  return sid;
}

void SemWait(int sid)
{
  asm("pushl %%eax; \
       movl %0, %%eax; \
       int $0x67; \
       popl %%eax;"
       :
       : "g" (sid));

}

void SemPost(int sid)
{

  asm("pushl %%eax; \
       movl %0, %%eax; \
       int $0x68; \
       popl %%eax;"
       :
       : "g" (sid));

}

void SysPrint(char *str_to_print)
{
  asm("pushl %%eax; \
       movl %0, %%eax; \
       int $0x69; \
       popl %%eax;"
	:
	: "g" ((int)str_to_print));
}

int PortAlloc(void) {
  int port_num;
  asm("pushl %%eax; \
       int $0x6a; \
       movl %%eax, %0; \
       popl %%eax;"
       : "=g" (port_num)
       : );
  Sleep(1);
  port[port_num].write_sid = SemAlloc(Q_SIZE);
  port[port_num].read_sid = SemAlloc(0);
  port[port_num].read_q.size = 0;
  return port_num;
}

void PortWrite(char *p, int port_num) {
  while (*p)
  {
    SemWait(port[port_num].write_sid);
    asm("pushl %%eax; \
         pushl %%ebx; \
         movl  %0, %%eax; \
         movl  %1, %%ebx; \
         int   $0x6b; \
         popl %%ebx; \
         popl %%eax;"
         : 
         : "g" ((char)*p), "g" (port_num));
    p++;
  }
}

void PortRead(char *p, int port_num) {
  int size;
  size = 0;
  while(1)
  {
    SemWait(port[port_num].read_sid);
    asm("pushl %%eax; \
         pushl %%ebx; \
         movl  %0, %%eax; \
         movl  %1, %%ebx; \
         int $0x6c; \
         popl %%ebx; \
         popl %%eax;"
         :
         : "g" ((int)p), "g" (port_num));
    if (*p == '\r') break;
    p++;
    size++;
    if (size == Q_SIZE -1) break;

  }
  *p = '\0';
}

void FSfind (char *name, char *cwd, char *data)
{
  char tmp[BUFF_SIZE];
  MyStrcpy(tmp, cwd);
  MyStrcat(tmp, "/");
  MyStrcat(tmp, name);
  asm("pushl %%eax; \
       pushl %%ebx; \
       movl %0, %%eax; \
       movl %1, %%ebx; \
       int $0x6D; \
       popl %%ebx; \
       popl %%eax;"
       :
       : "g" ((int)tmp), "g" ((int)data));
}

int FSopen(char *name, char *cwd)
{
  int fd;
  char tmp[BUFF_SIZE];
  MyStrcpy(tmp, cwd);
  MyStrcat(tmp, "/");
  MyStrcat(tmp, name);
  asm("pushl %%eax; \
       pushl %%ebx; \
       movl %1, %%eax; \
       int $0x6E; \
       movl %%ebx, %0; \
       popl %%ebx; \
       popl %%eax;"
       : "=g" (fd)
       : "g" ((int)tmp));
  return fd;
}

void FSread(int fd, char *data)
{
  asm("pushl %%eax; \
       pushl %%ebx; \
       movl %0, %%eax; \
       movl %1, %%ebx; \
       int $0x6F; \
       popl %%ebx; \
       popl %%eax;"
       : 
       : "g" (fd), "g" ((int)data));
}

void FSclose(int fd)
{
  asm("pushl %%eax; \
       movl %0, %%eax; \
       int $0x70; \
       popl %%eax;"
       :
       : "g" (fd));
}

int Fork(char *p)
{
int pid;
  asm("pushl %%eax;\
       pushl %%ebx;\
       movl %1, %%eax; \
       int $0x71; \
       movl %%ebx, %0; \
       popl %%ebx; \
       popl %%eax;"
       : "=g" (pid)
       : "g" ((int)p));
return pid;
}

int Wait()
{
 int temp;
 asm("pushl %%eax; \
	int $0x72; \
	movl %%eax, %0; \
	popl %%eax;"
	: "=g" (temp)
	: );
 return temp;
}

void Exit(int exit_num)
{
  asm("pushl %%eax; \
       movl %0, %%eax; \
	int $0x73; \
	popl %%eax;"
	:
	: "g" (exit_num));
}
