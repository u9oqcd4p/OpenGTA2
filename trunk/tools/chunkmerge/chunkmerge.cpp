#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#pragma warning (disable : 4996)

int main(int argc, char* argv[])
{
	if (argc < 2) return 0;
	FILE* out = fopen(argv[1],"wb");
	bool wroteHeader = false;

	//Merge all input files
	for (int i = 2; i < argc; i++) {
		char chunkHeader[5] = "    ";
		int chunkSize;
		int version;

		FILE* in = fopen(argv[i],"rb");

		printf("Merging file %d (%s)\n",i-1,argv[i]);

		fread(chunkHeader,1,4,in);
		fread(&chunkSize,4,1,in);
		fread(&version,4,1,in);

		if (wroteHeader == false) {
			fwrite(chunkHeader,1,4,out);
			fwrite(&chunkSize,4,1,out);
			fwrite(&version,4,1,out);

			if (chunkSize != 4) {
				printf("Warning: no support for these files yet (they have more than 4 bytes in header chunk - %d bytes left)\n",chunkSize-4);
				return 0;
			}
		}

		while (fread(chunkHeader,1,4,in)) {
			fread(&chunkSize,4,1,in);

			printf("Copying chunk %s (size %d bytes)\n",chunkHeader,chunkSize);
			if ((chunkSize < 0) || (chunkSize > 1024*1024*256)) {
				printf("Nutsy chunk size (over 256 MB)\n");
				return 0;
			}

			void* buf;
			buf = malloc(chunkSize);

			fread(buf,1,chunkSize,in);
			fwrite(chunkHeader,1,4,out);
			fwrite(&chunkSize,4,1,out);
			fwrite(buf,1,chunkSize,out);

			free(buf);
		}
		fclose(in);
	}

	fclose(out);
}