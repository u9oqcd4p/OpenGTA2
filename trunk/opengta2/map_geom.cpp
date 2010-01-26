#include "opengta_cl.h"	

//Vector3f Block[3] = {Vector3f(0,0,0),Vector3f(1,1,1),Vector3f(2,2,2)};

Map_Geometry_Manager Map_Geometry;

void Map_Geometry_Manager::genRotate90(geomBlock* dstblock, geomBlock* srcblock) {
	for (int f=4;f>=0;f--) {
		for (int c=0;c<4;c++) {
			dstblock->Face[f].Coord[c] = srcblock->Face[f].Coord[c];
		//	dstblock->Face[f].Coord[c].Rotate_Z(90.0f,Vector3f(1.0f,1.0f,0.0f));
		//	dstblock->Face[f].Coord[c] -= Vector3f(1.0f,0.0f,0.0f);
		}

		if (f == 4) {
			dstblock->Face[f].Coord[0].z = srcblock->Face[f].Coord[3].z;
			dstblock->Face[f].Coord[1].z = srcblock->Face[f].Coord[0].z;
			dstblock->Face[f].Coord[2].z = srcblock->Face[f].Coord[1].z;
			dstblock->Face[f].Coord[3].z = srcblock->Face[f].Coord[2].z;
		} else {
			/*if (srcblock->Face[f].Coord[0].z > 0.0f) dstblock->Face[f].Coord[0].z = srcblock->Face[4].Coord[0].z;
			if (srcblock->Face[f].Coord[1].z > 0.0f) dstblock->Face[f].Coord[1].z = srcblock->Face[4].Coord[1].z;
			if (srcblock->Face[f].Coord[2].z > 0.0f) dstblock->Face[f].Coord[2].z = srcblock->Face[4].Coord[2].z;
			if (srcblock->Face[f].Coord[3].z > 0.0f) dstblock->Face[f].Coord[3].z = srcblock->Face[4].Coord[3].z;*/
		}
		

		//Check for triangleface (only first three coords are used, so slide the remaining ones to 0..2 range)
		dstblock->Face[f].Triangle = false;
		if (dstblock->Face[f].Coord[0] == dstblock->Face[f].Coord[1]) {
			dstblock->Face[f].Triangle = true;
			dstblock->Face[f].Coord[1] = dstblock->Face[f].Coord[2];
			dstblock->Face[f].Coord[2] = dstblock->Face[f].Coord[3];
		}
		if (dstblock->Face[f].Coord[1] == dstblock->Face[f].Coord[2]) {
			dstblock->Face[f].Triangle = true;
			dstblock->Face[f].Coord[2] = dstblock->Face[f].Coord[3];
		}
		if (dstblock->Face[f].Coord[2] == dstblock->Face[f].Coord[3]) {
			dstblock->Face[f].Triangle = true;
		}
		if (dstblock->Face[f].Coord[3] == dstblock->Face[f].Coord[1]) {
			dstblock->Face[f].Triangle = true;
		}
	}
}

