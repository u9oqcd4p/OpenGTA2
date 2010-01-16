#ifdef _DEBUG
#ifdef _DO_GAME_TEST

#include "opengta.h"
#include "opengta_cl.h"
#include "opengta_sv.h"

#define TEST_MAX_THREAD_NUMBER			8
#define TEST_ALLOCATION_CYCLES			0
#define TEST_ALLOCATIONS_IN_CYCLE		256
#define TEST_ALLOCATION_CYCLE_SIZE		1024
#define TEST_ALLOC_LARGE_CHUNK_MB		64
#define TEST_ALLOCATE_NUM_STRINGS		4096
#define TEST_ALLOCATE_MAX_STR_SIZE		8192

extern bool logDisableDebugOutput;





void GLFWCALL debugThread_Loop(void* t) {
	int tid = (int)t;
	logWrite("Thread %d starts",tid);
	for (int i = 0; i < 25; i++) {
		logWrite("Thread %d: iteration %d",tid,i);
	}
	logWrite("Thread %d end",tid);
}

void GLFWCALL debugThread_InfLoop(void* t) {
	logWrite("Infinite loop thread %d starts, thread ID 0x%X",(int)t,Thread.GetCurrentThreadID());
	while (1) Thread.Sleep();
}

void debugTestThreading() {
	logWrite("Testing thread creation and access to log file (and mutex locks)");
	ThreadID Thread1 = Thread.Create(debugThread_Loop,(void*)0);
	ThreadID Thread2 = Thread.Create(debugThread_Loop,(void*)1);

	Thread.WaitForThread(Thread1);
	Thread.WaitForThread(Thread2);
	logWrite("Threads completed, killing remains...");
	Thread.Kill(Thread1);
	Thread.Kill(Thread2);

	ThreadID randomThread[TEST_MAX_THREAD_NUMBER];
	logWrite("Creating and destroying %d random threads...",TEST_MAX_THREAD_NUMBER);
	for (int i = 0; i < TEST_MAX_THREAD_NUMBER; i++) {
		randomThread[i] = Thread.Create(debugThread_InfLoop,(void*)i);		
	}
	logWrite("Waiting...");
	Thread.Sleep(3.0f);
	logWrite("Killing threads...");
	for (int i = 0; i < TEST_MAX_THREAD_NUMBER; i++) {
		Thread.Kill(randomThread[i]);
	}
	logWrite("Waiting for remaining threads...");
	for (int i = 0; i < TEST_MAX_THREAD_NUMBER; i++) {
		Thread.WaitForThread(randomThread[i]);
	}

	logWrite("Found %d processors/cores",glfwGetNumberOfProcessors());

	logWrite("Creating mutex...");
	GLFWmutex mutex = glfwCreateMutex();
	logWrite("Mutex object pointer: 0x%.8X. Trying to lock and unlock",mutex);
	glfwLockMutex(mutex);
	glfwUnlockMutex(mutex);

	logWrite("Thread test passed!\n");
}





void debugTestDebug() {
	logWrite("Testing debugging...");
	logError("This line should trigger a breakpoint");
	assert(true);
	assert(false);
	logWrite("Log entry");
	logWritem("Log debug entry");

	logWrite("Debug test passed!\n");
}




void debugTestTimerLoop(float FPSLimit, float interval) {
	logWrite("FPS must be %.2f. dT must be %.4f seconds",FPSLimit,interval);

	logDisableDebugOutput = true;

	float startTime = curtime();
	int startFrames = Timer.totalFrames;

	Timer.Frame(); //First frame
	Timer.FPSLimit = FPSLimit;
	while ((curtime() - startTime < 2.0f) && (Timer.totalFrames - startFrames < 64)) {
		Thread.Sleep(interval);
		Timer.Frame();
		logWrite("Time: %.8f sec; dT: %.8f; FPS: %.4f",Timer.Time(),Timer.dT(),Timer.FPS());
		
		if (abs(Timer.dT() - interval) > 0.1f) {
			logError("Timer test failed: dT difference too high (> d0.1 sec)");
			return;
		}
		if ((FPSLimit > 0) && (abs(Timer.FPS() - FPSLimit) > 0.05f * FPSLimit)) {
			logError("Timer test failed: FPS difference too high (>5%, >%.5f sec)",0.05f * FPSLimit);
			return;
		}
	}

	logDisableDebugOutput = false;
}

