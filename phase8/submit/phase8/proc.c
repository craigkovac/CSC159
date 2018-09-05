// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls


#include "services.h"
#include "spede.h"      // cons_xxx below needs
#include "data.h"       // current_pid needed below
#include "proc.h"       // prototypes of processes
#include "handlers.h"
#include "tools.h"

void Init(void) {
   int i;
   char key;
   while(1) {

     for (i=0; i < LOOP; i++)
      {
        asm("inb $0x80");
      }

if (cons_kbhit() ) {
      key = cons_getchar();

      switch (key) {
         case 'p':
            SysPrint("Team name:  offby1, Brandon Byrne \r\n");
	          break;
      case 'b':
            breakpoint();
            break;
      case 't':
            NewProcHandler(TermProc);
            break;
     }
   }      
   }
}

// PID 2, 3, 4, etc. mimicking a usual user process
void UserProc(void) {
   
   while(1) {
      cons_printf( "%d..", GetPid());
      Sleep(GetPid()); 
   }
}

void TermProc(void) {
  int i,len,my_port,exit_num;
  char login_str[BUFF_SIZE], passwd_str[BUFF_SIZE],
       cmd_str[BUFF_SIZE], cwd[BUFF_SIZE], str[BUFF_SIZE];
  my_port = PortAlloc();
  

  while (1)
  {
    while(1)
    {
      MyMemcpy(passwd_str, "\0", 1); 
      MyMemcpy(login_str, "\0", 1); 
      PortWrite("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n", my_port);
      PortWrite("Team offby1 Login: ", my_port);
      PortRead(login_str, my_port);
      PortWrite("Team offby1 Password: ", my_port);
      PortRead(passwd_str, my_port);
      len = MyStrlen(login_str);
      if (!len)
      {
        PortWrite("\tStop hacking, get to work!\r\n", my_port);
        continue;
      }
 
      if (len != MyStrlen(passwd_str))
      { 
        PortWrite("\tStop hacking, get to work!\r\n", my_port);
        continue;
      }

      for(i=0; i<len; i++)
      {
        if (login_str[i] != passwd_str[len-i-1])
        {  
          PortWrite("\tStop hacking, get to work!\r\n", my_port);
          len = 0;
          break;
        }
      }

      if (len == 0) continue;
  
      PortWrite("\tWelcome!\n\r\tServices are: pwd, cd dir-name, ls, cat filename, exit\r\n", my_port);
      MyMemcpy(cwd, "/\0", 2);   
      exit_num = 0;    
      break;
    }

    while(1)
    {
      PortWrite("Team offby1 -> ", my_port);
      PortRead(cmd_str, my_port);
      len = MyStrlen(cmd_str);
      if (!len) continue;
      if (MyStrcmp(cmd_str, "exit\0", 5)) break;
      else if (MyStrcmp(cmd_str, "pwd\0", 4)) 
      {
       // PortWrite("\r\n",my_port);
        
        PortWrite(cwd, my_port);
        PortWrite("\r\n", my_port);
      }
      else if (MyStrcmp(cmd_str, "echo", 4))
      {
        sprintf(str, "%d (0x%x)\r\n", exit_num, exit_num); 
      	PortWrite(str, my_port);
      }
      else if (MyStrcmp(cmd_str, "cd ", 3)) 
      {
        MyMemcpy(cmd_str, cmd_str+3,MyStrlen(cmd_str)-2);
        TermCd(cmd_str ,cwd, my_port);
      }
      else if (MyStrcmp(cmd_str, "ls\0", 3)) TermLs(cwd, my_port);
      else if (MyStrcmp(cmd_str, "cat ", 3)) 
      {  
        MyMemcpy(cmd_str, cmd_str+4,MyStrlen(cmd_str)-3);
        TermCat(cmd_str ,cwd, my_port);
      }
      else 
      {
//	PortWrite("\tStop Hacking, get to work!\r\n", my_port);
	exit_num = TermBin(cmd_str, cwd, my_port);
      }
    }
    

    //cons_printf("Read from port #%d: %s\n", my_port, str_read);
  }
}

