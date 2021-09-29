
#include "buddy.h"
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>



//Define your Constants here
#define MIN 5
#define LEVELS 8
#define PAGE 4096 






enum AEflag { Free, Taken };

struct head {
	enum AEflag status;
	short int level;
	struct head* next;
	struct head* prev; 
};

struct head* flists [LEVELS] = {NULL};// Check if needs NULL initialization !!

//Complete the implementation of new here
struct head *new() {

struct head *new = (struct head *) mmap(NULL,PAGE,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);

if (new == MAP_FAILED)
{
	return NULL;
}
assert(((long int)new & 0xfff) == 0); // last 12 bits have to be 0??
new->status = Free;
new->level = LEVELS - 1;
return new;


}

//Complete the implementation of level here
int level(int req) {

	if (req  > 0 && req <= 32)
	{
		return 0;
	} 
	else if (req > 32 && req <= 64)
	{
		return 1;
	}
	else if (req > 64 && req <= 128)
	{
		return 2;
	}
	else if (req > 128 && req <= 256)
	{
		return 3;
	}
	else if (req > 256 && req <= 512)
	{
		return 4;
	}
	else if (req > 512 && req <= 1024)
	{
		return 5;
	}
	else if (req > 1024 && req <= 2048)
	{
		return 6;
	}
	else if (req > 2048 && req <= 4096)
	{
		return 7;
	}
}

void* balloc(size_t size) {
	
	if(size == 0){
		return NULL;
	}

	int lvl = level(size);
	int unalloc_level = 7;

	struct head*unallocated = NULL;
	struct head*unalloc_hidden;

	
	if(flists[lvl] != NULL){

		if(flists[lvl]->status == Free){
			flists[lvl]->status = Taken;
			unalloc_hidden = hide(flists[lvl]);
			return unalloc_hidden;
		}
		else{
			struct head*temp = flists[lvl];
			while(temp->prev != NULL){
				temp = temp->prev;

				if(temp->status == Free){
					temp->status = Taken;
					unalloc_hidden = hide(temp);
					return unalloc_hidden;
				}
			}
		}
	}

	for(int i = lvl+1 ; i < 7 ; i ++){
		if(flists[i] != NULL){
			if(flists[i]->status == Free){
				unalloc_level = i;
				unallocated = flists[i];
				break;
			}
		}
	}

	
	if(unallocated == NULL){
		unallocated = new();
		if(flists[unalloc_level] != NULL){
			flists[unalloc_level]->next = unallocated;
			unallocated->prev = flists[unalloc_level];
			flists[unalloc_level] = unallocated;
		}
		else{
			flists[unalloc_level] = unallocated;
		}
	}
	


	for(int i = unalloc_level ; i > lvl ; i--){

		unallocated = flists[i];
		
		if(unallocated->prev != NULL){
			flists[i] = unallocated->prev;
			flists[i]->next = NULL;
		}	
		else{
			flists[i] = NULL;
		}

		struct head*temp_split = split(unallocated);

		temp_split->prev = unallocated;
		unallocated->next = temp_split;

		temp_split->level = (i-1);
		unallocated->level = (i-1);

		if(flists[i-1] != NULL){
			flists[i-1]->next = unallocated;
			unallocated->prev = flists[i-1];
			flists[i-1] = temp_split;
			
		}	
		else{
			unallocated->prev = NULL;
			flists[i-1] = temp_split;
		}

	}
	
	
	unallocated->status = Taken;
	unalloc_hidden = hide(unallocated);
	return unalloc_hidden;
}


