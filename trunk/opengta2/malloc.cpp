#include "opengta.h"
#include "stdlib.h"
#include "string.h"

#ifdef WIN32
	#include "windows.h"
#endif

Memory_Manager mem;

#define MAGIC 0xABadCafe
#define TWOINTS (2 * sizeof(int))
#define A_LOT 768*1024*1024

#ifdef _DEBUG
	#define PAD_FACTOR 2
#else
	#define PAD_FACTOR 1
#endif

void Memory_Manager::Initialize() {
	malloc_total = 0;
	malloc_cnt = 0;
	malloc_cnt_max = 0;
	malloc_max = 0;
	malloc_memory = 0;
	malloc_totalcount = 0;

	malloc_stringpoolentries = 64;
	malloc_stringpool = (char**)mem.alloc(malloc_stringpoolentries*sizeof(char*));
	malloc_stringpoolsz = (int*)mem.alloc(malloc_stringpoolentries*sizeof(int));
	malloc_curstringpoolsz = (int*)mem.alloc(malloc_stringpoolentries*sizeof(int));

	malloc_curstringpool = 0;
	malloc_stringpoolsz[0] = STRING_POOL_SIZE;
	malloc_stringpool[0] = (char*)mem.alloc(malloc_stringpoolsz[0]);
	malloc_curstringpoolsz[0] = 0;
}

void Memory_Manager::strpool_grow(int addEntries) {
	malloc_stringpoolentries += addEntries;
	malloc_stringpool = (char**)mem.realloc(malloc_stringpool,malloc_stringpoolentries*sizeof(char*));
	malloc_stringpoolsz = (int*)mem.realloc(malloc_stringpoolsz,malloc_stringpoolentries*sizeof(int));
	malloc_curstringpoolsz = (int*)mem.realloc(malloc_curstringpoolsz,malloc_stringpoolentries*sizeof(int));
}

void Memory_Manager::Deinitialize() {
	for (int i = 0; i <= malloc_curstringpool; i++) {
		mem.free(malloc_stringpool[i]);
	}
	int stringpool_size = 0;
	int stringpool_used = 0;
	int stringpool_lost = 0;
	for (int i = 0; i <= malloc_curstringpool; i++) { 
		stringpool_size += malloc_stringpoolsz[i];
		stringpool_used += malloc_curstringpoolsz[i];
		stringpool_lost += malloc_stringpoolsz[i] - malloc_curstringpoolsz[i];
	}

	mem.free(malloc_stringpool);
	mem.free(malloc_stringpoolsz);
	mem.free(malloc_curstringpoolsz);

	logWrite("\n\nMemory report:\n"
			"Peak allocations: %d\n"
			"Peak memory usage: %d bytes (%d KB)\n"
			"String pool usage: %d out of %d bytes (%.2f percent, lost %d bytes, %d entries used)\n"
			"Total amount of allocations: %d\n"
			"Average size of allocation call: %d bytes (%d KB)\n\n",
			malloc_cnt_max,malloc_max,malloc_max / 1024,
			stringpool_used,stringpool_size,(100.0f*stringpool_used) / stringpool_size,stringpool_lost,malloc_curstringpool+1,
			malloc_totalcount,malloc_totalmemory / malloc_totalcount,(malloc_totalmemory / malloc_totalcount) / 1024);

	if (malloc_cnt != 0) {
		logWrite("MEMORY LEAK: %d memory allocations remaining in memory!",malloc_cnt);
	}	
	if (malloc_memory != 0) {
		logWrite("MEMORY LEAK: %d bytes (%d KB) are still remaining in memory!",malloc_memory,malloc_memory/1024);
		getc(stdin);
	}
}

void* Memory_Manager::alloc(int sz) {
	//LockID lock = Thread.EnterLock("GBH_MemMgr");
	char *ptr;

	if (malloc_totalmemory > A_LOT) {
		logWrite("ERROR: Too much memory allocated (unsafe, stopping)");
		return 0;
	}

	malloc_totalcount++;

	malloc_total += sz;
	malloc_totalmemory += sz;
	if (malloc_total > malloc_max) malloc_max = malloc_total;
	
	ptr = (char*)malloc(PAD_FACTOR*sz+TWOINTS);
	if (!ptr) {
		//while (!ptr) {
			//Thread.Sleep(1.0f);
			//ptr = (char*)malloc(PAD_FACTOR*sz+TWOINTS);
		//}
		logError("Out of memory/allocation error!");
		return 0;
	}

	malloc_memory += PAD_FACTOR*sz+TWOINTS;
	logWritem("malloc: allocate %d bytes [ptr: %p] [usage: %d kb]",sz,ptr+TWOINTS,malloc_memory/1024);

	//store allocation size and array checks
	((int*)ptr)[0] = sz;
	((int*)ptr)[1] = MAGIC;
	for (int i = 0; i < PAD_FACTOR * sz; ++i)
		ptr[i + TWOINTS] = (char) (i ^ 0xDEADBEEF);

	++malloc_cnt;
	if (malloc_cnt > malloc_cnt_max) malloc_cnt_max = malloc_cnt;
	//memset(ptr,0,sz);

	//Thread.LeaveLock(lock);
	return (void*)(ptr+TWOINTS);
}

