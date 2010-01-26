#ifndef DARRAY_H
#define DARRAY_H
#include "string.h"
#include "log.h"

//FIXME: make neater?
template<class dataType> struct DataArray {
	unsigned int Count;
	unsigned int AllocCount;
	dataType* Data;

	//void Add(dataType* data)	{ if ((!Data) || (!data)) return 0; memcpy(&Data[Count],data,sizeof(dataType)); Count++; return &Data[Count-1]; }
	//Add new element to array, and return its pointer (doesn't really allocate new members!)
	dataType* Add()				{ if (!Data) return 0; if (Count >= AllocCount) return 0; memset(&Data[Count],0,sizeof(dataType)); Count++; return &Data[Count-1]; }
	//Preallocate array to certain size (clears it)
	void Preallocate(uint count) { Count = 0; if (Data) { mem.free(Data); }  Data = (dataType*)mem.alloc(count*sizeof(dataType)); AllocCount = count; }
	//Reallocate array to certain size (changes allocated size, slow!)
	void Reallocate(uint count)  { if (!Data) return;  if (count < Count) Count = count; 
								  Data = (dataType*)mem.realloc(Data,count*sizeof(dataType)); AllocCount = count; }
	//Release array (frees up all resources)
	void Release()				{ if (Data) mem.free(Data); Data = 0; AllocCount = 0; }
	//Shrink array to hold only added elements
	void Shrink()				{ if (!Data) return; Data = (dataType*)mem.realloc(Data,Count*sizeof(dataType)); AllocCount = Count; }
	//Get array element by index
	dataType* operator[](unsigned int i)	{ 
#ifdef _DEBUG
		if (i >= AllocCount) { 
			logError("Out of bounds in data array [%p], indexing element %d out of %d allocated",Data,i,AllocCount); 
			return 0; 
		}
#endif
		return &Data[i]; }
	//Create empty data array
	void Create()				{ Count = 0; AllocCount = 0; Data = 0; }
	//Create of specified length
	void Allocate(int count)	{ Preallocate(count); Count = count; memset(Data,0,Count*sizeof(dataType)); }
	//Clear array (not physically)
	void Clear()				{ Count = 0; }
	//Set element (FIXME: make this overload [] operator)
	void Set(int index, dataType* ptr)	{ memcpy(&Data[index],ptr,sizeof(dataType)); }
	//Add all elements/make it static-like array
	void AddAll()				{ Count = AllocCount; /*memset(Data,0,sizeof(dataType)*AllocCount);*/ }
};

#endif