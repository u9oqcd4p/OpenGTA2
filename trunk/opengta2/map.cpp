#include "opengta.h"

#include "stdio.h"
#include "string.h"

#include "sys/types.h"
#include "dirent.h"

Map_Manager Map;

rmp_cityscape* empty_cityscape = 0;

MapID Map_Manager::cellPrecache(int cx, int cy) {
	LockID lockID = Thread.EnterLock(MUTEX_MAP);
		if (cellIsCached(cx,cy)) { //FIXME: twice calling cache index, BAD
			//logWritem("[%d;%d] Warning: map was requested to be precached twice!",cx,cy);
			Thread.LeaveLock(lockID);
			return cellCacheIndex(cx,cy);
		}
	
		if (cacheCityscape.Count == cacheCityscape.AllocCount) {
			//FIXME: Pick random to uncache
			cellUncache((*cacheCityscape[0])->CX,(*cacheCityscape[0])->CY);
		}
	
		MapInfoID idx = cellEntryIndex(cx,cy);
		if (idx == BAD_ID) { //This map doesn't exist
			logWrite("Precaching empty map: [%d;%d]",cx,cy);
			rmp_cityscape** city = cacheCityscape.Add();

			*city = LoadEmpty(); //Loads a new empty map
			(*city)->CX = cx;
			(*city)->CY = cy;
		} else {
			logWrite("Precaching map from disk: [%d;%d]",cx,cy);
			rmp_cityscape** city = cacheCityscape.Add();

			*cacheCityscape[cacheCityscape.Count-1] = LoadMap(idx);
		}
		//FIXME: error messages and handling
		//if (!city) logWrite("Warning: you really should get more RAM, because something didn't fit (or maybe there was some other error)");
	Thread.LeaveLock(lockID);
	return cacheCityscape.Count-1;
}

MapInfoID Map_Manager::cellEntryIndex(int cx, int cy) {
	for (uint i = 0; i < mapsInfo.Count; i++) {
		if ((mapsInfo[i]->CX == cx) && (mapsInfo[i]->CY == cy)) return i;
	}
	return BAD_ID;
}

bool Map_Manager::cellIsCached(int cx, int cy) {
	return cellCacheIndex(cx,cy) != BAD_ID;
}

MapID Map_Manager::cellCacheIndex(int cx, int cy) {
	LockID lockID = Thread.EnterLock(MUTEX_MAP);
		for (uint i = 0; i < cacheCityscape.Count; i++) {
			if (((*cacheCityscape[i])->CX == cx) && ((*cacheCityscape[i])->CY == cy)) {
				Thread.LeaveLock(lockID);
				return i;
			}
		}
	Thread.LeaveLock(lockID);
	return BAD_ID;
}

void Map_Manager::releaseMap(rmp_cityscape* cityscape) {
	if (cityscape->rmp_city_scape) mem.free(cityscape->rmp_city_scape);
	if (cityscape->rmp_interior_scape) mem.free(cityscape->rmp_interior_scape);
	if (cityscape->rmp_columns) mem.free(cityscape->rmp_columns);
	mem.free(cityscape);
}

void Map_Manager::cellUncache(int cx, int cy) {
	LockID lockID = Thread.EnterLock(MUTEX_MAP);
		int idx = cellCacheIndex(cx,cy);
		if (idx == BAD_ID) {
			logWritem("cellUncache: map [%d;%d] is not cached",cx,cy);
			Thread.LeaveLock(lockID);
			return;
		}
	
		releaseMap(*cacheCityscape[idx]);
		cacheCityscape.Count--;
		if (cacheCityscape.Count > 0) {
			*cacheCityscape[idx] = *cacheCityscape[cacheCityscape.Count];
		}
		logWritem("cellUncache: map [%d;%d] removed from cache",cx,cy);
	Thread.LeaveLock(lockID);
}

rmp_cityscape* Map_Manager::cellCityscape(int cx, int cy) {
	LockID lockID = Thread.EnterLock(MUTEX_MAP);
		int idx = cellCacheIndex(cx,cy);
		if (idx == BAD_ID) idx = cellPrecache(cx,cy);
	
		if (idx == BAD_ID) {
			logWrite("Can't retrieve cityscape from cache - there is some serious error. Look up!");
			Thread.LeaveLock(lockID);
			return 0;
		}
		if (Game.IsDedicatedServer) {
			//Dedicated servers use own server timer clock (which is always correct)
			(*cacheCityscape[idx])->lastAccessTime = ServerTimer.Time();
		} else {
			//Local servers and clients use client timer - which is always running here
			(*cacheCityscape[idx])->lastAccessTime = Timer.Time();
		}
	Thread.LeaveLock(lockID);
	return *cacheCityscape[idx];
}