int TermBin(char *name, char *cwd, int my_port)
{
  char attr_data[BUFF_SIZE], str[BUFF_SIZE];
  int cpid;
  attr_t *attr_p;
  
  FSfind(name, cwd, attr_data);
  
  if (MyStrlen(attr_data) == 0)
  {
    PortWrite("File not found\r\n", my_port);
    return -1;
  }
  attr_p = (attr_t *)attr_data;
  if (!(attr_p->mode == MODE_EXEC)) 
  {
    PortWrite("Not executable\r\n", my_port);
    return -1;
  }
  cpid = Fork(attr_p->data);
  sprintf(str, "\tForked child PID: %d\r\n", cpid);
  PortWrite(str, my_port);
  Sleep(4);
  return Wait();
}

void TermCd(char *name, char *cwd, int my_port)
{
  char attr_data[BUFF_SIZE];
  attr_t *attr_p;

  if (!MyStrlen(name)) return;
  if (MyStrcmp(name, ".\0", 2)) return;
  if (MyStrcmp(name, "/\0", 2) || MyStrcmp(name, "..\0", 3)) 
  {
    MyMemcpy(cwd, "/\0", 2);       
    return;
  }

  FSfind(name, cwd, attr_data);
  if (MyStrlen(attr_data) == 0)
  {
    PortWrite("\nFile not found\r\n", my_port);
    return;
  }
  attr_p = (attr_t *)attr_data;
  if (!A_ISDIR(attr_p->mode))
  {
    PortWrite("\nCannot cd a file\r\n", my_port);
    return;
  }

  MyStrcat(cwd, name);
}

void TermCat(char *name, char *cwd, int my_port)
{
  char read_data[BUFF_SIZE], attr_data[BUFF_SIZE];
  attr_t *attr_p;
  int my_fd;
  MyStrcat(cwd, "/");
  FSfind(name, cwd, attr_data);
  if(MyStrlen(attr_data) == 0)
  {
    PortWrite("\nFile not found\r\n", my_port);
    return;
  }
  
  attr_p = (attr_t *)attr_data;
  if (A_ISDIR(attr_p->mode))
  { 
    PortWrite("\nCannot cat a dir\r\n", my_port);
    return;
  }

  my_fd = FSopen(name, cwd);

  while(1)
  {
    FSread(my_fd, read_data);
    if (!MyStrlen(read_data)) break;
    PortWrite(read_data, my_port);
  }

  FSclose(my_fd);
}

void TermLs(char *cwd, int my_port)
{
  char ls_str[BUFF_SIZE], attr_data[BUFF_SIZE];
  attr_t *attr_p;
  int my_fd;

  FSfind("", cwd, attr_data);

  if(MyStrlen(attr_data) == 0)
  {
    PortWrite("\nFile not found\r\n", my_port);
    return;
  }
  
  attr_p = (attr_t *)attr_data;
  if (!A_ISDIR(attr_p->mode))
  { 
    PortWrite("\nCannot ls a file\r\n", my_port);
    return;
  }

  my_fd = FSopen("", cwd);

  while(1)
  {
    FSread(my_fd, attr_data);
    if (!MyStrlen(attr_data)) break;
    attr_p = (attr_t *)attr_data;
    Attr2Str(attr_p, ls_str);
    PortWrite(ls_str, my_port);
  }

  FSclose(my_fd);
}

void Attr2Str(attr_t *attr_p, char *str)
{
  char *name = (char *)(attr_p + 1);
  sprintf(str, "     - - - -    SIZE %4d    NAME  %s\n\r", attr_p->size,     name);
  if ( A_ISDIR(attr_p->mode) ) str[5] = 'D';          // mode is directo    ry
  if ( QBIT_ON(attr_p->mode, A_ROTH) ) str[7] = 'R';  // mode is readabl    e
  if ( QBIT_ON(attr_p->mode, A_WOTH) ) str[9] = 'W';  // mode is writabl    e
  if ( QBIT_ON(attr_p->mode, A_XOTH) ) str[11] = 'X'; // mode is executa    ble
}
