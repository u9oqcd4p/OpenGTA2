#include "opengta_cl.h"

#ifdef _WIN32
	PFNGLGENBUFFERSARBPROC pglGenBuffersARB = 0;                     // VBO Name Generation Procedure
	PFNGLBINDBUFFERARBPROC pglBindBufferARB = 0;                     // VBO Bind Procedure
	PFNGLBUFFERDATAARBPROC pglBufferDataARB = 0;                     // VBO Data Loading Procedure
	PFNGLBUFFERSUBDATAARBPROC pglBufferSubDataARB = 0;               // VBO Sub Data Loading Procedure
	PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB = 0;               // VBO Deletion Procedure
	PFNGLGETBUFFERPARAMETERIVARBPROC pglGetBufferParameterivARB = 0; // return various parameters of VBO
	PFNGLMAPBUFFERARBPROC pglMapBufferARB = 0;                       // map VBO procedure
	PFNGLUNMAPBUFFERARBPROC pglUnmapBufferARB = 0;                   // unmap VBO procedure
#endif

Screen_Manager Screen;

void screenSizeChanged(int width, int height) {
	Screen.Window.Width = (float)width;
	Screen.Window.Height = (float)height;
	Screen.Window.Size.x = (float)width;
	Screen.Window.Size.y = (float)height;
	Screen.SetViewport(0,0,width,height);
}

void Screen_Manager::SetViewport(int left, int top, int width, int height) {
	Screen.Width = (float)width;
	Screen.Height = (float)height;
	Screen.Left = left;
	Screen.Top = top;
	glViewport(Screen.Left, Screen.Top, (int)Screen.Width, (int)Screen.Height);
	Camera.setParameters(Camera.FOV);

	Screen.Size.x = (float)width;
	Screen.Size.y = (float)height;
}

bool Screen_Manager::Initialize(int width, int height, bool fullscreen) {
	Width = (float)width;
	Height = (float)height;

	logWrite("Initializing output screen");//FIXME: fullscreen
	int mode = GLFW_WINDOW;
	if (fullscreen) mode = GLFW_FULLSCREEN;
	//glfwOpenWindowHint(GLFW_FSAA_SAMPLES,4);
	if (!glfwOpenWindow(width,height,8,8,8,8,24,0,mode)) {
		logWrite("Failed to initialize screen - reverting to text mode");
		//FIXME: add text mode
		glfwTerminate();
		return false;
    }

	glfwSetWindowTitle("OpenGTA2");

    //glfwEnable(GLFW_STICKY_KEYS);
    glfwSwapInterval(0);
	glfwSetWindowSizeCallback(screenSizeChanged);

	glViewport(0, 0, (int)Width, (int)Height);

	//Use perspective-corrected texcoord interpolation (vs. linear on-screen interpolation)
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	Clear();
	//Render.setPerspective(45);

	//Perform checks for extensions and load OpenGL library functions
	if (glfwExtensionSupported("GL_ARB_vertex_buffer_object")) {
		#ifdef _WIN32
			//Get function pointers
			glGenBuffersARB				= (PFNGLGENBUFFERSARBPROC)				glfwGetProcAddress("glGenBuffersARB");
			glBindBufferARB				= (PFNGLBINDBUFFERARBPROC)				glfwGetProcAddress("glBindBufferARB");
			glBufferDataARB				= (PFNGLBUFFERDATAARBPROC)				glfwGetProcAddress("glBufferDataARB");
			glBufferSubDataARB			= (PFNGLBUFFERSUBDATAARBPROC)			glfwGetProcAddress("glBufferSubDataARB");
			glDeleteBuffersARB			= (PFNGLDELETEBUFFERSARBPROC)			glfwGetProcAddress("glDeleteBuffersARB");
			glGetBufferParameterivARB	= (PFNGLGETBUFFERPARAMETERIVARBPROC)	glfwGetProcAddress("glGetBufferParameterivARB");
			glMapBufferARB				= (PFNGLMAPBUFFERARBPROC)				glfwGetProcAddress("glMapBufferARB");
			glUnmapBufferARB			= (PFNGLUNMAPBUFFERARBPROC)				glfwGetProcAddress("glUnmapBufferARB");
	
			//Check again
			if(glGenBuffersARB && glBindBufferARB && glBufferDataARB && glBufferSubDataARB &&
			glMapBufferARB && glUnmapBufferARB && glDeleteBuffersARB && glGetBufferParameterivARB) {
				VBOSupported = true;
			} else {
				logWrite("WARNING: some drivers or libraries may be missing, cant use VBO");
				VBOSupported = false;
			}
		#else
			VBOSupported = true;
		#endif
	} else {
		VBOSupported = false;
		logWrite("WARNING: VBO not supported, using vertex arrays instead");
	}

	return true;
}

void Screen_Manager::Deinitialize() {
	logWrite("Shutting down output screen");
	glfwTerminate();
}

void Screen_Manager::Clear() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDepthMask(GL_TRUE);
	glClearDepth(1.0f); glDepthFunc(GL_LEQUAL);	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glCullFace(GL_BACK);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);

	glEnable(GL_TEXTURE_2D); //FIXME: should properly set flags and stuff

	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

void Screen_Manager::Start2D() {
	//Set projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,Width,Height,0,-1.0f,1.0f);

	//Set view matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	//Setup 2D stuff
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void Screen_Manager::End2D() {
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
}