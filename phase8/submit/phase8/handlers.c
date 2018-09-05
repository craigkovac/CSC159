// handlers.c, 159

#include "spede.h"
#include "types.h"
#include "handlers.h"
#include "tools.h"
#include "data.h"
#include "proc.h"
#include "services.h"
void NewProcHandler(func_ptr_t p) {  
   int pid;

   if (free_q.size == 0) { 
      cons_printf( "Kernel Panic: no more PID left!\n");
      return;                  
   }

   pid = DeQ(&free_q);
   MyBzero((char *)&proc_stack[pid], PROC_STACK_SIZE);

   pcb[pid].state = READY;
   EnQ(pid, &ready_q);

   pcb[pid].TF_p = (TF_t *)&proc_stack[pid][PROC_STACK_SIZE - sizeof(TF_t) - 1] ;
   
   pcb[pid].TF_p->eflags = EF_DEFAULT_VALUE|EF_INTR;
   
   pcb[pid].TF_p->cs = get_cs();  
   pcb[pid].TF_p->eip = (unsigned int)p;
   pcb[pid].TF_p->ds = get_ds(); 
   pcb[pid].TF_p->es = get_es();
   pcb[pid].TF_p->fs = get_fs();
   pcb[pid].TF_p->gs = get_gs();
   pcb[pid].MMU = kernel_MMU;

   if (pid < 10)
   ch_p[pid*80+40] = 0xf00 + (0x30+pid);
   else 
   {
   ch_p[pid*80+39] = 0xf00 + (0x30+1);
   ch_p[pid*80+40] = 0xf00 + (0x30 + (pid%10));
   }
}

void TimerHandler(void) {
   int i;
   pcb[current_pid].cpu_time += 1;
   current_time++;

   for (i = 0; i < PROC_NUM; i++)
   {
      if (pcb[i].state == SLEEP && pcb[i].wake_time == current_time)
      {
        
        pcb[i].state = READY;
        EnQ(i, &ready_q);
        ch_p[i*80+43] = 0xf00 + 'r';
      }
   }

   if (pcb[current_pid].cpu_time >= TIME_LIMIT) 
   {
      
      pcb[current_pid].state = READY;
      EnQ(current_pid, &ready_q);
      ch_p[current_pid*80+43] = 0xf00 + 'r';
      current_pid = 0;
   }

   outportb( 0x20, 0x60 );
}

void GetPidHandler(void) 
{
  pcb[current_pid].TF_p->eax = current_pid;
}

void SleepHandler(void)
{
  pcb[current_pid].wake_time = current_time + 100 * pcb[current_pid].TF_p->eax;
  pcb[current_pid].state = SLEEP;
  ch_p[current_pid*80+43] = 0xf00 + 'S';
  current_pid = 0;
 
}

void SemAllocHandler(int passes)
{
  int sid = 0;
  int i;
  for(i=1;i<Q_SIZE;i++) if (sem[i].owner == 0) sid = i;
  if (sid != 0) 
  {
    sem[sid].wait_q.size = 0;
    sem[sid].passes = passes;
    sem[sid].owner  = current_pid;
  }
  else 
  {
    sid = -1;
  }
  pcb[current_pid].TF_p->ebx = sid;
}

void SemWaitHandler(int sid)
{
  if (sem[sid].passes > 0) 
  {
    sem[sid].passes--;
    ch_p[48] = 0xf00 + (0x30+sem[sid].passes);
  }
  else
  {
    EnQ(current_pid, &sem[sid].wait_q);
    pcb[current_pid].state = WAIT;
    ch_p[current_pid*80+43] = 0xf00 + 'W';
    current_pid = 0;
  }
  
}

void SemPostHandler(int sid)
{
  int waiting_pid;
  if (sem[sid].wait_q.size == 0)
  {
    sem[sid].passes++;
    ch_p[48] = 0xf00 + (0x30+sem[sid].passes);
  }
  else 
  {
    waiting_pid = DeQ(&sem[sid].wait_q);
    EnQ(waiting_pid, &ready_q);
    ch_p[waiting_pid*80+43] = 0xf00 + 'r';
  }

}

