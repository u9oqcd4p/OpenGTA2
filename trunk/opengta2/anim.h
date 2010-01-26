#ifndef ANIM_H
#define ANIM_H

typedef unsigned int AnimSeqID;

#define ANIMTYPE_FORWARDLOOP	0
#define ANIMTYPE_FORWARDONCE	1
#define ANIMTYPE_LOOP			2
#define ANIMTYPE_ONCE			3
#define ANIMTYPE_NONE			4
#define ANIMTYPE_RANDOM			5

struct Animation {
	AnimSeqID animSeq;
	TexID currentFrameTex;
	int currentFrame;
	float startTime;

	void Start();
	void Update();
	TexID GetTexID();
	float RemainingTime();
};

struct animation_entry {
	char* Name;
	int Type;

	float FPS;
	TexID* Sprites;
	int numSprites;
};

struct Animation_Manager {
	void Initialize();
	void Deinitialize();

	Animation GetAnimation(const char* name);
	Animation GetAnimation(AnimSeqID animseq);
	AnimSeqID GetAnimationSeq(const char* name);
	AnimSeqID GetAnimationSeq(TexID texture);
	void LoadFromChunk(Chunk_Loader* TEX);

	Animation operator[](const char* name) { return GetAnimation(name); }
	animation_entry* operator[](AnimSeqID animseq) { return Entries[animseq]; }

	DataArray<animation_entry> Entries;
};

extern Animation_Manager Animations;

#endif