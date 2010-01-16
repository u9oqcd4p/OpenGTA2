#include "stdio.h"

char gbuffer[1024*1024*8];
FILE* log;

void extract(char* regionname) {
	char filename[256];

	sprintf(&filename[0],"%s.raw",regionname); FILE* snd = fopen(&filename[0],"rb");
	sprintf(&filename[0],"%s.sdt",regionname); FILE* sdt = fopen(&filename[0],"rb");

	if ((!snd) || (!sdt)) {
		printf("SDT2WAV: These files don't exist: %s.raw or %s.sdt\n",regionname,regionname);
		return;
	}

	printf("SDT2WAV: Unpacking %s.raw and %s.sdt\n",regionname,regionname);
	fprintf(log,"=============================================================\n");
	fprintf(log,"Unpacking: %s.raw and %s.sdt\n",regionname,regionname);

	int fileid = 0;
	fseek(sdt,0,2);
	int fmax = ftell(sdt) / 24;
	fseek(sdt,0,0);

	while (fileid < fmax) {
		printf("#%d | ",fileid);
		fprintf(log,"#%d | ",fileid);

		int start_offset,size,freq,unk1;
		unsigned short unk2,unk3;

		fread(&start_offset,4,1,sdt);
		fread(&size,4,1,sdt);
		fread(&freq,4,1,sdt);
		fread(&unk1,4,1,sdt);
		fread(&unk2,4,1,sdt);
		fread(&unk3,4,1,sdt);

		int t = ftell(sdt);

		printf("off: %d	| sz: %d	| freq %d	| %d	%d\n",start_offset,size,freq,unk2,unk3);
		fprintf(log,"Offset: %d	| Size: %d	| Frequency: %d	| Position:  %d	| Loop start: %d |	Loop size: %d\n",start_offset,size,freq,t,unk2,unk3);

		fseek(snd,start_offset,0);
		fread(&gbuffer,size,1,snd);

		//Write it
		int data;
		sprintf(&filename[0],"%s_%d.wav",regionname,fileid);
		FILE* wave = fopen(&filename[0],"wb+");

		//WAVE header:
		data = 0x46464952;		fwrite(&data,4,1,wave); //RIFF
		data = size+36;			fwrite(&data,4,1,wave); //Header size
		data = 0x45564157;		fwrite(&data,4,1,wave); //WAVE
		data = 0x20746D66;		fwrite(&data,4,1,wave); //fmt
		data = 16;				fwrite(&data,4,1,wave); //fixed1
		data = 1;				fwrite(&data,2,1,wave); //format tag
		data = 1;				fwrite(&data,2,1,wave); //Channels (mono)
		data = freq;			fwrite(&data,4,1,wave); //Sample rate
		data = freq*2;			fwrite(&data,4,1,wave); //Bytes per second
		data = 2;				fwrite(&data,2,1,wave); //Bytes per sample
		data = 16;				fwrite(&data,2,1,wave); //Bits per sample
		data = 0x61746164;		fwrite(&data,4,1,wave); //data
		data = size;			fwrite(&data,4,1,wave); //Audio data size

		fwrite(&gbuffer,size*2,1,wave);
		fclose(wave);

		fileid++;
	}

	printf("SDT2WAV: Finished %s.raw and %s.sdt\n",regionname,regionname);

	fclose(snd);
	fclose(sdt);
}


int main(int argc, char* argv[])
{
	log = fopen("std2wav.log","w+");
	if (argc < 2) {
		printf("SDT2WAV: No files specified, so enjoy this letter instead: M\n");
		fclose(log);
		return 1;
	} else {
		for (int i = 1; i < argc; i++) {
			extract(argv[i]);
		}
	}
	fclose(log);
	return 0;
}