void Map_Manager::Frame() {
	LockID lockID = Thread.EnterLock(MUTEX_MAP);
		for (uint i = 0; i < cacheCityscape.Count; i++) {
			if (Timer.Time() - (*cacheCityscape[i])->lastAccessTime > 16.0f) { //FIXME: a proper timeout here
				cellUncache((*cacheCityscape[i])->CX,(*cacheCityscape[i])->CY);
			}
		}
	Thread.LeaveLock(lockID);
}

void Map_Manager::blockShouldExist(int bx, int by) {
	Map.cellPrecache((int)floor(bx / 64.0f),(int)floor(by / 64.0f));
}

void Map_Manager::planeShouldExist(int bx1, int by1, int bx2, int by2) {
	//FIXME: it checks all blocks, should check only 64x64 areas
	for (int cx = (int)floor(bx1 / 64.0f); cx <= (int)floor(bx2 / 64.0f); cx++)
		for (int cy = (int)floor(by1 / 64.0f); cy <= (int)floor(by2 / 64.0f); cy++)
			Map.cellPrecache(cx,cy);
}

void Map_Manager::Initialize() {
	logWrite("Initializing map manager (scanning for maps)");

	DIR *directory;
	struct dirent *direntry;

	int MapSize = 0; //Total size of maps loaded (for stats)

	mapsInfo.Preallocate(1024);
	cacheCityscape.Preallocate(1024);

	directory = opendir("./data");
	while ((directory) && ((direntry = readdir(directory)) != 0)) {
		int len = strlen(direntry->d_name);
		switch (len) {
			case 1:
				if (direntry->d_name[0] == '.') continue;
			case 2:
				if (direntry->d_name[0] == '.' && direntry->d_name[1] == '.') continue;
			default:
				break;
		}

		Chunk_Loader RMP;

		//Read the map
		char Filename[256];
		snprintf(Filename,256,"./data/%s",direntry->d_name);

		RMP.Open(Filename);
		if (RMP.IsFileFormat("ZRMP",RMP_VERSION) == false) {
			continue;
		}

		//Read all chunks from file
		while (RMP.ReadChunk()) {
			if (RMP.IsChunk("RMAP")) { //Scan the chunk for district name, and cell coordinates
				int CX,CY;
				char districtName[5] = "    ";
				RMP.Read(&CX,4);
				RMP.Read(&CY,4);
				RMP.Read(districtName,4);
				
				//Check if the map entry was already there
				int idx = cellEntryIndex(CX,CY);
				if (idx == BAD_ID) {
					logWritem("Found cell [%d;%d belongs to %s]: %s",CX,CY,districtName,direntry->d_name);

					rmp_entry* entry = mapsInfo.Add();
					entry->CX = CX;
					entry->CY = CY;
					strncpy(entry->Filename,Filename,256);
				} else {
					logWritem("Duplicate map cell! No place for [%d;%d] from %s",CX,CY,direntry->d_name);
				}
			}
		}
		MapSize += RMP.fileSize;
		RMP.Close();
	}
	if (directory) closedir(directory);
	logWrite("Found %d KB worth of maps",MapSize/1024);
	
	//Shrink entry table
	mapsInfo.Shrink();
}

void Map_Manager::Deinitialize() {
	logWrite("Deinitializing maps");
	for (uint i = 0; i < cacheCityscape.Count; i++) {
		releaseMap(*cacheCityscape[i]);
	}
	mapsInfo.Release();
	cacheCityscape.Release();
}

