#ifndef FORMAT_RMP_H
#define FORMAT_RMP_H

struct rmp_block_info { //24 bytes
	//65536 textures indexed (1024 really used)
	unsigned short tex_left; //10 bytes
	unsigned short tex_right;
	unsigned short tex_top;
	unsigned short tex_bottom;
	unsigned short tex_lid;

	//params:
	//bits 0 - mirroring
	//bits 1..2 - face rotation (0* 90* 180* 270*)
	//bits 3 - flat face (other face should not be flat)
	//bits 4..3 - face collision type:
	//  0: no collision
	//  1: wall [nothing]
	//  2: fence [lets bullets through]
	//  3: building window
	//  4: building door
	//  5: garage door [building door for cars]
	//  6:
	//  7:
	//bits 4..7 - reserved
	unsigned char param_left; //5 bytes
	unsigned char param_right;
	unsigned char param_top;
	unsigned char param_bottom;
	unsigned char param_lid;

							//4 bytes
	unsigned short arrows;	//left - right - up - down in bits 0..3
							//bits 0..3: normal traffic arrows
							//bits 4..7: shortcut arrows (for emergency vehicles and faggot drivers)

	//bits 0..3 - ground type
	//  0 - air
	//  1 - road (cars spawn on it)
	//  2 - pavement (peds spawn on it)
	//  3 - generic/solid
	//  4 - park
	//  5-15 - ?
	//
	//bits 4-10: slope type:
	//  0 = none
	//  1- 2 = up 26 low, high
	//  3 - 4 = down 26 low, high
	//  5 - 6 = left 26 low, high
	//  7 - 8 = right 26 low, high
	//  9 - 16 = up 7 low – high
	//  17 - 24 = down 7 low – high
	//  25 - 32 = left 7 low – high
	//  33 - 40 = right 7 low – high
	//  41 - 44 = 45 up,down,left,right
	//  45 = diagonal, facing up left
	//  46 = diagonal, facing up right
	//  47 = diagonal, facing down left
	//  48 = diagonal, facing down right
	//  49 = 3-sided diagonal slope, facing up left
	//  50 = 3-sided diagonal slope, facing up right
	//  51 = 3-sided diagonal slope, facing down left
	//  52 = 3-sided diagonal slope, facing down right
	//  53 = partial block left
	//  54 = partial block right
	//  55 = partial block top
	//  56 = partial block bottom
	//  57 = partial block top left corner
	//  58 = partial block top right corner
	//  59 = partial block bottom right corner
	//  60 = partial block bottom left corner
	//  61 = partial centre block 16x16
	//  62 = <unused>
	//  63 = <indicates slope in block above, doesnt mean anything in OpenGTA2>
	//  64 = 4-sided diagonal slope, facing up left
	//  65 = 4-sided diagonal slope, facing up right
	//  66 = 4-sided diagonal slope, facing down left
	//  67 = 4-sided diagonal slope, facing down right
	//bits 11..15 - unused
	unsigned short block_type;

	//unsigned  int padding0; //5 bytes reserved padding:
	unsigned char padding0;
	unsigned char padding1;
	unsigned char padding2;
	unsigned char padding3;
};

struct rmp_column_info { //8 bytes
	unsigned int owner;  //Owner ID
	float height_offset; //Height offset for the column, in blocks
};

struct rmp_cityscape {
	int CX; int CY;				//Cell coordinates (for fast lookup)
	char District[4];			//District code
	char* GraphicsName[4];//[64];	//Graphics set name

	float lastAccessTime;		//Last time accessed, for caching system
	int maxHeight;				//7 for GTA2 levels

	//rmp_block_info  rmp_city_scape		[16][64][64];
	//rmp_block_info  rmp_interior_scape	[16][64][64];
	//rmp_column_info rmp_columns				[64][64];
	rmp_block_info*   rmp_city_scape;
	rmp_block_info*   rmp_interior_scape;
	rmp_column_info*  rmp_columns;
};

#endif