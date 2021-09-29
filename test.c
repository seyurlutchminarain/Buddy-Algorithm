#include "buddy.h"
#include "buddy.c"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main(){

void *init = sbrk(0);
void *current;
printf("The initial top of the heap is %p.\n", init);

struct head* block = new();
blockinfo(block);

current = sbrk(0);
int allocated = (int)((current - init) / 1024);
printf("The current top of the heap is %p. \n", current);
printf("increased by %d Kbyte\n", allocated);

struct head* bud = buddy(block);
blockinfo(bud);

struct head* splitted = split(block);
blockinfo(splitted);

struct head* primes = primary(splitted);
blockinfo(primes);
struct head* primeb = primary(block);
blockinfo(primeb);

struct head* hiddenblockhead = hide(block);
blockinfo(hiddenblockhead);

struct head* unhiddenblockhead = magic(hiddenblockhead);
blockinfo(unhiddenblockhead);

int leveltest1 = level(32);
printf("min required level = %d\n",leveltest1);
int leveltest2 = level(100);
printf("min required level = %d\n",leveltest2);
int leveltest3 = level(1500);
printf("min required level = %d\n",leveltest3);


return 0;
}