void debugTestTimer() {
	logWrite("Testing timer. High delta times in tests might be clamped to slow down the game during time lag");
	logWrite("Log output to console will be inhibited during this test");

	debugTestTimerLoop(0.0f, 0.0f);
	debugTestTimerLoop(0.0f, 0.1f);
	debugTestTimerLoop(0.0f, 0.25f);
	debugTestTimerLoop(0.0f, 0.5f);
	debugTestTimerLoop(0.0f, 1.0f);

	debugTestTimerLoop(5.0f, 0.1f);
	debugTestTimerLoop(1.0f, 0.0f);
	debugTestTimerLoop(10.0f, 0.0f);
	debugTestTimerLoop(100.0f, 0.0f);
	debugTestTimerLoop(1000.0f, 0.0f);

	logWrite("Timer test passed!\n");
}




void debugTestMalloc() {
	logWrite("Testing memory allocation");
	float startTime;
	logDisableDebugOutput = true;

	void* ptrs[TEST_ALLOCATIONS_IN_CYCLE];
	for (int j = 0; j < TEST_ALLOCATION_CYCLES; j++) {
		logWrite("Memory alloc/free cycle %d, peak usage %d bytes",j,mem.malloc_max);
		startTime = curtime();

		for (int i = 0; i < TEST_ALLOCATIONS_IN_CYCLE; i++) {
			ptrs[i] = mem.alloc(TEST_ALLOCATION_CYCLE_SIZE);
			if (!ptrs[i]) {
				logError("Memory test failed: null block returned");
				return;
			}
		}

		for (int i = 0; i < TEST_ALLOCATIONS_IN_CYCLE; i++) mem.free(ptrs[i]);

		logWrite("Cycle time %.5f seconds",curtime() - startTime);
	}

	logWrite("Allocating large memory chunk (%d mb)",TEST_ALLOC_LARGE_CHUNK_MB);
	startTime = curtime();
	void* largePtr = mem.alloc(TEST_ALLOC_LARGE_CHUNK_MB*1024*1024);
	logWrite("Done, %.5f seconds",curtime() - startTime);
	if (!largePtr) {
		logError("Could not allocate large memory chunk");
	}

	logWrite("Filling with pattern...");
	startTime = curtime();
	for (int i = 0; i < TEST_ALLOC_LARGE_CHUNK_MB*1024*1024; i++) {
		((char*)largePtr)[i] = (i ^ 0xFF) & 0xFF;
	}
	logWrite("Done, %.5f seconds",curtime() - startTime);	
	mem.free(largePtr);


	logWrite("Testing string pool...");
	for (int i = 0; i < TEST_ALLOCATE_NUM_STRINGS; i++) {
		int strSize = Random.Int(1,TEST_ALLOCATE_MAX_STR_SIZE);
		char* strPtr = mem.alloc_string(strSize);
		if (mem.malloc_memory > 256*1024*1024) {
			logWrite("Too much memory used, terminating earlier with %d strings",i+1);
			break;
		}
		for (int j = 0; j < strSize; j++) strPtr[j] = (i ^ j) & 0xFF;
	}
	int stringpool_size = 0;
	int stringpool_used = 0;
	int stringpool_lost = 0;
	for (int i = 0; i <= mem.malloc_curstringpool; i++) { 
		stringpool_size += mem.malloc_stringpoolsz[i];
		stringpool_used += mem.malloc_curstringpoolsz[i];
		stringpool_lost += mem.malloc_stringpoolsz[i] - mem.malloc_curstringpoolsz[i];
		
		if ((mem.malloc_stringpoolsz[i] <= 0) || (mem.malloc_stringpoolsz[i] > 16384) ||
			(mem.malloc_curstringpoolsz[i] <= 0) || (mem.malloc_curstringpoolsz[i] > 16384)) {
			logError("Memory test failed: corrupted string pool block %d",i);
			return;
		}
	}

	logWrite("String pool usage: %d out of %d bytes (%.2f percent, lost %d bytes, %d entries used)",
			stringpool_used,stringpool_size,(100.0f*stringpool_used) / stringpool_size,stringpool_lost,mem.malloc_curstringpool+1);

	logWrite("Testing boundary overflow detection");
	char* testPtr = (char*)mem.alloc(512);
	testPtr[600] = 0x10;
	mem.free(testPtr);

	logWrite("Reinitializing memory (and console variables, they are in mem too)...");
	logWrite("You should see warning about memory leak");
	Convar.Deinitialize();
	Memory.Deinitialize();
	logDisableDebugOutput = false;
	Memory.Initialize();
	Convar.Initialize();

	logWrite("Memory allocation test passed!\n");
}




