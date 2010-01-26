#include "opengta_cl.h"

Draw_Manager Draw;

void Draw_Manager::Initialize() {
	VBO.Create(DRAW_VBO_SIZE);
	VBO.Clear();
	currentGraphicsID = BAD_ID; //No graphics bound

	curVBOSwitches = 0;
	curTextureBinds = 0;
	curSprites = 0;
	curPolies = 0;
}

void Draw_Manager::Frame() {
	statVBOSwitches = curVBOSwitches;
	statTextureBinds = curTextureBinds;
	statSprites = curSprites;
	statPolies = curPolies;

	curVBOSwitches = 0;
	curTextureBinds = 0;
	curSprites = 0;
	curPolies = 0;

	currentGraphicsID = BAD_ID;

	VBO.Clear();
}

void Draw_Manager::Deinitialize() {
	VBO.Release();
}

//Draw sprites which are yet to be drawn
void Draw_Manager::FlushSprites() {
	drawVBO();
	//currentGraphicsID = BAD_ID;
}

void Draw_Manager::drawVBO() {
	if (VBO.vertexCount > 0) {
		VBO.SendToGPU();
		VBO.Draw();
		VBO.Clear();

		curVBOSwitches++;
	}
}

void Draw_Manager::Sprite2D(TexID texID, Vector2f pos, float scale, int align, float rotation) {
	Sprite3D(texID,Vector3f(pos.x,pos.y,0.0f),scale,align,rotation);
}

void Draw_Manager::Sprite3D(TexID texID, Vector3f pos, float scale, int align, float rotation) {
	texture_entry* texEntry = Graphics.GetTextureEntryByID(texID);
	GraphicsID newGraphicsID = Graphics.GetGraphicsEntryByID(texID);
	if (texID == BAD_ID) return;// texEntry;

	if (newGraphicsID != currentGraphicsID) {
		drawVBO();

		//Switch current OpenGL texture
		OGLTexture textureOGLID = Graphics.GetOGLTextureByID(texID);
		glBindTexture(GL_TEXTURE_2D, textureOGLID);

		currentGraphicsID = newGraphicsID;
		//logWrite("GID %d",currentGraphicsID);

		curTextureBinds++;
	}

	if (VBO.vertexCount+6 > VBO.vertexAllocCount) {
		drawVBO();
	}

	float w = texEntry->Width*scale;
	float h = texEntry->Height*scale;

	if (rotation == 0.0f) {
		if (align == 0) {
			VBO.Add(Vector3f(pos.x+w,pos.y+h,pos.z),	Vector2f(texEntry->U2,texEntry->V2));
			VBO.Add(Vector3f(pos.x+w,pos.y,  pos.z),	Vector2f(texEntry->U2,texEntry->V1));
			VBO.Add(Vector3f(pos.x,  pos.y,  pos.z),	Vector2f(texEntry->U1,texEntry->V1));
			
			VBO.Add(Vector3f(pos.x+w,pos.y+h,pos.z),	Vector2f(texEntry->U2,texEntry->V2));
			VBO.Add(Vector3f(pos.x,  pos.y,  pos.z),	Vector2f(texEntry->U1,texEntry->V1));
			VBO.Add(Vector3f(pos.x,  pos.y+h,pos.z),	Vector2f(texEntry->U1,texEntry->V2));
		} else {
			VBO.Add(Vector3f(pos.x+w/2,pos.y+h/2,pos.z),Vector2f(texEntry->U2,texEntry->V2));
			VBO.Add(Vector3f(pos.x+w/2,pos.y-h/2,pos.z),Vector2f(texEntry->U2,texEntry->V1));
			VBO.Add(Vector3f(pos.x-w/2,pos.y-h/2,pos.z),Vector2f(texEntry->U1,texEntry->V1));
			
			VBO.Add(Vector3f(pos.x+w/2,pos.y+h/2,pos.z),Vector2f(texEntry->U2,texEntry->V2));
			VBO.Add(Vector3f(pos.x-w/2,pos.y-h/2,pos.z),Vector2f(texEntry->U1,texEntry->V1));
			VBO.Add(Vector3f(pos.x-w/2,pos.y+h/2,pos.z),Vector2f(texEntry->U1,texEntry->V2));
		}
	} else {
		Vector3f center = Vector3f(pos.x,pos.y,pos.z);
		Vector3f v1 = Vector3f(pos.x-w/2,pos.y-h/2,pos.z);
		Vector3f v2 = Vector3f(pos.x+w/2,pos.y-h/2,pos.z);
		Vector3f v3 = Vector3f(pos.x+w/2,pos.y+h/2,pos.z);
		Vector3f v4 = Vector3f(pos.x-w/2,pos.y+h/2,pos.z);

		v1.Rotate_Z(rotation*RAD_TO_DEG,center);
		v2.Rotate_Z(rotation*RAD_TO_DEG,center);
		v3.Rotate_Z(rotation*RAD_TO_DEG,center);
		v4.Rotate_Z(rotation*RAD_TO_DEG,center);

		VBO.Add(v3,	Vector2f(texEntry->U2,texEntry->V2));
		VBO.Add(v2,	Vector2f(texEntry->U2,texEntry->V1));
		VBO.Add(v1,	Vector2f(texEntry->U1,texEntry->V1));
		
		VBO.Add(v3,	Vector2f(texEntry->U2,texEntry->V2));
		VBO.Add(v1,	Vector2f(texEntry->U1,texEntry->V1));
		VBO.Add(v4,	Vector2f(texEntry->U1,texEntry->V2));
	}

	curPolies += 6;
	curSprites++;

	//return texEntry;
}

void Draw_Manager::Rectangle2D(Vector2f pos1, Vector2f pos2, Vector4f color) {
	//FIXME: what I was supposed to fix here
	drawVBO();

	glDisable(GL_TEXTURE_2D);
	VBO.Add(Vector3f(pos1.x,	pos1.y,	0),	Vector2f(0,0));
	VBO.Add(Vector3f(pos2.x,	pos1.y,	0),	Vector2f(0,0));
	VBO.Add(Vector3f(pos2.x,	pos2.y,	0),	Vector2f(0,0));
			
	VBO.Add(Vector3f(pos1.x,	pos1.y,	0),	Vector2f(0,0));
	VBO.Add(Vector3f(pos2.x,	pos2.y,	0),	Vector2f(0,0));
	VBO.Add(Vector3f(pos1.x,	pos2.y,	0),	Vector2f(0,0));

	glColor4f(color.x,color.y,color.z,color.w);
	drawVBO();

	glEnable(GL_TEXTURE_2D);
}

void Draw_Manager::Line2D(Vector2f pos1, Vector2f pos2, Vector4f color) {
	glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINES);
		glColor4f(color.x,color.y,color.z,color.w);
		glVertex2f(pos1.x, pos1.y);
		glVertex2f(pos2.x, pos2.y);
		glColor4f(1,1,1,1); //Reset color
    glEnd();
	glEnable(GL_TEXTURE_2D);
}

void Draw_Manager::Line3D(Vector3f pos1, Vector3f pos2, Vector4f color) {
	glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINES);
		glColor4f(color.x,color.y,color.z,color.w);
		glVertex3f(pos1.x, pos1.y, pos1.z);
		glVertex3f(pos2.x, pos2.y, pos2.z);
		glColor4f(1,1,1,1); //Reset color
    glEnd();
	glEnable(GL_TEXTURE_2D);
}

/*void Draw_Manager::Draw2DToVBO(Vertex_Buffer VBO, int texID, Vector2f pos, float scale) {

}

void Draw_Manager::Draw2D(int texID, Vector2f pos, float scale) {
	
}*/