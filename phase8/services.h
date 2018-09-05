#ifndef __SERVICES_H__
#define __SERVICES_H__
int GetPid();
void Sleep(int);
void SemWait(int);
void SemPost(int);
int SemAlloc(int);
void SysPrint(char *);
int PortAlloc(void);
void PortWrite(char *, int);
void PortRead(char *, int);
void FSfind(char *, char *, char *);
int FSopen(char*, char*);
void FSread(int, char*);
void FSclose(int);
int Fork(char *p);
int Wait(void);
void Exit(int exit_num);


#endif
