#ifndef MAP_RENDER_H
#define MAP_RENDER_H

//THIS DEFINES ENTIRE 3D WORLD RENDERING PIPELINE
//struct Map_Render_Base {
	//Called when map and all map objects should be rendered
	//virtual void Render();
	//Called each frame to update map geometry and all map objects
	//virtual void Frame();
//};

//Classic map render is without shadows or anything
//struct Map_Render_Classic : Map_Render_Base {
	//Called when map and all map objects should be rendered
	//void Render();
	//Called each frame to update map geometry and all map objects
	//void Frame();
//};

struct debugrender_vboentry {
	char* GraphicsName;
	Vertex_Buffer VBO;
	TexID SpriteBase;
};

struct Map_Render_Debug {
	//Called when map and all map objects should be rendered
	void Render();
	void Initialize();
	void Deinitialize();

	//Generate plane geometry (INTERNAL, IT USES VBOEntry)
	void drawPlane(int bx1, int by1, int bx2, int by2, int bz);
	//Wake up plane (refresh it in any map caches, so it doesnt get baleeted)
	void wakeupPlane(int bx1, int by1, int bx2, int by2, int bz);

	//Time when wakeupPlane was last called (INTERNAL)
	float lastWakeupTime;
	//Time when VBO must be refreshed cause something got animated
	//FIXME: replace, no need to rebuild entire VBO cause of one tile
	float nextAnimationTime; 

	Vector3f vboPosition; //camera position valid for current VBO
	DataArray<debugrender_vboentry> VBOEntry;
};

extern Map_Render_Debug Map_Render;

#endif