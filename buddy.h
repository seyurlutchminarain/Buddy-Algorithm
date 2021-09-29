
#ifndef buddy_h
# define buddy_h

//put your additional function headers here


#include <stddef.h>
#include <stdio.h>




struct head *new();
struct head *buddy(struct head*);
struct head *split(struct head*);
struct head *primary(struct head*);
void *hide(struct head*);
struct head *magic(void*);
int level(int);
void* balloc(size_t size);
void bfree(void* memory);
void dispblocklevel(struct head*);
void dispblockstatus(struct head*);
void blockinfo(struct head*);
void printflists();


#endif