void SysPrintHandler(char *p)
{
    int i, code;
    const int printer_port = 0x378;

    outportb(printer_port, 16);
    code = inportb(printer_port +1);
    for(i=0; i<50; i++) asm("inb $0x80");
    outportb(printer_port +2, 4 | 8);

    while(*p)
    {
	outportb(printer_port, *p);
        code = inportb(printer_port + 2);
	outportb(printer_port + 2, code | 1);
	for(i=0; i < 50; i++) asm("inb $0x80");
	outportb(printer_port + 2, code);

	for(i=0; i < LOOP*3; i++)
	{
	    code = inportb(printer_port +1) & 64;
	    if (code == 0) break;
	    asm("inb $0x80");
	}	

	if (i == LOOP*3)
	{
		cons_printf(">>> Printer timed out! \n");
		break;
	}
	p++;
    }
}

void PortWriteOne(int port_num) {
  char one;
  if (port[port_num].write_q.size == 0 && port[port_num].loopback_q.size == 0) 
  {
    port[port_num].write_ok = 1;
    return;
  }

  if (port[port_num].loopback_q.size != 0)
  one = DeQ(&port[port_num].loopback_q);
  else
  {
    one = DeQ(&port[port_num].write_q);
    SemPostHandler(port[port_num].write_sid);
  }
  

 
  outportb(port[port_num].IO, one);
  port[port_num].write_ok = 0;

}

void PortReadOne(int port_num) {
  char one;
 
  one = inportb(port[port_num].IO);
  if (port[port_num].read_q.size == BUFF_SIZE)
  {
    cons_printf("Kernal Panic: your typing on terminal is super fast!\n");
    return;
  }

  EnQ(one, &port[port_num].read_q);
  EnQ(one, &port[port_num].loopback_q);

  if (one == '\r') EnQ('\n', &port[port_num].loopback_q);

  SemPostHandler(port[port_num].read_sid);
}

void PortHandler(void) {
  int port_num, intr_type; 
  for (port_num = 0; port_num < PORT_NUM - 1; port_num++)
  {
    if (port[port_num].owner != 0) 
    {
      intr_type = inportb(port[port_num].IO + IIR);
      if (intr_type == IIR_RXRDY)  PortReadOne(port_num);
      if (intr_type == IIR_TXRDY)  PortWriteOne(port_num);
      if (port[port_num].write_ok == 1) PortWriteOne(port_num);
    }
  }

  //Clear COM interrupts
  outportb(0x20, 0x63);
  outportb(0x20, 0x64);
}

void PortAllocHandler(int *eax) {
  int port_num, baud_rate, divisor;
  static int IO[PORT_NUM] = { COM2_IOBASE, COM3_IOBASE, COM4_IOBASE };

  for (port_num = 0; port_num < PORT_NUM -1; port_num++)
    if (port[port_num].owner == 0) break;
  

  if (port_num == PORT_NUM) 
  {
    cons_printf("Kernal Panic: no ports left!\n");
    return;
  }

  *eax = port_num;
  MyBzero((char *)&port[port_num], sizeof(port_t));
  port[port_num].owner = current_pid;
  port[port_num].IO = IO[port_num];
  port[port_num].write_ok = 1;

  baud_rate = 9600;
  divisor = 115200 / baud_rate;

  outportb(port[port_num].IO + CFCR, CFCR_DLAB);
  outportb(port[port_num].IO + BAUDLO, LOBYTE(divisor));
  outportb(port[port_num].IO + BAUDHI, HIBYTE(divisor));
  outportb(port[port_num].IO + CFCR, CFCR_PEVEN | CFCR_PENAB | CFCR_7BITS);
  outportb(port[port_num].IO + IER, 0);
  outportb(port[port_num].IO + MCR, MCR_DTR | MCR_RTS | MCR_IENABLE);
  asm("inb $0x80");
  outportb(port[port_num].IO + IER, IER_ERXRDY | IER_ETXRDY);
}

void PortWriteHandler(char one, int port_num) {
  if (port[port_num].write_q.size == Q_SIZE)
  {
    cons_printf("Kernal Panic: terminal is not prompting (fast enough)?\n");
    return;
  }

  EnQ(one, &port[port_num].write_q);
  if (port[port_num].write_ok == 1) PortWriteOne(port_num);
}

void PortReadHandler(char *one, int port_num) {
  if (port[port_num].read_q.size == 0)
  {
    cons_printf("Kernel Panic: nothing in typing/read buffer\n");
    return;
  }

  *one = DeQ(&port[port_num].read_q);
}

