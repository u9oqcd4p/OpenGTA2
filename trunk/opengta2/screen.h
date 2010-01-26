#ifndef WINDOW_H
#define WINDOW_H

#ifdef _WIN32
	extern PFNGLGENBUFFERSARBPROC pglGenBuffersARB;                     // VBO Name Generation Procedure
	extern PFNGLBINDBUFFERARBPROC pglBindBufferARB;                     // VBO Bind Procedure
	extern PFNGLBUFFERDATAARBPROC pglBufferDataARB;                     // VBO Data Loading Procedure
	extern PFNGLBUFFERSUBDATAARBPROC pglBufferSubDataARB;               // VBO Sub Data Loading Procedure
	extern PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB;               // VBO Deletion Procedure
	extern PFNGLGETBUFFERPARAMETERIVARBPROC pglGetBufferParameterivARB; // return various parameters of VBO
	extern PFNGLMAPBUFFERARBPROC pglMapBufferARB;                       // map VBO procedure
	extern PFNGLUNMAPBUFFERARBPROC pglUnmapBufferARB;                   // unmap VBO procedure

	#define glGenBuffersARB           pglGenBuffersARB
	#define glBindBufferARB           pglBindBufferARB
	#define glBufferDataARB           pglBufferDataARB
	#define glBufferSubDataARB        pglBufferSubDataARB
	#define glDeleteBuffersARB        pglDeleteBuffersARB
	#define glGetBufferParameterivARB pglGetBufferParameterivARB
	#define glMapBufferARB            pglMapBufferARB
	#define glUnmapBufferARB          pglUnmapBufferARB
#endif

struct Screen_Manager {
	//(Re)initialize screen to this width/height
	bool Initialize(int width, int height, bool fullscreen);
	//Deinitialize screen (this closes everything too)
	void Deinitialize();

	//Set viewport
	void SetViewport(int left, int right, int width, int height);

	//Clear screen
	void Clear();

	//Start drawing 2D graphics on screen
	void Start2D();
	//End drawing 2D graphics on screen
	void End2D();

	//Window manager
	struct {
		float Width,Height;
		Vector2f Size;
	} Window;

	//Output screen dimensions
	float Width,Height;
	//Output screen position
	int Left,Top;

	Vector2f Size;

	//Is VBO supported?
	bool VBOSupported;
};

extern Screen_Manager Screen;

#endif

