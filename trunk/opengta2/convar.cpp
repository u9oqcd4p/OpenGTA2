#include "opengta.h"

Convar_Manager Convar;

#define CONVAR_STRING_LENGTH 256

int concmd_cvar_list(int numParams, int* paramType, int* params) {
	LockID lockID = Thread.EnterLock(MUTEX_CONSOLE);
		logWrite("Convar list");
		for (uint i = 0; i < Convar.List.Count; i++) {
			if (Convar[i]->writeCallback) {
				logWrite("[Command] %s",Convar[i]->namePtr);
			} else if (Convar[i]->IsFloat()) {
				logWrite("[Float] %s = %.5f",Convar[i]->namePtr,Convar[i]->fValue);
			} else {
				logWrite("[Int] %s = %d",Convar[i]->namePtr,Convar[i]->iValue);
			}
		}
	Thread.LeaveLock(lockID);
	return 0;
}

int concmd_version(int numParams, int* paramType, int* params) {
	logWrite("Engine: %s",ENGINE_VERSION);
	return 0;
}

void concmd_echo(int numParams, int* paramType, int* params) {
	for (int i = 0; i < numParams; i++) {
		if (paramType[i] == CONVAR_FLOAT) {
			logWrite("%f",((float*)params)[i]);
		} else if (paramType[i] == CONVAR_INT) {
			logWrite("%d",params[i]);
		} else if (paramType[i] == CONVAR_CSTRING) {
			logWrite("%s",(char*)(params[i]));
		}
	}
}

void Convar_Manager::Initialize() {
	List.Preallocate(MAX_CONVARS);
	memset(&invalid_entry,0,sizeof(convar_entry));
	invalid_entry.namePtr = mem.alloc_string(1);
	invalid_entry.namePtr[0] = 0;

	Create("cvar_list",concmd_cvar_list,0);
	Create("version",concmd_version,0);

	Create("echo",0,concmd_echo);
}

void Convar_Manager::Deinitialize() {
	List.Release();
}





//FIXME: support for sending convars over MP if setting convar with type CONVAR_UNKNOWN
ConvarID Convar_Manager::GetID(const char* convarName) {
	LockID lockID = Thread.EnterLock(MUTEX_CONSOLE);
	for (uint i = 0; i < List.Count; i++) {
		if (utf8casecmp(convarName,List[i]->namePtr) == 0) {
			Thread.LeaveLock(lockID);
			return i;
		}
	}
	Thread.LeaveLock(lockID);
	return BAD_ID;
}





ConvarID Convar_Manager::Create(const char* convarName) {
	convar_entry* convar = List.Add();
	if (!convar) return BAD_ID;

	convar->namePtr = mem.alloc_string(strlen(convarName)+1);
	strcpy(convar->namePtr,convarName);

	convar->fValue = 0 ;
	convar->iValue = 0;
	convar->readCallback = 0;
	convar->writeCallback = 0;
	convar->isInitialized = 0;
	
	return List.Count-1;
}

ConvarID Convar_Manager::Create(const char* convarName, float def) {
	ConvarID cvarid = Create(convarName);
	if (cvarid == BAD_ID) return BAD_ID;
	convar_entry* convar = List[cvarid];

	convar->SetFloat(def);
	return cvarid;
}

ConvarID Convar_Manager::Create(const char* convarName, int def) {
	ConvarID cvarid = Create(convarName);
	if (cvarid == BAD_ID) return BAD_ID;
	convar_entry* convar = List[cvarid];

	convar->SetInt(def);
	return cvarid;
}

ConvarID Convar_Manager::Create(const char* convarName, ConvarReadFunction* readCallback, ConvarWriteFunction* writeCallback) {
	ConvarID cvarid = Create(convarName);
	if (cvarid == BAD_ID) return BAD_ID;
	convar_entry* convar = List[cvarid];

	convar->readCallback = readCallback;
	convar->writeCallback = writeCallback;
	return cvarid;
}





float		Convar_Manager::GetFloat(const char* convarName, float def) { 
	ConvarID cvarid = GetID(convarName);
	if (cvarid == BAD_ID) cvarid = Create(convarName);
	return Convar[GetID(convarName)]->GetFloat(def); 
}
int			Convar_Manager::GetInt(const char* convarName, int def) { 
	ConvarID cvarid = GetID(convarName);
	if (cvarid == BAD_ID) cvarid = Create(convarName);
	return Convar[GetID(convarName)]->GetInt(def); 
}

