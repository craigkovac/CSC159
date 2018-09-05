#include <spede/stdio.h>
#include <spede/flames.h>

void DisplayMsg(int i)
{
	printf("%d Hello World %d \nECS", i, 2*i);
        cons_printf("--> Hello World <--\nCPE/CSC" );
}


int main(void)
{
	long i;
	int p = 0;

	i = 111;
	
	while (p<5)
	{
		p++;
		i++;
		if (i ==113)
		{
			DisplayMsg(i);
		}
	}
	return 0;
}

