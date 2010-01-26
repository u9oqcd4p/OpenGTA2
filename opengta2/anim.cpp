#include "opengta_cl.h"

Animation_Manager Animations;

void Animation_Manager::Initialize() {
	Entries.Preallocate(1024); //FIXME: another static allocation.. make convar when we have console!
}

void Animation_Manager::Deinitialize() {
	for (uint i = 0; i < Entries.Count; i++) {
		if (Entries[i]->Sprites) mem.free(Entries[i]->Sprites);
	}
	Entries.Release();
}

Animation Animation_Manager::GetAnimation(const char* name) {
	return GetAnimation(GetAnimationSeq(name));
}

Animation Animation_Manager::GetAnimation(AnimSeqID animseq) {
	Animation anim;
	
	anim.animSeq = animseq;
	anim.currentFrameTex = BAD_ID;
	anim.currentFrame = 0;
	anim.startTime = -1.0f;

	return anim;
}

AnimSeqID Animation_Manager::GetAnimationSeq(const char* name) {
	int nameLen = strlen(name);
	for (uint i = 0; i < Entries.Count; i++) {
		if (strncmp(Entries[i]->Name,name,nameLen) == 0) return i;
	}
	logWritem("Animation not found: %s",name);
	return BAD_ID;
}

AnimSeqID Animation_Manager::GetAnimationSeq(TexID texture) {
	for (uint i = 0; i < Entries.Count; i++) {
		if ((Entries[i]->Sprites) &&
		    (Entries[i]->Sprites[0] == texture)) return i;
	}
	return BAD_ID;
}

void Animation_Manager::LoadFromChunk(Chunk_Loader* TEX) {
	if (!TEX->IsChunk("ANIM")) return;

	if (!TEX->IsEndOfChunk()) {
		logWritem("Loading animations...");
	} else {
		logWritem("Warning: animation chunk is empty");
	}
	
	while (!TEX->IsEndOfChunk()) {
		char sprStartName[256]; 
		char sprEndName[256];
		char animName[256];
		float animFPS;
		char animType;

		TEX->ReadString(animName);
		TEX->Read(&animType,1);

		logWritem("\t%s [type: %.2x]",animName,animType);

		animation_entry* anim = Animations.Entries.Add();
		anim->Type = animType;
		anim->Name = mem.alloc_string(strlen(animName)+1);
		strcpy(anim->Name,animName);

		switch (animType) { //
			case ANIMTYPE_FORWARDLOOP:
			case ANIMTYPE_FORWARDONCE:
				TEX->ReadString(sprStartName);
				TEX->ReadString(sprEndName);
				TEX->Read(&animFPS,4);
				anim->FPS = animFPS;

				anim->Sprites = (TexID*)mem.alloc(2*sizeof(TexID));
				anim->Sprites[0] = Graphics.GetTextureID(sprStartName);
				anim->Sprites[1] = Graphics.GetTextureID(sprEndName);
				anim->numSprites = 2;
				break;
			default:
				logError("Animation: unsupported type %d",animType);
				return;
		}
	}
}

void Animation::Start() {
	startTime = ServerTimer.Time();
}

void Animation::Update() {
	float Time = ServerTimer.Time() - startTime;
	int NumFrames = Animations[animSeq]->numSprites;
	
	if (Animations[animSeq]->Type < 2) { //FIXME: use define constant
		NumFrames = Animations[animSeq]->Sprites[1] - Animations[animSeq]->Sprites[0] + 1;
	}

	switch (Animations[animSeq]->Type) {
		case ANIMTYPE_FORWARDLOOP:
			currentFrame = (int)floor(Time * Animations[animSeq]->FPS) % (NumFrames);
			break;
		case ANIMTYPE_FORWARDONCE:
			currentFrame = min((int)floor(Time * Animations[animSeq]->FPS),NumFrames-1);
			break;
	}
}

TexID Animation::GetTexID() {
	if (startTime <= 0.0f) {
		Update(); //FIXME 
	}

	int Frames;
	switch (Animations[animSeq]->Type) {
		case ANIMTYPE_FORWARDLOOP:
		case ANIMTYPE_FORWARDONCE:
			Frames = Animations[animSeq]->Sprites[1] - Animations[animSeq]->Sprites[0] + 1;
			if ((currentFrame >= 0) && (currentFrame < Frames))	return Animations[animSeq]->Sprites[0] + currentFrame;
			break;

		default:
			if ((currentFrame >= 0) && (currentFrame < Animations[animSeq]->numSprites))
				return Animations[animSeq]->Sprites[currentFrame];
			break;
	}

	return BAD_ID;
}

float Animation::RemainingTime() {
	return 1.0f / Animations.Entries[animSeq]->FPS;
}