void Map_Geometry_Manager::genBlock(geomBlock* dstblock, Vector3f LT, Vector3f RT, Vector3f LB, Vector3f RB, bool revLidTriangles) {
	for (int i=0;i<5;i++) dstblock->Face[i].Triangle = false;
	//LEFT FACE
	dstblock->Face[0].Coord[0] = Vector3f(LT.x,LT.y,LT.z);
	dstblock->Face[0].Coord[1] = Vector3f(LB.x,LB.y,LB.z);
	dstblock->Face[0].Coord[2] = Vector3f(LB.x,LB.y,0.0f);
	dstblock->Face[0].Coord[3] = Vector3f(LT.x,LT.y,0.0f);
	dstblock->Face[0].UV[0]    = Vector2f(LT.x,LT.z);
	dstblock->Face[0].UV[1]    = Vector2f(LB.x,LT.z);
	dstblock->Face[0].UV[2]    = Vector2f(LB.x,0.0f);
	dstblock->Face[0].UV[3]    = Vector2f(LT.x,0.0f);
	//RIGHT FACE
	dstblock->Face[1].Coord[0] = Vector3f(RB.x,RB.y,RB.z);
	dstblock->Face[1].Coord[1] = Vector3f(RT.x,RT.y,RT.z);
	dstblock->Face[1].Coord[2] = Vector3f(RT.x,RT.y,0.0f);
	dstblock->Face[1].Coord[3] = Vector3f(RB.x,RB.y,0.0f);
	dstblock->Face[1].UV[0]    = Vector2f(RB.x,RT.z);
	dstblock->Face[1].UV[1]    = Vector2f(RT.x,RT.z);
	dstblock->Face[1].UV[2]    = Vector2f(RT.x,0.0f);
	dstblock->Face[1].UV[3]    = Vector2f(RB.x,0.0f);
	//TOP FACE
	dstblock->Face[2].Coord[0] = Vector3f(RT.x,RT.y,RT.z);
	dstblock->Face[2].Coord[1] = Vector3f(LT.x,LT.y,LT.z);
	dstblock->Face[2].Coord[2] = Vector3f(LT.x,LT.y,0.0f);
	dstblock->Face[2].Coord[3] = Vector3f(RT.x,RT.y,0.0f);
	dstblock->Face[2].UV[0]    = Vector2f(RT.y,RT.z);
	dstblock->Face[2].UV[1]    = Vector2f(LT.y,LT.z);
	dstblock->Face[2].UV[2]    = Vector2f(LT.y,0.0f);
	dstblock->Face[2].UV[3]    = Vector2f(RT.y,0.0f);
	//BOTTOM FACE
	dstblock->Face[3].Coord[0] = Vector3f(LB.x,LB.y,LB.z);
	dstblock->Face[3].Coord[1] = Vector3f(RB.x,RB.y,RB.z);
	dstblock->Face[3].Coord[2] = Vector3f(RB.x,RB.y,0.0f);
	dstblock->Face[3].Coord[3] = Vector3f(LB.x,LB.y,0.0f);
	dstblock->Face[3].UV[0]    = Vector2f(LB.y,LB.z);
	dstblock->Face[3].UV[1]    = Vector2f(RB.y,RB.z);
	dstblock->Face[3].UV[2]    = Vector2f(RB.y,0.0f);
	dstblock->Face[3].UV[3]    = Vector2f(LB.y,0.0f);
	//LID FACE
	dstblock->Face[4].Coord[0] = Vector3f(LT.x,LT.y,LT.z);
	dstblock->Face[4].Coord[1] = Vector3f(RT.x,RT.y,RT.z);
	dstblock->Face[4].Coord[2] = Vector3f(RB.x,RB.y,RB.z);
	dstblock->Face[4].Coord[3] = Vector3f(LB.x,LB.y,LB.z);
	dstblock->Face[4].UV[0]    = Vector2f(LT.x,LT.y);
	dstblock->Face[4].UV[1]    = Vector2f(RT.x,RT.y);
	dstblock->Face[4].UV[2]    = Vector2f(RB.x,RB.y);
	dstblock->Face[4].UV[3]    = Vector2f(LB.x,LB.y);
	//ANGRY FACE
	dstblock->Face[0].Reversed = false;
	dstblock->Face[1].Reversed = false;
	dstblock->Face[2].Reversed = false;
	dstblock->Face[3].Reversed = false;
	dstblock->Face[4].Reversed = revLidTriangles;
}

void Map_Geometry_Manager::Deinitialize() {
	Blocks.Release();
}

