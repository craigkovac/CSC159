// tools.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"

// clear DRAM data blocks by filling zeroes
void MyBzero(char *p, int size) {
   //loop size times during which:
     // set where p points to to 0 and increment p
    int i;
    for (i = 0; i<size;i++) 
    {
      p = 0;
      p++;
    }

}

// dequeue, return 1st integer in array, and move all forward
// if queue empty, return 0
int DeQ(q_t *p) { // return 0 if q[] is empty
   int i, data = 0;

   if (p->size == 0 ) return data;
   else {
      //data is the 1st integer in the array that p points to
      data = p->q[0];
      //decrement the size of the queue (that p points to)
      p->size--;
      //move all integers in the array forward by one position
      for (i=0; i<20;i++)
      {
        p->q[i] = p->q[i+1];
      }
   }
   return data;
}

// enqueue integer to next available slot in array, size is index
void EnQ(int data, q_t *p) {
   if (p->size == Q_SIZE) {
      cons_printf("Kernel Panic: queue is full, cannot EnQ!\n");
      return;       // alternative: breakpoint() into GDB
   }
   //add data into the array index by the size of the queue
   p->q[p->size] = data;
   p->size++;
}

int MyStrlen(char *p)
{
  int len=0;
  while (*p != '\0')
  {
    p++;
    len++;
  }

  return len;
}

void MyStrcat(char *dst, char *addendum)
{
  while(*dst) dst++;
  while (*addendum != '\0') 
  {
    *dst = *addendum;
    dst++; addendum++;
  }
  *dst = '\0';
}

int MyStrcmp(char *p, char *q, int len)
{
  int i;
  for(i=0; i<len; i++)
  {
    if (*p != *q) return 0;
    p++; q++;
  }
  return 1;
}

void MyStrcpy(char *dst, char *src)
{
  char ch;
  ch = *src;
  while(ch)
  {
    *dst = ch;
    dst++;
    src++;
    ch = *src;
  }
  
  *dst = ch;
}

void MyMemcpy(char *dst, char *src, int size)
{
  int i;
  for(i=0; i<size; i++) 
  {
    *dst = *src;
    dst++; src++;
  }
}


