#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#pragma warning (disable : 4996)

int main(int argc, char* argv[])
{
	if (argc < 3) {
		printf("Bad arguments specified! :(\n");
		return 1;
	} else {
		FILE* in = fopen(argv[1],"r");
		FILE* out = fopen(argv[2],"wb+");
		
		int temp = 1;
		while (temp) {
			char fileLine[8192];
			char chunkHeader[5] = "   ";
			int chunkSize;
			int chunkRewritePos;
			int repeatStart;
			int repeatSize;
			int repeatCount;

			//temp = 
			if (!fgets(fileLine,8192,in)) {
				temp = 0;
				fileLine[0] = '\0';
			}//fscanf(in,"%s\n",fileLine);
			//temp = !temp;

			if (strncmp(fileLine,"CHUNK",5) == 0) {
				sscanf(fileLine,"CHUNK %c%c%c%c",&chunkHeader[0],&chunkHeader[1],&chunkHeader[2],&chunkHeader[3]);
				fwrite(chunkHeader,1,4,out);
				chunkRewritePos = ftell(out);
				chunkSize = 0;
				fwrite(&chunkSize,4,1,out);
				continue;
			} 
			if (strncmp(fileLine,"ENDCHUNK",8) == 0) {
				fseek(out,chunkRewritePos,0);
				fwrite(&chunkSize,4,1,out);
				fseek(out,chunkSize,1);

				printf("Chunk %s [%d bytes]\n",chunkHeader,chunkSize);
				continue;
			}
			if (strncmp(fileLine," REP",4) == 0) {
				sscanf(fileLine," REP %d\n",&repeatCount);
				repeatStart = ftell(out);
				repeatSize = 0;
				continue;
			}
			if (strncmp(fileLine," ENDREP",7) == 0) {
				char buf[16384];

				repeatSize = ftell(out) - repeatStart;
				fseek(out,repeatStart,0);
				fread(buf,1,repeatSize,out);
				fseek(out,repeatStart+repeatSize,0);
				for (int i = 1; i < repeatCount; i++) {
					fwrite(buf,1,repeatSize,out);
					chunkSize += repeatSize;
				}
				printf("Repeat %d times (%d bytes block, %d bytes total)\n",repeatCount,repeatSize,repeatCount*repeatSize);
				continue;
			}
			if (strncmp(fileLine,"  BYTE",6) == 0) {
				char temp;
				sscanf(fileLine,"  BYTE %d",&temp);
				fwrite(&temp,1,1,out);
				chunkSize += 1;
				continue;
			}
			if (strncmp(fileLine,"  WORD",6) == 0) {
				unsigned short temp;
				sscanf(fileLine,"  WORD %d",&temp);
				fwrite(&temp,2,1,out);
				chunkSize += 2;
				continue;
			}
			if (strncmp(fileLine,"  INT",5) == 0) {
				unsigned int temp;
				sscanf(fileLine,"  INT %d",&temp);
				fwrite(&temp,4,1,out);
				chunkSize += 4;
				continue;
			}
			if (strncmp(fileLine,"  FLT",5) == 0) {
				float temp;
				sscanf(fileLine,"  FLT %f",&temp);
				fwrite(&temp,4,1,out);
				chunkSize += 4;
				continue;
			}
			/*if (strncmp(fileLine,"  BYTEARG",9) == 0) {
				char temp;
				int index;
				sscanf(fileLine,"  BYTEARG %d",&index);
				fwrite(&temp,1,1,out);
				chunkSize += 1;
			}
			if (strncmp(fileLine,"  WORD",6) == 0) {
				unsigned short temp;
				sscanf(fileLine,"  WORD %d",&temp);
				fwrite(&temp,2,1,out);
				chunkSize += 2;
			}*/
			if (strncmp(fileLine,"  ARGINT",8) == 0) { //FIXME: will error when someone adds ARGBYTE
				int temp;
				int index;
				sscanf(fileLine,"  ARGINT %d",&index);
				temp = atoi(argv[index+3]);
				fwrite(&temp,4,1,out);
				chunkSize += 4;
			}
			if (strncmp(fileLine,"  STR",5) == 0) {
				char tempStr[256];
				sscanf(fileLine,"  STR %s",tempStr);
				
				int tempSz = strlen(tempStr);
				chunkSize += fwrite(&tempSz,1,1,out);
				chunkSize += fwrite(tempStr,1,tempSz,out);
			}
			if (strncmp(fileLine,"  CHAR",6) == 0) {
				char temp;
				sscanf(fileLine,"  CHAR %c",&temp);
				
				fwrite(&temp,1,1,out);
				chunkSize += 1;
			}
			if (strncmp(fileLine,"  CHAR4",7) == 0) {
				char temp[4];
				sscanf(fileLine,"  CHAR4 %c%c%c%c",&temp[0],&temp[1],&temp[2],&temp[3]);
				fwrite(temp,1,4,out);
				chunkSize += 4;
			}


			//int entries;
			//fscanf(in,"ENTRIES %d\n",entries);
		}
		fclose(in);
		fclose(out);
	}
	return 0;
}