//Complete the implementation of bfree here
void bfree(void* memory) {
	
	if(memory == NULL){
		return;
	}
	
	struct head* block = magic(memory);
	block->status = Free;

	int lvl = block->level;
	
	struct head* block_buddy = buddy(block);
	struct head* blocksPrimary = primary(block_buddy);
	struct head*temp = blocksPrimary;
	
	while(lvl < 6){

		
		if(block_buddy->status == Taken){
			
			break;
		}
		

		if(blocksPrimary->prev == NULL){
			if(blocksPrimary->next != NULL){
				if(blocksPrimary->next->next == NULL){
					
					flists[lvl] = NULL;
				}
				else{
					
					blocksPrimary->next->next->prev = NULL;
					
				}
			}
			
		}
		else{
			
			if(blocksPrimary->next != NULL){
				if(blocksPrimary->next->next == NULL){
					blocksPrimary->prev->next = NULL;
					flists[lvl] = blocksPrimary->prev;
				}
				else{
					blocksPrimary->prev->next = blocksPrimary->next->next;
					blocksPrimary->next->next->prev = blocksPrimary->prev;
					
				}
			}
		}
		blocksPrimary->level +=1;
		block_buddy = buddy(blocksPrimary);
		
		
		blocksPrimary->next = NULL;
		blocksPrimary->prev = NULL;

		
		
		
		if(block_buddy->status == Taken){
			if(flists[blocksPrimary->level]==NULL){
				flists[blocksPrimary->level] = temp;
			}
			else{
			
				flists[blocksPrimary->level]->next = temp;
				temp->prev = flists[blocksPrimary->level];
				flists[blocksPrimary->level] = temp;
			}
			
		}
		else{
			temp=blocksPrimary;
			blocksPrimary = primary(block_buddy);
			if(temp == blocksPrimary){
			
				if(block_buddy->prev == NULL){
					blocksPrimary->next = block_buddy;
					block_buddy->prev = blocksPrimary;
					temp->prev = NULL;
				}
			}
			else{
				
				if(block_buddy->next == NULL){
					block_buddy->next = temp;
					temp->prev = block_buddy;
					temp->next = NULL;
				}
			}
		}
		

	
		lvl++;
	}
	
	

	block_buddy = buddy(block_buddy);
	
	

	if(block_buddy->status == Free && blocksPrimary->level==6){

		blocksPrimary = primary(block_buddy);

		if(blocksPrimary->prev == NULL){
			if(blocksPrimary->next->next == NULL){
				flists[lvl] = NULL;
			}
			else{
				blocksPrimary->next->next->prev = NULL;
			}
		}
		else{
			if(blocksPrimary->next->next ==NULL){
				blocksPrimary->prev->next = NULL;
				flists[blocksPrimary->level] = blocksPrimary->prev;
			}
			else{
				blocksPrimary->prev->next = blocksPrimary->next->next;
				blocksPrimary->next->next->prev = blocksPrimary->prev;
			}
		}
		blocksPrimary->level += 1;
		blocksPrimary->next = NULL;
		blocksPrimary->prev = NULL;

		if(flists[blocksPrimary->level] == NULL){
			flists[blocksPrimary->level] = blocksPrimary;
		}
		else{
			flists[blocksPrimary->level]->next = blocksPrimary;
			flists[blocksPrimary->level] = blocksPrimary;
		}	
		
	}


	
}


//Helper Functions
struct head* buddy(struct head* block) {
	int index = block->level;
	long int mask = 0x1 << (index + MIN);
	return (struct head*)((long int)block ^ mask);

}

struct head* split(struct head* block) {
	int index = block->level - 1;
	int mask = 0x1 << (index + MIN);
	return (struct head*)((long int)block | mask);
}

struct head* primary(struct head* block) {
	int index = block->level;
	long int mask = 0xffffffffffffffff << (1 + index + MIN);
	return (struct head*)((long int)block & mask);
}

void* hide(struct head* block) {
	return (void*)(block + 1);
}


struct head* magic(void* memory) {
	return ((struct head*)memory - 1);
}
	


void dispblocklevel(struct head* block){
	printf("block level = %d\n",block->level);
}
void dispblockstatus(struct head* block){
	printf("block status = %d\n",block->status);
}

void blockinfo(struct head* block){
	printf("===================================================================\n");
	dispblockstatus(block);
	dispblocklevel(block);
	printf("start of block in memory: %p\n", block);
	printf("size of block in memory: %ld in bytes\n",sizeof(struct head));
	printf("===================================================================\n");
}

void printflists() {
	for (int i = 0; i < 8; i++)
	{
		if (flists[i] != NULL)
		{
			printf("========================%d========================\n",i);
			struct head* temp = flists[i];
			blockinfo(temp);
		}
	}
}






