#include "opengta_cl.h"

Map_Vertex_Geometry_Manager Map_Vertex_Geometry;

/********
 *      *
 *		*
 *		*
 ********/

const int texcoord_rotation[8][8] = {
	//Normal face rotation
	{0,1,2,1,2,3,0,3},//0
	{2,1,0,1,0,3,2,3},//90
	{0,3,0,1,2,1,2,3},//180
	{0,1,0,3,2,3,2,1},//270
	//Flipped face rotation
	{2,3,0,3,0,1,2,1},//0
	{0,3,2,3,2,1,0,1},//90
	{2,1,2,3,0,3,0,1},//180
	{2,3,2,1,0,1,0,3},//270
};

const Vector3f flatface_offsets[4] = {
	Vector3f( 1.0f, 0.0f, 0.0f),
	Vector3f(-1.0f, 0.0f,0.0f),
	Vector3f( 0.0f, 1.0f ,0.0f),
	Vector3f( 0.0f,-1.0f ,0.0f)};

void Map_Vertex_Geometry_Manager::blockFace(Vertex_Buffer* vBuffer, int slope_type, int face, int texID, int texRot, int flat, Vector3f offset) {
	texture_entry* tex = Graphics.GetTextureEntryByID(texID);

	float TexCoordTex[4];
	TexCoordTex[0] = tex->U1;
	TexCoordTex[1] = tex->V1;
	TexCoordTex[2] = tex->U2;
	TexCoordTex[3] = tex->V2;
	float TexWidth  = (tex->U2 - tex->U1);
	float TexHeight = (tex->V2 - tex->V1);
	
	//Apply block UV tranform
	//TexCoordTex[0] = tex->U1;
	//TexCoordTex[1] = tex->V1;
	//TexCoordTex[2] = tex->U1+TexWidth;
	//TexCoordTex[3] = tex->V1+TexHeight;

	float TexCoord[4][2];
	TexCoord[0][0] = TexCoordTex[texcoord_rotation[texRot][0]];//+TexWidth		* Blocks[slope_type]->Face[face].UV[0].x;
	TexCoord[0][1] = TexCoordTex[texcoord_rotation[texRot][1]];//+TexHeight	* Blocks[slope_type]->Face[face].UV[0].y;
	TexCoord[1][0] = TexCoordTex[texcoord_rotation[texRot][2]];//+TexWidth		* Blocks[slope_type]->Face[face].UV[1].x;
	TexCoord[1][1] = TexCoordTex[texcoord_rotation[texRot][3]];//+TexHeight	* Blocks[slope_type]->Face[face].UV[1].y;
	TexCoord[2][0] = TexCoordTex[texcoord_rotation[texRot][4]];//+TexWidth		* Blocks[slope_type]->Face[face].UV[2].x;
	TexCoord[2][1] = TexCoordTex[texcoord_rotation[texRot][5]];//+TexHeight	* Blocks[slope_type]->Face[face].UV[2].y;
	TexCoord[3][0] = TexCoordTex[texcoord_rotation[texRot][6]];//+TexWidth		* Blocks[slope_type]->Face[face].UV[3].x;
	TexCoord[3][1] = TexCoordTex[texcoord_rotation[texRot][7]];//+TexHeight	* Blocks[slope_type]->Face[face].UV[3].y;

	if ((flat) && (slope_type == 0)) {
		assert(face < 4);//FIXME
		offset += flatface_offsets[face];
	}

	if (Map_Geometry.Blocks[slope_type]->Face[face].Triangle == false) {
		if (Map_Geometry.Blocks[slope_type]->Face[face].Reversed == true) {
			//0 1
			//3 2
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[0]+offset,Vector2f(TexCoord[0][0],TexCoord[0][1]));
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[3]+offset,Vector2f(TexCoord[3][0],TexCoord[3][1]));
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[1]+offset,Vector2f(TexCoord[1][0],TexCoord[1][1]));
	
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[1]+offset,Vector2f(TexCoord[1][0],TexCoord[1][1]));
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[3]+offset,Vector2f(TexCoord[3][0],TexCoord[3][1]));
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]+offset,Vector2f(TexCoord[2][0],TexCoord[2][1]));
		} else {
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[0]+offset,Vector2f(TexCoord[0][0],TexCoord[0][1]));
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]+offset,Vector2f(TexCoord[2][0],TexCoord[2][1]));
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[1]+offset,Vector2f(TexCoord[1][0],TexCoord[1][1]));
			
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[0]+offset,Vector2f(TexCoord[0][0],TexCoord[0][1]));
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[3]+offset,Vector2f(TexCoord[3][0],TexCoord[3][1]));
			vBuffer->Add(Map_Geometry.Blocks[slope_type]->Face[face].Coord[2]+offset,Vector2f(TexCoord[2][0],TexCoord[2][1]));
		}
	} else {
		//logWrite("AURGH");
		//vBuffer->Add(Blocks[slope_type]->Face[face].Coord[0]+offset,Vector2f(TexCoord[0][0],TexCoord[0][1]));
		//vBuffer->Add(Blocks[slope_type]->Face[face].Coord[1]+offset,Vector2f(TexCoord[1][0],TexCoord[1][1]));
		//vBuffer->Add(Blocks[slope_type]->Face[face].Coord[2]+offset,Vector2f(TexCoord[2][0],TexCoord[2][1]));
	}
}

