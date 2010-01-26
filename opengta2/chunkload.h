#ifndef CHUNKLOAD_H
#define CHUNKLOAD_H

struct Chunk_Loader {
	FILE* Data;

	//Open chunked file
	void Open(const char* FileName);
	//Close chunked file (dont forget this!)
	void Close();
	//Do file format ID and version match?
	bool IsFileFormat(const char FormatID[4], uint FormatVersion);
	//Read next chunk (skips any unread bytes in current one)
	bool ReadChunk();
	//Read from chunk
	int Read(void* buf, int sz);
	//Read 256-byte string (can be smaller)
	int ReadString(char buf[256]);
	//Check ID of current chunk
	bool IsChunk(const char ChunkID[4]);
	//Did we reach end of chunk?
	bool IsEndOfChunk();

	//Format of loaded file (4 chars)
	char fileFormat[4];
	char fileName[256]; //Filename
	uint fileVersion;
	uint fileSize;

	char chunkHeader[4];
	uint chunkSize;
	uint chunkPos;
	//char*
};

#endif