rmp_cityscape* Map_Manager::LoadMap(MapInfoID idx) {
	Chunk_Loader RMP;

	//Prepare empty map in case loading fails
	rmp_cityscape* cityscape = LoadEmpty();

	//Load the map file
	char Filename[256];
	snprintf(Filename,256,"%s",mapsInfo[idx]->Filename);

	RMP.Open(Filename);
	if (RMP.IsFileFormat("ZRMP",RMP_VERSION) == false) {
		logWrite("Error loading map %s (earlier it was loading fine...)",mapsInfo[idx]->Filename);
		return cityscape;
	}

	//Read all chunks from file
	while (RMP.ReadChunk()) {
		if (RMP.IsChunk("RMAP")) { //GTA2-exported cityscape (no interior)
			int CX,CY;
			RMP.Read(&CX,4);
			RMP.Read(&CY,4);
			if ((CX == mapsInfo[idx]->CX) && (CY == mapsInfo[idx]->CY)) {
				char districtName[5] = "    ";

				//Read district name
				RMP.Read(districtName,4);
				strncpy(cityscape->District,districtName,4);

				//Load tileset names
				for (int i = 0; i < 4; i++) {
					char graphicsName[256];
					RMP.ReadString(graphicsName);

					cityscape->CX = CX;
					cityscape->CY = CY;
					cityscape->GraphicsName[i] = mem.alloc_string(strlen(graphicsName)+1);
					strcpy(cityscape->GraphicsName[i],graphicsName);
				}

				//logWritem("RMP: Reading cityscape");
				char maxHeight;
				RMP.Read(&maxHeight,1);
				cityscape->maxHeight = maxHeight;
				cityscape->rmp_city_scape = (rmp_block_info*)mem.alloc(sizeof(rmp_block_info)*cityscape->maxHeight*64*64);
				cityscape->rmp_columns = (rmp_column_info*)mem.alloc(sizeof(rmp_column_info)*64*64);

				RMP.Read(cityscape->rmp_city_scape,cityscape->maxHeight*64*64*sizeof(rmp_block_info));
				RMP.Read(cityscape->rmp_columns,64*64*sizeof(rmp_column_info));
			}
		}
	}
	RMP.Close();

	return cityscape;
}

rmp_cityscape* Map_Manager::LoadEmpty() {
	rmp_cityscape* cityscape = (rmp_cityscape*)mem.alloc(sizeof(rmp_cityscape));
	//FIXME: error handling, right now this causes massive crash
	if (!cityscape) {
		logWrite("Almost could not empty map - not enough RAM to fit dummy data. Try lowering cache size!");
	} else {
		memset(cityscape,0,sizeof(rmp_cityscape));
		strncpy(cityscape->District,"NONE",4);
	}
	return cityscape;
}



//=============================================================================

rmp_block_info* Map_Manager::GetBlock(int X, int Y, int Z) {
	LockID lockID = Thread.EnterLock(MUTEX_MAP);

	//Find cityscape this block belongs to
	rmp_cityscape* cityscape = cellCityscape(X / 64, Y / 64);
	X = X % 64;
	Y = Y % 64;

	if ((Z >= 0) && (Z < cityscape->maxHeight) && (cityscape->rmp_city_scape)) {
		Thread.LeaveLock(lockID);
		return &cityscape->rmp_city_scape[Z*64*64 + Y*64 + X];
	} else {
		Thread.LeaveLock(lockID);
		return NULL;
	}
}

rmp_block_info* Map_Manager::GetBlock(Vector3f pos) {
	return GetBlock((int)pos.x, (int)pos.y, (int)pos.z+1);
}


//=============================================================================
rmp_cityscape* Map_Manager::GetCityscape(int X, int Y) {
	return cellCityscape(X / 64, Y / 64);
}

rmp_cityscape* Map_Manager::GetCityscape(Vector2f pos) {
	return GetCityscape((int)pos.x, (int)pos.y);
}

rmp_cityscape* Map_Manager::GetCityscape(Vector3f pos) {
	return GetCityscape((int)pos.x, (int)pos.y);
}


//=============================================================================
int Map_Manager::GetZ(int X, int Y, bool ignoreFlat) {
	LockID lockID = Thread.EnterLock(MUTEX_MAP);
		rmp_cityscape* cityscape = cellCityscape(X / 64, Y / 64);
		int Z = cityscape->maxHeight-1;
		X = X - 64*(X / 64);
		Y = Y - 64*(Y / 64);
		if (ignoreFlat) {
			while ((Z >= 0) &&
				   (cityscape->rmp_city_scape) && 
				  ((cityscape->rmp_city_scape[Z*64*64 + Y*64 + X].tex_lid == 0) ||
			       (cityscape->rmp_city_scape[Z*64*64 + Y*64 + X].param_lid & 0x8)))
				Z--;
		} else {
			while ((Z >= 0) && 
				   (cityscape->rmp_city_scape) && 
			       (cityscape->rmp_city_scape[Z*64*64 + Y*64 + X].tex_lid == 0))
				Z--;
		}
	Thread.LeaveLock(lockID);
	return Z;
}

