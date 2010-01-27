#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#pragma warning (disable : 4996)

/* @warning Nearly a copy of a macro in opengta2/chunckload.cpp
 */
#define FREAD(dest, size, times, src, filename)				\
{									\
	size_t const bytes_expected = (size) * (times);			\
	size_t const bytes_read = fread(				\
		(dest), (size), (times), (src));			\
									\
	if (bytes_expected != bytes_read) {				\
		fprintf(stderr, "Expected to read %d (%d * %d) bytes from %s into %s but could only read %d bytes in %s:%d (%s)",								\
			bytes_expected, (size), (times),		\
			filename, #dest,				\
			bytes_read,					\
			__FILE__, __LINE__, __func__			\
		);							\
		continue;						\
	}								\
}									\


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

		FILE* in = fopen(argv[i], "rb");
		if (!in) {
			fprintf(stderr, "Could not open file %d \"%s\" for reading\n", i-1, argv[i]);
			continue;
		}

		printf("Merging file %d (%s)\n", i-1, argv[i]);

		FREAD(chunkHeader, 1, 4, in, argv[i]);
		FREAD(&chunkSize, 4, 1, in, argv[i]);
		FREAD(&version, 4, 1, in, argv[i]);

		if (wroteHeader == false) {
			fwrite(chunkHeader,1,4,out);
			fwrite(&chunkSize,4,1,out);
			fwrite(&version,4,1,out);

			if (chunkSize != 4) {
				printf("Warning: no support for these files yet (they have more than 4 bytes in header chunk - %d bytes left)\n",chunkSize-4);
				return 0;
			}
		}

		while (fread(chunkHeader, 1, 4, in)) {
			FREAD(&chunkSize, 4, 1, in, argv[i]);

			printf("Copying chunk %s (size %d bytes)\n",chunkHeader,chunkSize);
			if ((chunkSize < 0) || (chunkSize > 1024*1024*256)) {
				printf("Nutsy chunk size (over 256 MB)\n");
				return 0;
			}

			void* buf;
			buf = malloc(chunkSize);

			FREAD(buf, 1, chunkSize, in, argv[i]);
			fwrite(chunkHeader,1,4,out);
			fwrite(&chunkSize,4,1,out);
			fwrite(buf,1,chunkSize,out);

			free(buf);
		}
		fclose(in);
	}

	fclose(out);
}