void FSfindHandler(void) 
{
  char *name, *data;
  attr_t *attr_p;
  dir_t *dir_p;
  name = (char *)pcb[current_pid].TF_p->eax;
  data = (char *)pcb[current_pid].TF_p->ebx;

  dir_p = FSfindName(name);

  if (! dir_p)
  {
    data[0] = 0;
    return;
  }

  attr_p = (attr_t *)data;
  FSdir2attr(dir_p, attr_p);
  MyMemcpy((char *)(attr_p+1), dir_p->name, MyStrlen(dir_p->name) +1);
}

void FSopenHandler(void) {
   char *name;
   int fd;
   dir_t *dir_p;

   name = (char *)pcb[current_pid].TF_p->eax;

   fd = FSallocFD(current_pid);  // current_pid is owner of fd allocated

   if( fd == -1 ) {
      cons_printf("FSopenHandler: no more File Descriptor!\n");
      pcb[current_pid].TF_p->ebx = -1;
      return;
   }

   dir_p = FSfindName(name);
   if(! dir_p) {
      cons_printf("FSopenHandler: name not found!\n");
      pcb[current_pid].TF_p->ebx = -1;
      return;
   }

   fd_array[fd].item = dir_p;        // dir_p is the name
   pcb[current_pid].TF_p->ebx = fd;  // process gets this to future read
}

// Copy bytes from file into user's buffer. Returns actual count of bytes
// transferred. Read from fd_array[fd].offset (initially given 0) for
// buff_size in bytes, and record the offset. may reach EOF though...
void FSreadHandler(void) {
   int fd, result, remaining;
   char *read_data;
   dir_t *lp_dir;

   fd = pcb[current_pid].TF_p->eax;
   read_data = (char *)pcb[current_pid].TF_p->ebx;

   if(! FScanAccessFD(fd, current_pid)) {
      cons_printf("FSreadHandler: cannot read from FD!\n");
      read_data[0] = 0;  // null-terminate it
      return;
   }

   lp_dir = fd_array[fd].item;

   if( A_ISDIR(lp_dir->mode ) ) {  // it's a dir
// if reading directory, return attr_t structure followed by obj name.
// a chunk returned per read. `offset' is index into root_dir[] table.
      dir_t *this_dir = lp_dir;
      attr_t *attr_p = (attr_t *)read_data;
      dir_t *dir_p;

      if( BUFF_SIZE < sizeof( *attr_p ) + 2) {
         cons_printf("FSreadHandler: read buffer size too small!\n");
         read_data[0] = 0;  // null-terminate it
         return;
      }

// use current dir, advance to next dir for next time when called
      do {
         dir_p = ((dir_t *)this_dir->data);
         dir_p += fd_array[fd].offset ;

         if( dir_p->inode == END_INODE ) {
            read_data[0] = 0;  // EOF, null-terminate it
            return;
         }
         fd_array[fd].offset++;   // advance
      } while(dir_p->name == 0);

// MyBzero() fills buff with 0's, necessary to clean buff
// since FSdir2attr may not completely overwrite whole buff...
      MyBzero(read_data, BUFF_SIZE);
      FSdir2attr(dir_p, attr_p);

// copy obj name after attr_t, add 1 to length for null
      MyMemcpy((char *)( attr_p + 1 ), dir_p->name, MyStrlen( dir_p->name ) + 1);

   } else {  // a file, not dir
// compute max # of bytes can transfer then MyMemcpy()
      remaining = lp_dir->size - fd_array[fd].offset;

      if( remaining == 0 ) {
         read_data[0] = 0;  // EOF, null-terminate it
         return;
      }

      MyBzero(read_data, BUFF_SIZE);  // null termination for any part of file read

      result = remaining<100?remaining:100; // -1 saving is for last null

      MyMemcpy(read_data, &lp_dir->data[ fd_array[ fd ].offset ], result);

      fd_array[fd].offset += result;  // advance our "current" ptr
   }
}

// check ownership of fd and the fd is valid within range
int FScanAccessFD( int fd, int owner ) {
   if( fd_array[fd].owner == owner) return 1;
   return 0;     // not good
}

