// proc.h, 159

#ifndef __PROC_H__
#define __PROC_H__

void Init(void);      // PID 1, eternal, never preempted
void UserProc(void);  // PID 2, 3, ...
void Vehicle(void);
void TermProc(void);
void TermCd(char *, char*, int);
void TermCat(char *, char*, int);
void TermLs(char *cwd, int);
void Attr2Str(attr_t*, char*);
int TermBin(char*, char*,int);
#endif
