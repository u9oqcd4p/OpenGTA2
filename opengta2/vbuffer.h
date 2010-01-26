#ifndef VBUFFER_H
#define VBUFFER_H

struct Vertex_Buffer {
	//Physical amount of variables in buffer
	int vertexAllocCount;
	//Formal size of the buffer
	int vertexCount;

	//Pointers to data
	Vector3f* vertexBuffer;
	Vector2f* texcoordBuffer;

	//Pointers to VBO data
	unsigned int vertexVBO;
	unsigned int texcoordVBO;

	//Allocate vertex buffer of this size
	void Create(int newCount);
	//Free up resources
	void Release();
	//Clear vertex buffer
	void Clear();
	//Add new entry to the buffer (warning: cant exceed vertexCount vertexes!)
	void Add(Vector3f v, Vector2f t);
	//Add new face to the buffer
	void AddFace(Vector3f v1, Vector3f v2, Vector3f v3, Vector3f v4);
	//Copy vertex buffer to the buffer
	void Copy(Vertex_Buffer* buf);

	//Send vertex buffer to videocard
	void SendToGPU();
	//Draw vertex buffer data
	void Draw();
};

#endif