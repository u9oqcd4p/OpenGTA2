/*
** OpenGBH Top-Down Game Engine
** Everything is coded entirely by Black Phoenix
** Does not use any DMA Design/Rockstar code or specifications/info
**   (see toolset license information for that)
** 
** You are free to do whatever you want with the code, it's nice if you credit 
** me somehow if you reuse something.
**
** The code is partially licensed under shortened beerware, which means if you
** reuse it, and we meet, you owe me beer (I don't drink beer, but I have
** friends who do)
**
*/

//Define this to build dedicated server instead of game
//FIXME: it compiles extra client CPP files in MSVS, fix that
//#define DEDICATED_SERVER

#define GL_GLEXT_PROTOTYPES

//FIXME: load these from config file
//Most of these aren't even required anymore.. :(
#define MAX_TEXTURE_FILE_ENTRIES 1024
#define MAX_FONT_ENTRIES 1024
#define DRAW_VBO_SIZE 65536
#define STRING_POOL_SIZE 4096
#define MAX_STRINGPOOLS 512
#define MAX_CONVARS 1024
#define MAX_CONVARLENGTH 256
#define MAX_LOG_SIZE 32768

#define RMP_VERSION 160
#define TEX_VERSION 130

#define ENGINE_VERSION "OpenGBH II"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
 
#define BAD_ID 0xFFFFFFFF
typedef unsigned int uint;

#ifdef _WIN32
	#define snprintf _snprintf
	#define snscanf _snscanf
#endif

//GLFW and standard libs
#include <GL/glfw.h>
#include <math.h>
#include <stdlib.h>

//Low-level support
#include "utf8.h"
#include "malloc.h"
#include "darray.h"
#include "log.h"
#include "timer.h"
#include "chunkload.h"
#include "thread.h"

//Math and utils
#include "matrix.h"
#include "vector.h"
#include "random.h"
#include "convar.h"

//Map geometry and stuff
#include "format_rmp.h"
#include "map.h"
#include "map_geom.h"
#include "collision.h"
#include "game.h"

#include "input_keys.h"