void debugTestMap() {
	logWrite("Testing map...");
	Map.Initialize();
	logDisableDebugOutput = true;

	logWrite("Precaching all maps...");
	for (uint i = 0; i < Map.mapsInfo.Count; i++) {
		logWrite("Precached map ID %d",Map.cellPrecache(Map.mapsInfo[i]->CX,Map.mapsInfo[i]->CY));
	}
	logWrite("Total memory usage: %.3f KB",mem.malloc_max / 1024.0f);
	logWrite("Uncaching all maps...");
	for (uint i = 0; i < Map.mapsInfo.Count; i++) {
		Map.cellUncache(Map.mapsInfo[i]->CX,Map.mapsInfo[i]->CY);
	}

	logWrite("Requesting random blocks in [0;0] [512;512] range...");
	for (int i = 0; i < 4096; i++) {
		Map.blockShouldExist(Random.Int(0,511),Random.Int(0,511));
	}

	logWrite("Feeding garbage into Map.blockShouldExist");
	for (int i = 0; i < 4096; i++) {
		Map.blockShouldExist(Random.Int(-0x7FFFFFF0,0x7FFFFFF0),
							 Random.Int(-0x7FFFFFF0,0x7FFFFFF0));
	}

	logDisableDebugOutput = false;
	Map.Deinitialize();
	logWrite("Map test passed!\n");
}

void cvar_write(int numParams, int* paramType, int* params) { 
	logWrite("Console variable written. Params: %d",numParams); 
	for (int i = 0; i < numParams; i++) {
		if (paramType[i] == CONVAR_FLOAT) {
			logWrite("Float[%d] = %.5f",i,((float*)params)[i]);
		} else {
			logWrite("Int  [%d] = %d",i,params[i]);
		}
	}
}
int cvar_read(int numParams, int* paramType, int* params) {
	logWrite("Console variable read");
	for (int i = 0; i < numParams; i++) {
		paramType[i] = CONVAR_FLOAT;
		((float*)params)[i] = Random.Float(0.0f,100.0f);
		logWrite("Float[%d] = %.5f",i,((float*)params)[i]);
	}
	return numParams;
}

#define PRINT_CVAR(name,tname) \
	logWrite(tname"  IsFloat: %d gInt: %d gFloat: %.5f iVar: %d fVar: %.5f", \
		Convar[name]->IsFloat(),Convar[name]->GetInt(),Convar[name]->GetFloat(),Convar[name]->iValue,Convar[name]->fValue);

void debugTestConsoleVars() {
	logWrite("Testing convars...");

	ConvarID cvar1 = Convar.Create("cvar1");
	ConvarID cvar2 = Convar.Create("cvar2",123.0f);
	ConvarID cvar3 = Convar.Create("cvar3",456);
	ConvarID cvar4 = Convar.Create("cvar4",cvar_read,cvar_write);

	logWrite("Initial cvar contents:");
	PRINT_CVAR(cvar1,"Var1")
	PRINT_CVAR(cvar2,"Var2")
	PRINT_CVAR(cvar3,"Var3")
	PRINT_CVAR(cvar4,"Var4")

	logWrite("Contents after setting stuff:");
	Convar[cvar1]->SetInt(Random.Int(111,999));
	PRINT_CVAR(cvar1,"Var1")
	Convar[cvar1]->SetFloat(Random.Float(111,999));
	PRINT_CVAR(cvar1,"Var1")

	Convar[cvar2]->SetInt(Random.Int(111,999));
	PRINT_CVAR(cvar2,"Var2")
	Convar[cvar2]->SetFloat(Random.Float(111,999));
	PRINT_CVAR(cvar2,"Var2")

	Convar[cvar3]->SetInt(Random.Int(111,999));
	PRINT_CVAR(cvar3,"Var3")
	Convar[cvar3]->SetFloat(Random.Float(111,999));
	PRINT_CVAR(cvar3,"Var3")

	Convar[cvar4]->SetInt(Random.Int(111,999));
	PRINT_CVAR(cvar4,"Var4")
	Convar[cvar4]->SetFloat(Random.Float(111,999));
	PRINT_CVAR(cvar4,"Var4")

	Convar.Create("cvar5");
	logWrite("Get CVAR1: %.5f",Convar.GetFloat("cvar1"));
	logWrite("Get CVAR5 with def 123: %.5f",Convar.GetFloat("cvar5",123));
	logWrite("Set CVAR1");
	Convar.SetFloat("cvar1",345);
	logWrite("Get CVAR1: %.5f",Convar.GetFloat("cvar1"));
	logWrite("Set CVAR6");
	logWrite("Get CVAR6 with def 123: %.5f",Convar.GetFloat("cvar6",123));
	Convar.SetFloat("cvar6",456456);
	logWrite("Get CVAR6: %.5f",Convar.GetFloat("cvar6"));

	logWrite("(cvar5 was created uninitialized, cvar6 was created implictly)");


	logWrite("Convar test passed!\n");
}



void debugDoTest() {
	logWrite("Running internal game tests");
	//debugTestThreading();
	//debugTestDebug();
	//debugTestTimer();
	//debugTestMalloc();
	//debugTestMap();
	//debugTestConsoleVars();

	while (1) Thread.Sleep();
}


#endif
#endif
