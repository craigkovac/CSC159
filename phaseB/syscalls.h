//syscalls.h
#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

int GetPid(void);         // no input, 1 return
void Write(int, char *);
void Sleep(int);
void Mutex(int);
char GetChar(int);
void PutChar(int, char);
void PutStr(int, char *);
void GetStr(int, char *, int);
int Fork(void);
//void Signal(int, unsigned int);
void Signal(int, void *);
void Exit(int);
int WaitPid(int *);
void Exec(unsigned int);

#endif