int Memory_Manager::size(void *ptr) {
	if (!ptr) {
		logWritem("msize: invalid pointer passed");
		breakpoint();
		return 0;
	}

	char* q = ((char*)ptr) - TWOINTS;
	if (!q) {
		logWritem("msize: invalid pointer passed");
		breakpoint();
		return 0;
	}

	return ((int*)q)[0];
}

void Memory_Manager::free(void *ptr) {
	//LockID lock = Thread.EnterLock("GBH_MemMgr");
	char* q;

	if (!ptr) {
		logWritem("mfree: [%p] invalid pointer passed",ptr);
		breakpoint();
		return;
	}

	q = ((char*)ptr) - TWOINTS;
	if (!q) {
		logWritem("mfree: [%p] invalid pointer passed",ptr);
		breakpoint();
		return;
	}

	{
		int n = ((int*)q)[0];
		int magic = ((int*)q)[1];

		*((int*)q) = 0; //avoid duplicate frees

		if (magic != MAGIC) {
			logWritem("mfree: [%p] invalid magic",ptr);
			breakpoint();
			return;
		}

		((int*)q)[1] = ~MAGIC;

		if (n < 0) {
			logWritem("mfree: [%p] invalid pointer size",ptr);
			breakpoint();
			return;
		}

		malloc_total -= n;

		if (malloc_total < 0) {
			logWrite("MEMORY LEAK: [%p] freeing up more memory than we should!",ptr);
			breakpoint();
			return;
		}

		//FIXME: toggle this checks in release-debug modes
		//check for writing past end of array
		for (int i = n; i < PAD_FACTOR * n; ++i)
			if (q[i+TWOINTS] != (char)(i ^ 0xDEADBEEF)) {
				logWrite("MEMORY LEAK: [%p] Array bounds overflow at %p, byte %d past end of array changed",ptr,q+i+TWOINTS,i - n + 1);
				breakpoint();
				return;
			}

		for (int i = 0; i < PAD_FACTOR * n; ++i) {
			q[i + TWOINTS] = (char)(i ^ 0xBEEFDEAD);
		}

		--malloc_cnt;
		if (malloc_cnt < 0) {
			logWrite("MEMORY LEAK: [%p] freeing up more times than allocated!",ptr);
			breakpoint();
			return;
		}

		if ((malloc_cnt == 0 && malloc_total > 0) || (malloc_cnt > 0 && malloc_total == 0)) {
			logWrite("MEMORY LEAK: [%p] lost %d bytes (or %d allocations)",ptr,malloc_total,malloc_cnt);
			breakpoint();
		}

		//free(q);
		::free(q);

		malloc_memory -= PAD_FACTOR*n+TWOINTS;
		logWritem("mfree: free %d bytes [ptr: %p] [usage: %d kb]",n,ptr,malloc_memory / 1024);
		//Thread.LeaveLock(lock);
	}
}

void* Memory_Manager::realloc(void* ptr, int sz) {
	void* tmp = mem.alloc(sz);
	int size = mem.size(ptr);
	if (sz > size) memcpy(tmp,ptr,size);
	else		   memcpy(tmp,ptr,sz);
	mem.free(ptr);
	return tmp;
}

char* Memory_Manager::alloc_string(int sz) {
	if (malloc_curstringpool == malloc_stringpoolentries) {
		logError("String pool overflow (internal table overflow)");
		return malloc_stringpool[0];
	}

	if (malloc_curstringpoolsz[malloc_curstringpool] + sz > malloc_stringpoolsz[malloc_curstringpool]) {
		malloc_curstringpool++;
		if (malloc_curstringpool >= malloc_stringpoolentries) {
			strpool_grow(32);
		} 

		logWritem("malloc_string: growing string pool...");
		malloc_stringpoolsz[malloc_curstringpool] = max(sz,STRING_POOL_SIZE);
		malloc_stringpool[malloc_curstringpool] = (char*)mem.alloc(malloc_stringpoolsz[malloc_curstringpool]);
		malloc_curstringpoolsz[malloc_curstringpool] = 0;
	}

	//Return string
	malloc_curstringpoolsz[malloc_curstringpool] += sz;
	return (char*)(malloc_stringpool[malloc_curstringpool] + malloc_curstringpoolsz[malloc_curstringpool] - sz);
}