void Map_Vertex_Geometry_Manager::blockGeometry(Vertex_Buffer* vBuffer, rmp_block_info* block, int texBase, Vector3f offset) {
	int slope_type = (block->block_type >> 4) & 0x7F;

	int tex_left =		texBase+block->tex_left;
	int tex_right =		texBase+block->tex_right;
	int tex_top =		texBase+block->tex_top;
	int tex_bottom =	texBase+block->tex_bottom;
	int tex_lid =		texBase+block->tex_lid;

	int face_left = 0;

	//logWrite("%d",slope_type);
	//assert((block->param_lid & 0x7) == 0);

	//FIXME: array lookup instead of this mess
	if ((slope_type >= 64) && (slope_type < 68)) {
		if (slope_type == 64) blockFace(vBuffer, slope_type, 4, tex_left,  block->param_lid & 0x7, 0, offset);
		if (slope_type == 65) blockFace(vBuffer, slope_type, 4, tex_right, block->param_lid & 0x7, 0, offset);
		if (slope_type == 66) blockFace(vBuffer, slope_type, 4, tex_left, block->param_lid & 0x7, 0, offset);
		if (slope_type == 67) blockFace(vBuffer, slope_type, 4, tex_right, block->param_lid & 0x7, 0, offset);
	} else {
		if (block->tex_left > 0)	blockFace(vBuffer, slope_type, 0, tex_left,  block->param_left & 0x7,	block->param_right & 0x8,	offset);
		if (block->tex_right > 0)	blockFace(vBuffer, slope_type, 1, tex_right, block->param_right & 0x7,	block->param_left & 0x8,	offset);
		if (block->tex_top > 0)		blockFace(vBuffer, slope_type, 2, tex_top,   block->param_top & 0x7,	block->param_bottom & 0x8,	offset);
		if (block->tex_bottom > 0)	blockFace(vBuffer, slope_type, 3, tex_bottom,block->param_bottom & 0x7,	block->param_top & 0x8,		offset);
		if (block->tex_lid > 0)		blockFace(vBuffer, slope_type, 4, tex_lid,   block->param_lid & 0x7,	0,							offset);
	}
}

void Map_Vertex_Geometry_Manager::planeGeometry(Vertex_Buffer* vBuffer, int texBase, int bx1, int by1, int bx2, int by2, int bz) {
	int tx = (int)floor(bx1 / 64.0f);
	int ty = (int)floor(by1 / 64.0f);
	rmp_cityscape* cityscape = Map.cellCityscape(tx,ty); //cityscape to gather data from
	
	for (int x = bx1; x <= bx2; x++)
		for (int y = by1; y <= by2; y++) {
			int cx = (int)floor(x / 64.0f); //current cityscape
			int cy = (int)floor(y / 64.0f);
			if ((tx != cx)  || (ty != cy)) {
				tx = cx; ty = cy; //precache new one
				cityscape = Map.cellCityscape(tx,ty);
			}

			int kx = abs(x % 64);//x-abs(cx*64);
			int ky = abs(y % 64);//y-abs(cy*64);

			if ((cityscape->rmp_city_scape) && (bz < cityscape->maxHeight)) {
				rmp_block_info* block = &cityscape->rmp_city_scape[bz*64*64+kx*64+ky];//[kx][ky];
				blockGeometry(vBuffer, block, texBase, Vector3f(x*1.0f,y*1.0f,bz*1.0f-1.0f));
			}
		}
}