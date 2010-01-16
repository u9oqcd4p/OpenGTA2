#ifndef CONVAR_H
#define CONVAR_H

typedef unsigned int ConvarID;
//numParams - how much params it can write; returns how much it actually wrote
//(numParams > 0) is always true for read
typedef int  (ConvarReadFunction)(int numParams, int* paramType, int* params);
typedef void (ConvarWriteFunction)(int numParams, int* paramType, int* params);

#define CONVAR_FLOAT	0
#define CONVAR_INT		1
#define CONVAR_CSTRING	2

struct convar_entry {
	char* namePtr;

	bool isInitialized;

	float fValue;
	int iValue;

	ConvarReadFunction* readCallback;
	ConvarWriteFunction* writeCallback;

	float		GetFloat(float def = 0.0f);
	int			GetInt(int def = 0);
	void		SetFloat(float f);
	void		SetInt(int i);

	bool		IsFloat() { return ((float)iValue != fValue); }
};

struct Convar_Manager {
	void Initialize();
	void Deinitialize();

	//Get ConvarID by name
	ConvarID	GetID(const char* convarName);

	//Create convar with default value
	ConvarID	Create(const char* convarName);
	ConvarID	Create(const char* convarName, float def);
	ConvarID	Create(const char* convarName, int def);
	ConvarID	Create(const char* convarName, ConvarReadFunction* readCallback, ConvarWriteFunction* writeCallback);

	//Get convar value
	float		GetFloat(const char* convarName, float def);
	int			GetInt(const char* convarName, int def);

	//Get convar value by convar name
	float		GetFloat(const char* convarName);
	int			GetInt(const char* convarName);

	//Get convar value by ConvarID
	float		GetFloat(ConvarID convar);
	int			GetInt(ConvarID convar);

	//Set convar value by name
	void		SetFloat(const char* convarName, float value);
	void		SetInt(const char* convarName, int value);

	//Set convar value by ConvarID
	void		SetFloat(ConvarID convar, float f);
	void		SetInt(ConvarID convar, int i);

	//Execute request
	void		Execute(char* request);

	//Utility functions to read convars
	float		getFloatParameter(int index, int numParams, int* paramType, int* params);
	int			getIntParameter(int index, int numParams, int* paramType, int* params);
	char*		getCStringParameter(int index, int numParams, int* paramType, int* params);
	void		setFloatParameter(int index, int numParams, int* paramType, int* params, float value);
	void		setIntParameter(int index, int numParams, int* paramType, int* params, int value);
	void		setCStringParameter(int index, int numParams, int* paramType, int* params, char* value);

	convar_entry invalid_entry;
	DataArray<convar_entry> List;
	convar_entry* operator[](uint i) { 
			if ((i == BAD_ID) || (i >= List.Count)) {
				logError("Reading from non-existing console variable");
				memset(&invalid_entry,0,sizeof(convar_entry));
				return &invalid_entry; 
			} else return List[i]; 
		}
};

#define CVAR_GET_FLOAT(index) Convar.getFloatParameter(index,numParams,paramType,params)
#define CVAR_GET_INT(index) Convar.getIntParameter(index,numParams,paramType,params)
#define CVAR_GET_CSTRING(index) Convar.getCStringParameter(index,numParams,paramType,params)

#define CVAR_SET_FLOAT(index,value) Convar.setFloatParameter(index,numParams,paramType,params,value)
#define CVAR_SET_INT(index,value) Convar.setIntParameter(index,numParams,paramType,params,value)
#define CVAR_SET_CSTRING(index,value) Convar.setCStringParameter(index,numParams,paramType,params,value)

extern Convar_Manager Convar;

#endif