void Map_Geometry_Manager::Initialize() {
	Blocks.Preallocate(128);

	//0 = none
	genBlock(Blocks[0],		Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));

	//1 - 2 = up 26 low, high
	genBlock(Blocks[1],		Vector3f(0.0f,0.0f,0.5f),		Vector3f(1.0f,0.0f,0.5f),
							Vector3f(0.0f,1.0f,0.0f),		Vector3f(1.0f,1.0f,0.0f));
	genBlock(Blocks[2],		Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,0.5f),		Vector3f(1.0f,1.0f,0.5f));
	//3 - 4 = down 26 low, high
	genBlock(Blocks[3],		Vector3f(0.0f,0.0f,0.0f),		Vector3f(1.0f,0.0f,0.0f),
							Vector3f(0.0f,1.0f,0.5f),		Vector3f(1.0f,1.0f,0.5f));
	genBlock(Blocks[4],		Vector3f(0.0f,0.0f,0.5f),		Vector3f(1.0f,0.0f,0.5f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	//5 - 6 = left 26 low, high
	genBlock(Blocks[5],		Vector3f(0.0f,0.0f,0.5f),		Vector3f(1.0f,0.0f,0.0f),
							Vector3f(0.0f,1.0f,0.5f),		Vector3f(1.0f,1.0f,0.0f));
	genBlock(Blocks[6],		Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,0.5f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,0.5f));
	//7 - 8 = right 26 low, high
	genBlock(Blocks[7],		Vector3f(0.0f,0.0f,0.0f),		Vector3f(1.0f,0.0f,0.5f),
							Vector3f(0.0f,1.0f,0.0f),		Vector3f(1.0f,1.0f,0.5f));
	genBlock(Blocks[8],		Vector3f(0.0f,0.0f,0.5f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,0.5f),		Vector3f(1.0f,1.0f,1.0f));

	//9 - 16 = up 7 low – high
	for (int div = 0; div < 8; div++)
		genBlock(Blocks[9+div],	Vector3f(0.0f,0.0f,0.125f*(div+1)),	Vector3f(1.0f,0.0f,0.125f*(div+1)),
								Vector3f(0.0f,1.0f,0.125f*(div+0)),	Vector3f(1.0f,1.0f,0.125f*(div+0)));
	//33 - 40 = right 7 low – high
	for (int div = 0; div < 8; div++)
		genBlock(Blocks[33+div],Vector3f(0.0f,0.0f,0.125f*(div+0)),	Vector3f(1.0f,0.0f,0.125f*(div+1)),
								Vector3f(0.0f,1.0f,0.125f*(div+0)),	Vector3f(1.0f,1.0f,0.125f*(div+1)));
	//17 - 24 = down 7 low – high
	for (int div = 0; div < 8; div++)
		genBlock(Blocks[17+div],Vector3f(0.0f,0.0f,0.125f*(div+0)),	Vector3f(1.0f,0.0f,0.125f*(div+0)),
								Vector3f(0.0f,1.0f,0.125f*(div+1)),	Vector3f(1.0f,1.0f,0.125f*(div+1)));
	//25 - 32 = left 7 low – high
	for (int div = 0; div < 8; div++)
		genBlock(Blocks[25+div],Vector3f(0.0f,0.0f,0.125f*(div+1)),	Vector3f(1.0f,0.0f,0.125f*(div+0)),
								Vector3f(0.0f,1.0f,0.125f*(div+1)),	Vector3f(1.0f,1.0f,0.125f*(div+0)));
	
	//41 - 44 = 45 up,down,left,right
	genBlock(Blocks[41],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,0.0f),		Vector3f(1.0f,1.0f,0.0f));
	genBlock(Blocks[42],	Vector3f(0.0f,0.0f,0.0f),		Vector3f(1.0f,0.0f,0.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	genBlock(Blocks[43],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,0.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,0.0f));
	genBlock(Blocks[44],	Vector3f(0.0f,0.0f,0.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,0.0f),		Vector3f(1.0f,1.0f,1.0f));

	//45 = diagonal, facing up left
	genBlock(Blocks[45],	Vector3f(1.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f),true);
	//46 = diagonal, facing up right
	genBlock(Blocks[46],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(0.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	//47 = diagonal, facing down left
	genBlock(Blocks[47],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(1.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	//48 = diagonal, facing down right
	genBlock(Blocks[48],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(0.0f,1.0f,1.0f),true);

	//FIXME: texture problem
	//49 = 3 or 4-sided diagonal slope, facing up left
	genBlock(Blocks[49],	Vector3f(0.0f,0.0f,0.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f),true);
	//50 = 3 or 4-sided diagonal slope, facing up right
	genBlock(Blocks[50],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,0.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	//51 = 3 or 4-sided diagonal slope, facing down left
	genBlock(Blocks[51],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,0.0f),		Vector3f(1.0f,1.0f,1.0f));
	//52 = 3 or 4-sided diagonal slope, facing down right
	genBlock(Blocks[52],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,0.0f),true);

	//53 = partial block left
	genBlock(Blocks[53],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(0.5f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(0.5f,1.0f,1.0f));
	//54 = partial block right
	genBlock(Blocks[54],	Vector3f(0.5f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.5f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	//55 = partial block top
	genBlock(Blocks[55],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,0.5f,1.0f),		Vector3f(1.0f,0.5f,1.0f));
	//56 = partial block bottom
	genBlock(Blocks[56],	Vector3f(0.0f,0.5f,1.0f),		Vector3f(1.0f,0.5f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));

	//57 = partial block top left corner
	genBlock(Blocks[57],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(0.5f,0.0f,1.0f),
							Vector3f(0.0f,0.5f,1.0f),		Vector3f(0.5f,0.5f,1.0f));
	//58 = partial block top right corner
	genBlock(Blocks[58],	Vector3f(0.5f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.5f,0.5f,1.0f),		Vector3f(1.0f,0.5f,1.0f));
	//59 = partial block bottom right corner
	genBlock(Blocks[59],	Vector3f(0.5f,0.5f,1.0f),		Vector3f(1.0f,0.5f,1.0f),
							Vector3f(0.5f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	//60 = partial block bottom left corner
	genBlock(Blocks[60],	Vector3f(0.0f,0.5f,1.0f),		Vector3f(0.5f,0.5f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(0.5f,1.0f,1.0f));

	//61 = partial centre block 16x16
	genBlock(Blocks[61],	Vector3f(0.25f,0.25f,1.0f),		Vector3f(0.75f,0.25f,1.0f),
							Vector3f(0.25f,0.75f,1.0f),		Vector3f(0.75f,0.75f,1.0f));

	//62 = <unused>
	//63 = <indicates slope in block above, same as 0 in OpenGTA2>
	//genBlock(Blocks[63],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
	//						Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));

	//Up left
	genBlock(Blocks[64],	Vector3f(1.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f),
							Vector3f(1.0f,0.0f,0.0f),		Vector3f(0.0f,1.0f,0.0f));

	//Up right
	genBlock(Blocks[65],	Vector3f(0.0f,1.0f,1.0f),		Vector3f(0.0f,1.0f,1.0f),
							Vector3f(1.0f,1.0f,0.0f),		Vector3f(0.0f,0.0f,0.0f));

	genBlock(Blocks[66],	Vector3f(1.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,0.0f,0.0f),		Vector3f(1.0f,1.0f,0.0f));

	genBlock(Blocks[67],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(0.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,0.0f),		Vector3f(1.0f,0.0f,0.0f));

	/*genBlock(Blocks[64],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));

	//Blocks[64]->Coords[
	genBlock(Blocks[65],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	genBlock(Blocks[66],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));
	genBlock(Blocks[67],	Vector3f(0.0f,0.0f,1.0f),		Vector3f(1.0f,0.0f,1.0f),
							Vector3f(0.0f,1.0f,1.0f),		Vector3f(1.0f,1.0f,1.0f));*/
}