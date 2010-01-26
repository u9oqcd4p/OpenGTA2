#include "opengta_cl.h"
#include "math.h"

Map_Render_Debug Map_Render;

void Map_Render_Debug::wakeupPlane(int bx1, int by1, int bx2, int by2, int bz) {
	int tx = (int)floor(bx1 / 64.0f);
	int ty = (int)floor(by1 / 64.0f);
	rmp_cityscape* cityscape = Map.cellCityscape(tx,ty); //cityscape to gather data from

	//if (bz > cityscape->maxHeight) return;
	
	for (int x = bx1; x <= bx2; x++)
		for (int y = by1; y <= by2; y++) {
			int cx = (int)floor(x / 64.0f); //current cityscape
			int cy = (int)floor(y / 64.0f);
			if ((tx != cx)  || (ty != cy)) {
				tx = cx; ty = cy; //precache new one
				cityscape = Map.cellCityscape(tx,ty);
			}
		}
}

//FIXME: a lot less map requests (therefore a lot higher map rendering speed) can be made
//if you draw volumes of cells separately. transparency may break
void Map_Render_Debug::drawPlane(int bx1, int by1, int bx2, int by2, int bz) {
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

			int kx = x - 64*(x / 64);//abs(x % 64);//x-abs(cx*64);
			int ky = y - 64*(y / 64);//abs(y % 64);//y-abs(cy*64);

			//int texID = 

			if ((cityscape->rmp_city_scape) && (bz < cityscape->maxHeight)) {
				Vertex_Buffer* vBuffer = 0;
				TexID texBase = 0;
				for (uint i = 0; i < VBOEntry.Count; i++) {
					if (strcmp(VBOEntry[i]->GraphicsName,cityscape->GraphicsName[0]) == 0) { 
						vBuffer = &VBOEntry[i]->VBO;
						texBase = VBOEntry[i]->SpriteBase;
						break;
					}
				}

				if (vBuffer == 0) {
					//debugrender_vboentry* vboEntry = VBOEntry.Add();
					VBOEntry.Count++; //Add();
					if (VBOEntry.Count >= VBOEntry.AllocCount) {
						logWrite("BAD ERROR: attempting to draw more different VBO's than allocated entries");
						return;
					}
					VBOEntry[VBOEntry.Count-1]->GraphicsName = cityscape->GraphicsName[0];
					VBOEntry[VBOEntry.Count-1]->VBO.Clear();
					vBuffer = &VBOEntry[VBOEntry.Count-1]->VBO;

					//Get sprite base TexID to use later to bind texture atlas (also this is offset for blockgeometry)
					char texName[256];
					snprintf(texName,256,"%s_0",VBOEntry[VBOEntry.Count-1]->GraphicsName);
					VBOEntry[VBOEntry.Count-1]->SpriteBase = Graphics.GetTextureID(texName);
					texBase = VBOEntry[VBOEntry.Count-1]->SpriteBase;
				}

				//Block fetched from map:
				rmp_block_info* block = &cityscape->rmp_city_scape[bz*64*64+ky*64+kx];
				//Block with (possibly) animated textures:
				rmp_block_info _block;
				memcpy(&_block,block,sizeof(rmp_block_info));

				TexID tex_left =	texBase+block->tex_left;
				TexID tex_right =	texBase+block->tex_right;
				TexID tex_top =		texBase+block->tex_top;
				TexID tex_bottom =	texBase+block->tex_bottom;
				TexID tex_lid =		texBase+block->tex_lid;

				AnimSeqID anim_left		= Animations.GetAnimationSeq(tex_left);
				AnimSeqID anim_right	= Animations.GetAnimationSeq(tex_right);
				AnimSeqID anim_top		= Animations.GetAnimationSeq(tex_top);
				AnimSeqID anim_bottom	= Animations.GetAnimationSeq(tex_bottom);
				AnimSeqID anim_lid		= Animations.GetAnimationSeq(tex_lid);

				//Animate faces
				float tgtAnimationTime = 0.0f;
				if (anim_left != BAD_ID) { //HAX
					Animation anim = Animations.GetAnimation(anim_left);
					anim.startTime = 0.0f;

					_block.tex_left = anim.GetTexID()-texBase;
					tgtAnimationTime = Timer.Time() + anim.RemainingTime();
				}
				if (anim_right != BAD_ID) {
					Animation anim = Animations.GetAnimation(anim_right);
					anim.startTime = 0.0f;

					_block.tex_right = anim.GetTexID()-texBase;
					tgtAnimationTime = Timer.Time() + anim.RemainingTime();
				}
				if (anim_top != BAD_ID) {
					Animation anim = Animations.GetAnimation(anim_top);
					anim.startTime = 0.0f;

					_block.tex_top = anim.GetTexID()-texBase;
					tgtAnimationTime = Timer.Time() + anim.RemainingTime();
				}
				if (anim_bottom != BAD_ID) {
					Animation anim = Animations.GetAnimation(anim_bottom);
					anim.startTime = 0.0f;

					_block.tex_bottom = anim.GetTexID()-texBase;
					tgtAnimationTime = Timer.Time() + anim.RemainingTime();
				}
				if (anim_lid != BAD_ID) {
					Animation anim = Animations.GetAnimation(anim_lid);
					anim.startTime = 0.0f;

					_block.tex_lid = anim.GetTexID()-texBase;
					tgtAnimationTime = Timer.Time() + anim.RemainingTime();
				}
				if (tgtAnimationTime > 0.0f) {
					if (nextAnimationTime > 0.0f) {
						nextAnimationTime = min(tgtAnimationTime,nextAnimationTime);
					} else {
						nextAnimationTime = tgtAnimationTime;
					}
				}

				//if ((block->block_type & 0xF) > 0) 
				Map_Vertex_Geometry.blockGeometry(vBuffer, &_block, texBase, Vector3f(x*1.0f,y*1.0f,bz*1.0f-1.0f));
			}
		}
}