float Map_Manager::GetZ(Vector2f pos, bool ignoreFlat) {
	return (float)GetZ((int)pos.x,(int)pos.y,ignoreFlat);
}

float Map_Manager::GetZ(Vector3f pos, bool ignoreFlat) {
	//FIXME: support for slopes
	return GetZ(Vector2f(pos.x,pos.y),ignoreFlat);
}


//=============================================================================

//Same side of line test
/*bool testSOL(Vector2f p1, Vector2f p2, Vector2f a, Vector2f b) {
	float cp1 = (b.x-a.x)*(p1.y-a.y) - (b.y-a.y)*(p1.x-a.x);
	float cp2 = (b.x-a.x)*(p2.y-a.y) - (b.y-a.y)*(p2.x-a.x);
	if (cp1*cp2 > 0) return true;
}
    
bool testTri(Vector2f p, Vector2f a, Vector2f b, Vector2f c) {
	if (testSOL(p,a,b,c) && testSOL(p,b,a,c) && testSOL(p,c,a,b)) return true;
	return false;
}*/



//FIXME: this function lacks following:
//1. check for degenerate faces (it will perform something bad if you pass triangle face)
//2. it only needs to check ONE triangle for quad face. FIX THIS LAZY BASTARD ASS
bool checkFace(Vector3f pos, int face, int slope_type) {
	if (Map_Geometry.Blocks[slope_type]->Face[face].Reversed == true) {
		return Collision.PointVsPlane(pos,Map_Geometry.Blocks[slope_type]->Face[face].Coord[0],
										  Map_Geometry.Blocks[slope_type]->Face[face].Coord[3],
										  Map_Geometry.Blocks[slope_type]->Face[face].Coord[1]) &&
			   Collision.PointVsPlane(pos,Map_Geometry.Blocks[slope_type]->Face[face].Coord[1],
										  Map_Geometry.Blocks[slope_type]->Face[face].Coord[3],
										  Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]);
	} else {
		return Collision.PointVsPlane(pos,Map_Geometry.Blocks[slope_type]->Face[face].Coord[0],
										  Map_Geometry.Blocks[slope_type]->Face[face].Coord[2],
										  Map_Geometry.Blocks[slope_type]->Face[face].Coord[1]) &&
			   Collision.PointVsPlane(pos,Map_Geometry.Blocks[slope_type]->Face[face].Coord[0],
										  Map_Geometry.Blocks[slope_type]->Face[face].Coord[3],
										  Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]);
	}
}

bool Map_Manager::isSolid(Vector3f pos) {
	rmp_block_info* block = GetBlock(pos);
	if (!block) return false;

	int block_type = (block->block_type & 0xF);
	int slope_type = (block->block_type >> 4) & 0x7F;

	//No collision with air
	if (block_type == 0) return false;

	//slope_type 0 doesnt need checks
	/*if (slope_type == 0) {
		return true;
	}*/

	Vector3f lpos = pos - Vector3f(floor(pos.x),floor(pos.y),floor(pos.z));

	bool faceAway = true;
	if (block->tex_left > 0)	faceAway = faceAway && checkFace(lpos,0,slope_type);
	if (block->tex_right > 0)	faceAway = faceAway && checkFace(lpos,1,slope_type);
	if (block->tex_top > 0)		faceAway = faceAway && checkFace(lpos,2,slope_type);
	if (block->tex_bottom > 0)	faceAway = faceAway && checkFace(lpos,3,slope_type);
	if (block->tex_lid > 0)		faceAway = faceAway && checkFace(lpos,4,slope_type);

	return faceAway;
}

void Map_Manager::traceListStruct::Reset() {
	BlocksCount = 0;
	CheckedCount = 0;
}

