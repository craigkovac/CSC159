//*****************************************************
//handlers.c, Phase 0, Exercise 4 -- Timer Event
//*****************************************************
#include "spede.h"

char my_name[] = "Craig Kovac";
int i = 0;
int tick_count = 0;
int j = 0;

unsigned short *char_p = (unsigned short *) 0xB8000+12*80+34;

void TimerHandler()
{
	//stuff
	if(tick_count == 0)
	{
		//*char_p = 
	 //       printf("%c", my_name[i]);
	        //cons_printf("%c", my_name[i]);
	}
	tick_count++;

	if(tick_count%75==0)
	{
		tick_count = 0;
		
		
		*char_p = my_name[i] + 0xf00;
		i++;
		char_p++;
		if(i%12==0 )
		{	
			char_p = char_p-12;
			i = 0;
			j= 0;
			
			while(j<12)
			{
				*char_p = (unsigned short *) 0xB8000+12*80+34;	
				*char_p = ' ' + 0xf00;
				char_p++;
				j++;	
			}
			char_p = char_p-12;
	//		*char_p = (unsigned short *) 0xB8000+12*80+34;
		}	
	}



	outportb(0x20, 0x60);
}
