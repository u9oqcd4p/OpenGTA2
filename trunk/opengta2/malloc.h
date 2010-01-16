#ifndef MALLOC_H
#define MALLOC_H

struct Memory_Manager {
	int malloc_total;
	int malloc_cnt;
	int malloc_max;
	int malloc_cnt_max;
	int malloc_memory;
	int malloc_totalcount;
	int malloc_totalmemory;

	int malloc_stringpoolentries;
	char** malloc_stringpool;
	int* malloc_stringpoolsz;
	int* malloc_curstringpoolsz;
	int malloc_curstringpool;

	// "Memory" interface:
	//Initialize memory manager
	void Initialize();
	//Deinitialize memory manager and check for memory leaks
	void Deinitialize();

	//Grow string pool
	void strpool_grow(int addEntries);

	// "mem" interface:
	//Allocate sz bytes
	void*   alloc(int sz);
	//Free pointer
	void    free(void* ptr);
	//Reallocate pointer to new size
	void* realloc(void* ptr, int sz);
	//Get pointer size
	int		size(void* ptr);
	//Allocates string literal. DO NOT FREE THIS WITH mem.free!
	//This one is automatically freed upon game end.
	char*   alloc_string(int sz);
};

extern Memory_Manager mem;

#define Memory mem

#endif