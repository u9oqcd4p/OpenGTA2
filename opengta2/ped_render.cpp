#include "opengta_cl.h"
#include "opengta_sv.h"

Ped_Render_Manager Ped_Render;

void Ped_Render_Manager::Initialize() {
	pedAnim[PED_ANIM_IDLE]		= Animations["PED_IDLE"];
	pedAnim[PED_ANIM_WALK]		= Animations["PED_WALK"];
	pedAnim[PED_ANIM_RUN]		= Animations["PED_RUN"];
	pedAnim[PED_ANIM_FALL]		= Animations["PED_FALL"];
}

void Ped_Render_Manager::Render() {
	for (int i = 0; i < PED_ANIM_COUNT; i++) {
		pedAnim[i].Update();
	}

	LockID lockID = Thread.EnterLock(MUTEX_PED);
		for (uint i = 0; i < Peds.pedPool.Count; i++) {
			pedAnim[(*Peds.pedPool[i])->currentAnimation].startTime = (*Peds.pedPool[i])->animationStart;
			TexID texID = pedAnim[(*Peds.pedPool[i])->currentAnimation].GetTexID();

			//Draw at 1/64.0f scale to convert from pixels to units (1 unit = 64 pixels)
			Draw.Sprite3D(texID,(*Peds.pedPool[i])->Position+Vector3f(0.0f,0.0f,0.1f),1/64.0f,1,(*Peds.pedPool[i])->Heading); 
		}
		Draw.FlushSprites();
	Thread.LeaveLock(lockID);
}