void Map_Render_Debug::Render() {
	float tempMatrix[16];

	//Generate camera matrix
	Camera.generateMatrix();

	//Set projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Matrix.toOGLMatrix(tempMatrix,Camera.projMatrix);
	glLoadMatrixf(tempMatrix);

	//Set view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Matrix.toOGLMatrix(tempMatrix,Camera.viewMatrix);
	glLoadMatrixf(tempMatrix);

	bool sameVBO = true;
	if (floor(Camera.Position.x) != floor(vboPosition.x)) sameVBO = false;
	if (floor(Camera.Position.y) != floor(vboPosition.y)) sameVBO = false;
	if (floor(Camera.Position.z) != floor(vboPosition.z)) sameVBO = false;
	vboPosition = Camera.Position;

	float DY = max(0,Camera.Position.z * tan(Camera.FOV * PI / 180.0f));
	float DX = max(0,DY * (1.0f*Screen.Width / Screen.Height));

	if ((nextAnimationTime > 0.0f) && (Timer.Time() > nextAnimationTime)) { //Cause animation
		sameVBO = false;
		nextAnimationTime = 0.0f;
	}

	if (sameVBO == false) {
		//int i = 1;
		VBOEntry.Clear();
		for (int i = 0; i < 7; i++) {
			drawPlane((int)(Camera.Position.x-DX),
					  (int)(Camera.Position.y-DY),
					  (int)(Camera.Position.x+DX),
				 	  (int)(Camera.Position.y+DY),i);
		}
	} else {
		//Wake up maps in cache
		if (Timer.Time() - lastWakeupTime > 10.0f) {
			for (int i = 0; i < 16; i++)
				wakeupPlane((int)(Camera.Position.x-DX),
							(int)(Camera.Position.y-DY),
							(int)(Camera.Position.x+DX),
							(int)(Camera.Position.y+DY),i);
			lastWakeupTime = Timer.Time();
		}
	}

	//SETUP DOF
	//glBlendFunc(GL_SRC_ALPHA,GL_DST_COLOR);
	//glBlendColor(0.0f,0.0f,0.5f,0.5f);
	//glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	//Draw level using VBOs
	GraphicsID graphicsID = BAD_ID;
	for (uint i = 0; i < VBOEntry.Count; i++) {
		TexID texID = VBOEntry[i]->SpriteBase;
		if (texID == BAD_ID) {
			logError("Trying to draw level without sprite set loaded");
			continue;
		}
		GraphicsID newGraphicsID = Graphics.GetGraphicsEntryByID(texID);

		if (newGraphicsID != graphicsID) {
			OGLTexture textureOGLID = Graphics.GetOGLTextureByID(texID);
			glBindTexture(GL_TEXTURE_2D, textureOGLID);
			graphicsID = newGraphicsID;
		}

		VBOEntry[i]->VBO.SendToGPU();

		//Draw normal
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		VBOEntry[i]->VBO.Draw();

		//Draw DOF
		//float drange = 0.1f;
		//float dstep = 0.02f;

		//DOF
		/*glColor4f(1.0f, 1.0f, 1.0f, 1.0f / (2.0f * drange / dstep));
		for (float disp = -drange; disp <= drange; disp += dstep) {
			float viewMatrix[16];
			Matrix.buildLookAtMatrix(Camera.Position.x+disp, Camera.Position.y,	Camera.Position.z,
									Camera.Position.x, Camera.Position.y,	Camera.Position.z - 3.0f,
									0.0f,				-1.0f,				0.0f,
									viewMatrix);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			Matrix.toOGLMatrix(tempMatrix,viewMatrix);
			glLoadMatrixf(tempMatrix);
	
			glClear(GL_DEPTH_BUFFER_BIT);
			VBOEntry[i]->VBO.Draw();
		}
		for (float disp = -drange; disp <= drange; disp += dstep) {
			float viewMatrix[16];
			Matrix.buildLookAtMatrix(Camera.Position.x, Camera.Position.y+disp,	Camera.Position.z,
									Camera.Position.x, Camera.Position.y,	Camera.Position.z - 3.0f,
									0.0f,				-1.0f,				0.0f,
									viewMatrix);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			Matrix.toOGLMatrix(tempMatrix,viewMatrix);
			glLoadMatrixf(tempMatrix);
	
			glClear(GL_DEPTH_BUFFER_BIT);
			VBOEntry[i]->VBO.Draw();
		}*/
	}

	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//Draw level (debug)
	//OGLTexture textureOGLID = Graphics.GetOGLTextureByID(0);
	//glBindTexture(GL_TEXTURE_2D, textureOGLID);
	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_COLOR,GL_DST_COLOR);

	//VBO.Draw();
}

void Map_Render_Debug::Initialize() {
    //glShadeModel(GL_SMOOTH);
    //glEnable(GL_LIGHTING);
	//glEnable(GL_NORMALIZE);
    //glEnable(GL_TEXTURE_2D);
    //glEnable(GL_CULL_FACE);

	nextAnimationTime = 0.0f;
	VBOEntry.Preallocate(4);
	for (uint i = 0; i < VBOEntry.AllocCount; i++) {
		logWritem("creating vertex buffer %d",i);
		VBOEntry[i]->VBO.Create(32768*3);//131072);//0);
	}
	//FIXME: VBOs take A LOT of space
}

void Map_Render_Debug::Deinitialize() {
	for (uint i = 0; i < VBOEntry.AllocCount; i++)
		VBOEntry[i]->VBO.Release();
	VBOEntry.Release();
	//VBO.Release();
}
