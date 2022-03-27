#ifndef ACC_SCAN_H
#define ACC_SCAN_H

#include "defs.h"

extern int Line;
extern int Preview;
extern FILE *Infile;

int scan(struct token *t);
void openfile(char *filename);

#endif