float		Convar_Manager::GetFloat(const char* convarName) { return Convar[GetID(convarName)]->GetFloat(); }
int			Convar_Manager::GetInt(const char* convarName) { return Convar[GetID(convarName)]->GetInt(); }
float		Convar_Manager::GetFloat(ConvarID convar) { return Convar[convar]->GetFloat(); }
int			Convar_Manager::GetInt(ConvarID convar) { return Convar[convar]->GetInt(); }
void		Convar_Manager::SetFloat(ConvarID convar, float f) { Convar[convar]->SetFloat(f); }
void		Convar_Manager::SetInt(ConvarID convar, int i) { Convar[convar]->SetInt(i); }

void		Convar_Manager::SetFloat(const char* convarName, float value) { 
	ConvarID cvarid = GetID(convarName);
	if (cvarid == BAD_ID) cvarid = Create(convarName);
	Convar[GetID(convarName)]->SetFloat(value); 
}
void		Convar_Manager::SetInt(const char* convarName, int value) { 
	ConvarID cvarid = GetID(convarName);
	if (cvarid == BAD_ID) cvarid = Create(convarName);
	Convar[GetID(convarName)]->SetInt(value); 
}

float Convar_Manager::getFloatParameter(int index, int numParams, int* paramType, int* params) {
	if (index < numParams) {
		if (paramType[index] == CONVAR_FLOAT) {
			return ((float*)params)[index];
		} else {
			return (float)params[index];
		}
	} else return 0.0f;
}

int	Convar_Manager::getIntParameter(int index, int numParams, int* paramType, int* params) {
	if (index < numParams) {
		if (paramType[index] == CONVAR_FLOAT) {
			return (int)((float*)params)[index];
		} else {
			return params[index];
		}
	} else return 0;
}

char* Convar_Manager::getCStringParameter(int index, int numParams, int* paramType, int* params) {
	if (index < numParams) {
		if (paramType[index] == CONVAR_CSTRING) {
			return (char*)((params)[index]);
		} else {
			return 0;
		}
	} else return 0;
}

void Convar_Manager::setFloatParameter(int index, int numParams, int* paramType, int* params, float value) {
	if (index < numParams) {
		paramType[index] = CONVAR_FLOAT;
		((float*)params)[index] = value;
	}
}

void Convar_Manager::setIntParameter(int index, int numParams, int* paramType, int* params, int value) {
	if (index < numParams) {
		paramType[index] = CONVAR_INT;
		params[index] = value;
	}
}

void Convar_Manager::setCStringParameter(int index, int numParams, int* paramType, int* params, char* value) {
	if (index < numParams) {
		paramType[index] = CONVAR_CSTRING;
		params[index] = (int)value;
	}
}


#ifndef DEDICATED_SERVER
	#include "opengta_cl.h"
#endif

//stolen from HybriC
bool tofloat(char* s, int n, float* result) {
	bool dprec;
	float frac;
	int sign;

	*result = 0; frac = 0;
	dprec = false;
	if (*s == '-') {
		*s++; --n;
		sign = -1;
	} else sign = 1;

	while (n && *s) {
		if (*s == '.') { //decimal comma
			dprec = true;
			*s++; --n;
		}
		if ((*s >= '0') && (*s <= '9')) {
			if (dprec == false) *result = *result * 10.0f + (*s - '0');
			else				frac = frac * 10.0f + (*s-'0');
		}
		else return false;
		*s++; --n;
	}
	//if (!dprec) return false;
	while (frac > 1.0f) frac = frac / 10;
	*result = (*result + frac)*sign;
	return true;
}

bool tointeger(char* s, int n, int* result) {
	int sign;
	*result = 0;
	if (*s == '-') {
		*s++; --n;
		sign = -1;
	} else sign = 1;

	while (n && *s) {
		if ((*s >= '0') && (*s <= '9')) *result = *result *10 + (*s - '0');
		else return false;
		*s++; --n;
	}

	*result = *result * sign;
	return true;
}


