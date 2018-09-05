//*****************************************************
//NAME: Craig Kovac
//Phase 0 , Exercise 4 -- Timer Event
//main.c
//*****************************************************


#define LOOP 1666000
#include "spede.h"
#include "events.h"


typedef void (* func_ptr_t)();
struct i386_gate *IDT_p;

void RunningProcess(void)
{
	int i;
	
	while(1)
	{
		//stuff
		//if keyboard is pressed break
	
		for(i = 0;i<LOOP;i++)
		{
			asm("inb $0x80");
		}
		cons_printf("z");
		if(cons_kbhit())
		{
			break;
		}

	//	TimerEvent();
	}
}


int main()
{
	IDT_p = get_idt_base();
	cons_printf("IDT located at DRAM addr %x (%d).\n",*IDT_p, IDT_p );

	

	fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
	outportb(0x21, ~1);
	asm("sti");

	RunningProcess();
	
	return 0;
}