// Search our (fixed size) table of file descriptors. returns fd_array[] index
// if an unused entry is found, else -1 if all in use. if avail, then all
// fields are initialized.
int FSallocFD( int owner ) {
   int i;

   for(i=0; i<FD_NUM; i++) {
      if( 0 == fd_array[i].owner ) {
         fd_array[i].owner = owner;
         fd_array[i].offset = 0;
         fd_array[i].item = 0;     // NULL is (void *)0, spede/stdlib.h

         return i;
      }
   }

   return -1;   // no free file descriptors
}

dir_t *FSfindName( char *name ) {
   dir_t *starting;

// assume every path relative to root directory. Eventually, the user
// context will contain a "current working directory" and we can possibly
// start our search there
   if( name[0] == '/' ) {
      starting = root_dir;

      while( name[0] == '/' ) name++;

      if( name[0] == 0 ) return root_dir; // client asked for "/"
   } else {
// path is relative, so start off at CWD for this process
// but we don't have env var CWD, so just use root as well
      starting = root_dir; // should be what env var CWD is
   }

   if( name[0] == 0 ) return 0;

   return FSfindNameSub(name, starting);
}

// go searching through a single dir for a name match. use MyStrcmp()
// for case-insensitive compare. use '/' to separate directory components
// if more after '/' and we matched a dir, recurse down there
// RETURN: ptr to dir entry if found, else 0
// once any dir matched, don't return name which dir was matched
dir_t *FSfindNameSub( char *name, dir_t *this_dir ) {
   dir_t *dir_p = this_dir;
   int len = MyStrlen(name);
   char *p;

// if name is '.../...,' we decend into subdir
   if( ( p = strchr( name, '/' ) ) != 0) len = p - name;  // p = to where / is (high mem)

   for( ; dir_p->name; dir_p++ ) {
//      if((unsigned int)dir_p->name > 0xdfffff) return 0; // tmp bug-fix patch

      if( 1 == MyStrcmp( name, dir_p->name, len ) ) {
         if( p && p[1] != 0 ) { // not ending with name, it's "name/..."
// user is trying for a sub-dir. if there are more components, make sure this
// is a dir. if name ends with "/" we don't check. thus "hello.html/" is legal
            while( *p == '/' ) {
               p++;                           // skipping trailing /'s in name
               if( '\0' == *p ) return dir_p; // name "xxx/////" is actually legal
            }

// altho name given is "xxx/yyy," xxx is not a directory
            if(dir_p->mode != MODE_DIR) return 0; // bug-fix patch for "cat h/in"

            name = p;
            return FSfindNameSub(name, (dir_t *)dir_p->data);
         }
         return dir_p;
      }
   }

   return 0;   // no match found
}

// copy what dir_p points to (dir_t) to what attr_p points to (attr_t)
void FSdir2attr( dir_t *dir_p, attr_t *attr_p ) {
   attr_p->dev = current_pid;            // current_pid manages this i-node

   attr_p->inode = dir_p->inode;
   attr_p->mode = dir_p->mode;
   attr_p->nlink = ( A_ISDIR( attr_p->mode ) ) + 1;
   attr_p->size = dir_p->size;
   attr_p->data = dir_p->data;
}

void FScloseHandler(void) {
   int fd;

   fd = pcb[current_pid].TF_p->eax;

   if (FScanAccessFD(fd, current_pid))fd_array[fd].owner = 0;
   else  cons_printf("FScloseHandler: cannot close FD!\n");
}

