#ifndef MAP_H
#define MAP_H

//#include "opengta.h"

#define MAX_TRACE_BLOCKS 16

//unsigned 
typedef unsigned int MapID;
typedef unsigned int MapInfoID;

struct rmp_entry {
	char Filename[256];
	int CX; int CY;
};

struct Map_Manager {
	//Precache this cell (loads map structure if needed) FIXME: would be neat if it was async
	MapID cellPrecache(int cx, int cy);
	//Check if this cell is in cache
	bool cellIsCached(int cx, int cy);
	//Retrieve cell index in cache (-1 if its not in cache)
	MapID cellCacheIndex(int cx, int cy);
	//Retrieve cell index in cell entries table
	MapInfoID cellEntryIndex(int cx, int cy);
	//Remove this cell from cache
	void cellUncache(int cx, int cy);
	//Release map and its resources
	void releaseMap(rmp_cityscape* cityscape);
	//Retrieve cityscape from cache (may return invalid pointer!)
	rmp_cityscape* cellCityscape(int cx, int cy);

	//Assure that this block exists (precache required cells if it doesn't)
	void blockShouldExist(int bx, int by);
	//Assure that this plane exists (precache required cells if it doesn't)
	void planeShouldExist(int bx1, int by1, int bx2, int by2);

	//Get block by coordinate
	rmp_block_info* GetBlock(int X, int Y, int Z);
	rmp_block_info* GetBlock(Vector3f pos);
	//Get cityscape by coordinate
	rmp_cityscape* GetCityscape(int X, int Y);
	rmp_cityscape* GetCityscape(Vector2f pos);
	rmp_cityscape* GetCityscape(Vector3f pos);
	//Get max height by coordinate
	int   GetZ(int X, int Y, bool ignoreFlat = false);
	float GetZ(Vector2f pos, bool ignoreFlat = false);
	float GetZ(Vector3f pos, bool ignoreFlat = false);


	//Check if point is inside the map
	bool isSolid(Vector3f pos);
	//Check if segment intersects with map
	bool traceSegment(Vector3f start, Vector3f end, Vector3f* collisionPoint);
	//Check if ray interescts with map
	//bool traceRay(Vector3f start, Vector3f dir, Vector3f* collisionPoint);

	//Trace list
	struct traceListStruct {
		rmp_block_info* Blocks[MAX_TRACE_BLOCKS];
		Vector3f BlockOffset[MAX_TRACE_BLOCKS];

		int BlocksCount;
		int CheckedCount;

		void Reset();
		bool AddBlock(Vector3f current, rmp_block_info* block);
	} traceList;

	
	//Initialize cache and various internal structures
	void Initialize();
	//Deinitialize and free up all resources
	void Deinitialize();
	//Do various cache checks (load missing maps, restructure quick lookup)
	void Frame();

	//Load map into target buffer
	rmp_cityscape* LoadMap(MapInfoID idx);
	rmp_cityscape* LoadEmpty();

	//Cityscape cache
	DataArray<rmp_cityscape*> cacheCityscape;

	//Maps information (about all maps in data folder)
	DataArray<rmp_entry> mapsInfo;
};

extern Map_Manager Map;

#endif