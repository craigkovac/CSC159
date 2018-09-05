// tools.h, 159

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include "types.h" // need definition of 'q_t' below

void MyBzero(char *, int);
int DeQ(q_t *);
void EnQ(int, q_t *);
int MyStrlen(char*);
void MyStrcat(char *, char*);
int MyStrcmp(char *, char*, int);
void MyStrcpy(char *, char*);
void MyMemcpy(char *, char *, int);
#endif

