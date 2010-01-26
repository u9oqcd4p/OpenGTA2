#ifndef SPRITE_H
#define SPRITE_H

#define ALIGN_TOPLEFT 0
#define ALIGN_CENTER 1

struct Draw_Manager {
	//void Draw2DToVBO(Vertex_Buffer VBO, int texID, Vector2f pos, float scale = 1.0f);
	//void Draw2D(int texID, Vector2f pos, float scale = 1.0f);
	void Initialize();
	void Deinitialize();
	void Frame();

	//Draw sprites which are yet to be drawn
	void FlushSprites();

	//Draw 3D sprite
	void Sprite3D(TexID texID, Vector3f pos, float scale = 1.0f, int align = 0, float rotation = 0.0f);
	//Draw sprite and return sprites parameterss
	void Sprite2D(TexID texID, Vector2f pos, float scale = 1.0f, int align = 0, float rotation = 0.0f);
	//Draw sprite, and modify current texID to reflect animation
	//void AnimSprite2D(TexID* texID, Vector2f pos, float scale = 1.0f, int align = 0);

	//Draw basic shapes
	void Rectangle2D(Vector2f pos1, Vector2f pos2, Vector4f color);
	void Line2D(Vector2f pos1, Vector2f pos2, Vector4f color);
	void Line3D(Vector3f pos1, Vector3f pos2, Vector4f color);

	//Vertex buffer for sprites
	void drawVBO();
	Vertex_Buffer VBO;
	GraphicsID currentGraphicsID;

	int curVBOSwitches;
	int curTextureBinds;
	int curSprites;
	int curPolies;
	int statVBOSwitches;
	int statTextureBinds;
	int statSprites;
	int statPolies;
};

extern Draw_Manager Draw;

#endif