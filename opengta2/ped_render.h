#ifndef PED_RENDER_H
#define PED_RENDER_H

//Get something from server part (ped animations)
#include "opengta_sv.h"

struct Ped_Render_Manager {
	Animation pedAnim[PED_ANIM_COUNT];

	void Initialize();
	void Render();
};

extern Ped_Render_Manager Ped_Render;

#endif