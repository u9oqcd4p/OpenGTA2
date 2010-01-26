#ifndef MAP_GEOM_CL
#define MAP_GEOM_CL

struct Map_Vertex_Geometry_Manager {
	//Generate face geometry for slopetype (face = geomBlock face)
	void blockFace(Vertex_Buffer* vBuffer, int slope_type, int face, int texID, int texRot, int flat, Vector3f offset);
	//Generate block geometry for block[X,Y,Z] and append it to buffer (it must be of enough size!)
	void blockGeometry(Vertex_Buffer* vBuffer, rmp_block_info* block, int texBase, Vector3f offset);
	//Generate plane geometry for height[Z] and append it to buffer (range [X1,Y1]..[X2,Y2])
	void planeGeometry(Vertex_Buffer* vBuffer, int texBase, int bx1, int by1, int bx2, int by2, int bz);
};

extern Map_Vertex_Geometry_Manager Map_Vertex_Geometry;

#endif