void ForkHandler(char *bin_code, int *child_pid)
{
  int i, got, page_got[5];
  TF_t *TF_p;
  
//Coding hints said to use a char * for all the tables, but I think that is wrong.  char * means that 
//each entry is only 1 byte, but pointers are 4 bytes.  So when you try to do main_page[256] only the lowest
//byte from code_table gets entered into it

  int *main_table, *code_table, *stack_table, *code_page, *stack_page;
  for(i=0,got=0;i<MEM_PAGE_NUM;i++) 
  {
    if (mem_page[i].owner == 0) 
	{
	  page_got[got++] = i;
	}
    if (got == 5) break;
  }

  if (got != 5) 
    {
	cons_printf("Kernel Panic: not enough memory pages available!\n");
        *child_pid = 0;
	return;
    }

  if (i == MEM_PAGE_NUM) 
  {
    cons_printf("Kernel Panic: no memory page available!\n");
    *child_pid = 0;
    return;
  }

  if (free_q.size == 0)
  {
    cons_printf("Kernel Panic: no PID available!'n");
    *child_pid = 0;
    return;
  }

  main_table = (int *)mem_page[page_got[0]].addr;
  code_table = (int *)mem_page[page_got[1]].addr;
  stack_table = (int *)mem_page[page_got[2]].addr;
  code_page =  (int *)mem_page[page_got[3]].addr;
  stack_page = (int *)mem_page[page_got[4]].addr;


  *child_pid = DeQ(&free_q);

  for(got=0; got<4; got++)
  {
    mem_page[page_got[got]].owner = *child_pid;
    MyBzero(mem_page[page_got[got]].addr, MEM_PAGE_SIZE);
  }


  MyBzero((char *)&proc_stack[*child_pid], sizeof(pcb_t)); 
  EnQ(*child_pid, &ready_q);
  pcb[*child_pid].state = READY;
  pcb[*child_pid].ppid  = current_pid;
  pcb[*child_pid].TF_p = (TF_t *)(0x80000000 - sizeof(TF_t));
  pcb[*child_pid].MMU = (int)main_table;



  MyMemcpy((char *)code_page, bin_code, MEM_PAGE_SIZE);
  TF_p = (TF_t *)((int)stack_page + (MEM_PAGE_SIZE - sizeof(TF_t)));
  
  //cons_printf("stack page %x + %d - %d", stack_page, MEM_PAGE_SIZE, sizeof(TF_t));
  TF_p->eip = 0x40000000;
 

  TF_p->eflags = EF_DEFAULT_VALUE|EF_INTR;

  TF_p->cs = get_cs();
  TF_p->ds = get_ds();
  TF_p->es = get_es();
  TF_p->fs = get_fs();
  TF_p->gs = get_gs();

  MyMemcpy((char *)main_table, (char *)kernel_MMU, sizeof(int *) * 4);   
  main_table[256] = ((unsigned int)code_table | 0x03);
  main_table[511] = ((unsigned int)stack_table | 0x03);
  code_table[0] = ((unsigned int )code_page | 0x03);
  stack_table[1023] = ((unsigned int)stack_page | 0x03);

}

void WaitHandler(int *eax_p) 
{
  int cpid, page_index;
  for (cpid=0; cpid < PROC_NUM; cpid++)
  {
    if (pcb[cpid].ppid == current_pid && pcb[cpid].state == ZOMBIE) break;
  }

  if (cpid == PROC_NUM) 
  {
    pcb[current_pid].state = WAIT;
    current_pid = 0;
    return;
  }
 if (pcb[cpid].MMU != 0) set_cr3(pcb[cpid].MMU);
  *eax_p = pcb[cpid].TF_p->eax;
  set_cr3(kernel_MMU);
  EnQ(cpid, &free_q);
  pcb[cpid].state = FREE;

  for (page_index=0; page_index<MEM_PAGE_NUM; page_index++)
    if(mem_page[page_index].owner == cpid) break;

  if(page_index == MEM_PAGE_NUM)
  {
    cons_printf("Kernel Panic: cpid memory not found\n");
    return;
  }

  mem_page[page_index].owner = 0;
}

void ExitHandler(int exit_num)
{
  int ppid, page_index;
  
  ppid = pcb[current_pid].ppid;

  if(pcb[ppid].state != WAIT)
  {
    pcb[current_pid].state = ZOMBIE;
    current_pid = 0;
    return;
  }
  set_cr3(pcb[ppid].MMU);

  EnQ(ppid, &ready_q);
  pcb[ppid].state = READY;
  pcb[ppid].TF_p->eax = exit_num;

  EnQ(current_pid, &free_q);
  pcb[current_pid].state = FREE;

  for (page_index=0; page_index<MEM_PAGE_NUM; page_index++)
  {
    if (mem_page[page_index].owner == current_pid) 
      mem_page[page_index].owner = 0;
  }

  current_pid = 0;

/*
  if(page_index == MEM_PAGE_NUM)
  {
    cons_printf("Kernel Panic: cpid memory not found!\n");
    return;
  }
  */

  //mem_page[page_index].owner = 0;
}
