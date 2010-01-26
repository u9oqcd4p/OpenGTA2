#include "opengta_cl.h"
#include "string.h"

void Vertex_Buffer::Create(int newCount) {
	vertexBuffer   = (Vector3f*)mem.alloc(newCount*sizeof(Vector3f));
	texcoordBuffer = (Vector2f*)mem.alloc(newCount*sizeof(Vector2f));

	vertexAllocCount = newCount;
	vertexCount = 0;
}

void Vertex_Buffer::Release() {
	mem.free(vertexBuffer);
	mem.free(texcoordBuffer);
}

void Vertex_Buffer::Clear() {
	vertexCount = 0;
}

//FIXME: faster
void Vertex_Buffer::Copy(Vertex_Buffer* buf) {
	for (int i = 0; i < buf->vertexCount; i++)
		Add(buf->vertexBuffer[i],buf->texcoordBuffer[i]);
}

void Vertex_Buffer::AddFace(Vector3f v1, Vector3f v2, Vector3f v3, Vector3f v4) {
	Add(v1,Vector2f(0.0f,0.0f));
	Add(v2,Vector2f(0.0f,1.0f));
	Add(v3,Vector2f(1.0f,1.0f));
	Add(v4,Vector2f(1.0f,0.0f));
}

void Vertex_Buffer::Add(Vector3f v, Vector2f t) {
	if (vertexCount >= vertexAllocCount) {
		//logWrite("Vertex_Buffer: could not add vector to buffer (max size %d reached!)",vertexCount);
		return;
	}
	
	vertexBuffer[vertexCount] = v;   //Vertex
	texcoordBuffer[vertexCount] = t; //Texture coords

	vertexCount++;
}

void Vertex_Buffer::SendToGPU() {
	if (vertexCount == 0) return;
	if (Screen.VBOSupported) {
		if (vertexVBO != 0) {
			//FIXME: this is ugly
			glDeleteBuffersARB(1, &vertexVBO);
			glDeleteBuffersARB(1, &texcoordVBO);


			//glBindBufferARB(GL_ARRAY_BUFFER_ARB,vertexVBO);
			//Vector3f* ptr = (Vector3f*)glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_READ_WRITE_ARB);
			//memcpy(ptr,vertexBuffer,vertexSize*sizeof(Vector3f));

            //glUnmapBufferARB(GL_ARRAY_BUFFER_ARB); // release pointer to mapping buffer
			//FIXME:
			//logWrite("OMFG DONT DO THIS!!!!!!!!! DONT CALL SEND TO GPU!!!!");
        }// else {
			glGenBuffersARB(1,&vertexVBO);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,vertexVBO);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,vertexCount*sizeof(Vector3f),vertexBuffer,GL_STATIC_DRAW_ARB);

			glGenBuffersARB(1,&texcoordVBO);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,texcoordVBO);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB,vertexCount*sizeof(Vector2f),texcoordBuffer,GL_STATIC_DRAW_ARB);
		//}
	}
}

void Vertex_Buffer::Draw() {
	//here be dragons
	if (vertexCount == 0) return;
	if (Screen.VBOSupported) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexVBO);
		glVertexPointer(3, GL_FLOAT, 0, (char*)NULL); //Set the vertex pointer to the vertexBuffer
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, texcoordVBO);
		glTexCoordPointer(2, GL_FLOAT, 0, (char*)NULL ); // Set texcoord pointer to the texcoordBuffer

		glDrawArrays(GL_TRIANGLES, 0, vertexCount);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	} else 	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, vertexBuffer);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoordBuffer);

		glDrawArrays(GL_TRIANGLES, 0, vertexCount);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

}