void Convar_Manager::Execute(char* request) {
	char convarName[256];
	char convarParam[256*8];

	char* requestPtr = request;
	if (sscanf(requestPtr,"%255s",convarName)) requestPtr += strlen(convarName);
	int cvarid = GetID(convarName);
	if (cvarid == BAD_ID) {
		#ifndef DEDICATED_SERVER
			Console.printf("\001Unknown convar/command: %s",convarName);
		#endif
		return;
	}

	int paramIdx = 0;
	if (sscanf(requestPtr,"%255s",&convarParam[0]) > 0) {
		requestPtr += strlen(&convarParam[0])+1;
		while (true) {
			paramIdx++;
			if (paramIdx > 8) {
				logError("FIXME: Unimplemented feature");
				break;
			}
			if (sscanf(requestPtr,"%255s",&convarParam[paramIdx*256]) > 0) requestPtr += strlen(&convarParam[paramIdx*256])+1;
			else break;
		}
	}

	if (paramIdx) {
		int params[8];
		int paramType[8];
		for (int i = 0; i < paramIdx; i++) {
			if (tofloat(&convarParam[i*256],strlen(&convarParam[i*256]),(float*)&params[i])) {
				paramType[i] = CONVAR_FLOAT;
			} else if (tointeger(&convarParam[i*256],strlen(&convarParam[i*256]),&params[i])) {
				paramType[i] = CONVAR_INT;
			} else {
				params[i] = (int)&convarParam[i*256];
				paramType[i] = CONVAR_CSTRING;
			}
		}
		
		if (List[cvarid]->writeCallback) {
			List[cvarid]->writeCallback(paramIdx,paramType,params);
		} else {
			if (paramType[0] == CONVAR_FLOAT) SetFloat(cvarid,((float*)params)[0]);
			if (paramType[0] == CONVAR_INT) SetInt(cvarid,params[0]);
		}
	} else {
		if (List[cvarid]->readCallback) {
			int params[8];
			int paramType[8];
			int numParams = List[cvarid]->readCallback(8,paramType,params);

			for (int i = 0; i < numParams; i++) {
				if (paramType[i] == CONVAR_FLOAT) {
					logWrite("%s = %f",convarName,((float*)params)[i]);
				} else if (paramType[i] == CONVAR_INT) {
					logWrite("%s = %d",convarName,params[i]);
				} else if (paramType[i] == CONVAR_CSTRING) {
					logWrite("%s = ""%s""",convarName,(char*)(params[i]));
				}
			}
		} else {
			#ifndef DEDICATED_SERVER
				if (List[cvarid]->IsFloat()) {
					Console.printf("%s = %f\n",convarName,GetFloat(cvarid));
				} else {
					Console.printf("%s = %d\n",convarName,GetInt(cvarid));	
				}
			#endif	
		}
	}
}







float convar_entry::GetFloat(float def) {
	if (readCallback) {
		int paramType[8];
		int params[8];

		int paramcount = readCallback(1,paramType,params);
		if (paramcount >= 1) {
			if (paramType[0] == CONVAR_FLOAT) {
				return *((float*)&params[0]);
			} else if (paramType[0] == CONVAR_INT) {
				return (float)params[0];
			}
		}
	}

	if (!isInitialized) return def;
	return fValue;
}

int convar_entry::GetInt(int def) {
	if (readCallback) {
		int paramType[8];
		int params[8];

		int paramcount = readCallback(1,paramType,params);
		if (paramcount >= 1) {
			if (paramType[0] == CONVAR_FLOAT) {
				return (int)(*((float*)&params[0]));
			} else if (paramType[0] == CONVAR_INT) {
				return params[0];
			}
		}
	}

	if (!isInitialized) return def;
	return iValue;
}

void convar_entry::SetFloat(float f) {
	if (writeCallback) {
		int paramType = CONVAR_FLOAT;
		writeCallback(1, &paramType, (int*)&f);
		return;
	}

	iValue = (int)f;
	fValue = f;
	isInitialized = 1;
}

void convar_entry::SetInt(int i) {
	if (writeCallback) {
		int paramType = CONVAR_INT;
		writeCallback(1, &paramType, &i);
		return;
	}

	iValue = i;
	fValue = (float)i;
	isInitialized = 1;
}