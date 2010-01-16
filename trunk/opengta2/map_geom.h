#ifndef MAP_GEOM_H
#define MAP_GEOM_H

struct geomFace {
	bool Triangle;
	bool Reversed; //reversed triangle order when drawing quad
	Vector3f Coord[4];
	Vector2f UV[4];
};

struct geomBlock {
	//0 - Left
	//1 - Right
	//2 - Top
	//3 - Bottom
	//4 - Lid
	geomFace Face[5];
};

//Geometry generator for vertex buffer
struct Map_Geometry_Manager {
	//Initialize block geometry
	void Initialize();
	void Deinitialize();

	void genBlock(geomBlock* dstblock, Vector3f LT, Vector3f RT, Vector3f RB, Vector3f LB, bool revLidTriangles = false);
	void genRotate90(geomBlock* dstblock, geomBlock* srcblock);

	//Blocks geometry (pregenerated)
	DataArray<geomBlock> Blocks;
};

extern Map_Geometry_Manager Map_Geometry;

#endif