bool Map_Manager::traceListStruct::AddBlock(Vector3f pos, rmp_block_info* block) {
	if ((block) && ((BlocksCount == 0) || (Blocks[BlocksCount-1] != block))) {
		if (BlocksCount >= MAX_TRACE_BLOCKS) {
			logWrite("Too many blocks in trace list!");
			return false;
		}

		Blocks[BlocksCount] = block;
		BlockOffset[BlocksCount] = Vector3f(floor(pos.x),floor(pos.y),floor(pos.z));
		BlocksCount++;
	}
	return true;
}	

bool Map_Manager::traceSegment(Vector3f start, Vector3f end, Vector3f* collisionPoint) {	
	//If start is the end, then most likely nothing is hit
	if (start == end) return false;

	//Add all blocks that will be needed to trace
	Vector3f current = start;
	Vector3f dir = ((end - start).Normalize()) * 0.5f;

	traceList.Reset();
	while ((traceList.BlocksCount < 4) && (traceList.CheckedCount < 16)) {//Vector3f::Dot(dir,end-current) > 0.0f) {
		rmp_block_info* block = Map.GetBlock(current);
		
		//Add current block our trace ray is in
		if (!traceList.AddBlock(current,block)) { 
			*collisionPoint = start; 
			return true; 
		}

		//Check edges - if we are on any block edge (256.0f), then check both (256.0f;###;###) and (255.0f;###;###) blocks
		if (floor(current.x) == current.x) { //X axis
			if (!traceList.AddBlock(current-Vector3f(1.0f,0.0f,0.0f),
			           Map.GetBlock(current-Vector3f(1.0f,0.0f,0.0f)))) { *collisionPoint = start; return true; }
		}
		if (floor(current.y) == current.y) { //Y axis
			if (!traceList.AddBlock(current-Vector3f(0.0f,1.0f,0.0f),
			           Map.GetBlock(current-Vector3f(0.0f,1.0f,0.0f)))) { *collisionPoint = start; return true; }
		}
		if (floor(current.z) == current.z) { //Z axis
			if (!traceList.AddBlock(current-Vector3f(0.0f,0.0f,1.0f),
			           Map.GetBlock(current-Vector3f(0.0f,0.0f,1.0f)))) { *collisionPoint = start; return true; }
		}

		current = current + dir;
		traceList.CheckedCount++;
	}

	//If no blocks were found, then no collision must be perfomed
	if (traceList.BlocksCount == 0) return false;

	//Trace segment against every face in blocks that lie ahead
	for (int i = 0; i < traceList.BlocksCount; i++) {
		int slope_type = (traceList.Blocks[i]->block_type >> 4) & 0x7F;

		int check_face[6];
		check_face[0] = (traceList.Blocks[i]->param_left & (1 << 4));
		check_face[1] = (traceList.Blocks[i]->param_right & (1 << 4));
		check_face[2] = (traceList.Blocks[i]->param_top & (1 << 4));
		check_face[3] = (traceList.Blocks[i]->param_bottom & (1 << 4));
		check_face[4] = (traceList.Blocks[i]->block_type & 0xF);//(Blocks[i]->param_lid & (1 << 4));

		for (int face = 0; face < 5; face++) {
			if (check_face[face]) {
				bool intersectHappened;

				if (Map_Geometry.Blocks[slope_type]->Face[face].Reversed == true) {
					intersectHappened = Collision.SegmentVsTriangle(
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[0]+traceList.BlockOffset[i],
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]+traceList.BlockOffset[i],
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[1]+traceList.BlockOffset[i],
						start, end, collisionPoint)
									|| Collision.SegmentVsTriangle(
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[1]+traceList.BlockOffset[i],
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]+traceList.BlockOffset[i],
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[3]+traceList.BlockOffset[i],
						start, end, collisionPoint);
				} else {
					intersectHappened = Collision.SegmentVsTriangle(
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[0]+traceList.BlockOffset[i],
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[1]+traceList.BlockOffset[i],
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]+traceList.BlockOffset[i],
						start, end, collisionPoint)
									|| Collision.SegmentVsTriangle(
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[0]+traceList.BlockOffset[i],
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]+traceList.BlockOffset[i],
						Map_Geometry.Blocks[slope_type]->Face[face].Coord[3]+traceList.BlockOffset[i],
						start, end, collisionPoint);
				}

				if (intersectHappened) {
					return true;
				}
			}
		}
	}

	return false;
}
