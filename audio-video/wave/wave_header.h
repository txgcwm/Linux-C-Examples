#ifndef _WAVE_HEADER_H_
#define _WAVE_HEADER_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// wav头部结构体
struct wave_header {
	char riff[4];
	unsignedlong fileLength;
	char wavTag[4];

	char fmt[4];
	unsignedlong size;
	unsignedshort formatTag;
	unsignedshort channel;
	unsignedlong sampleRate;
	unsignedlong bytePerSec;
	unsignedshort blockAlign;
	unsignedshort bitPerSample;

	char data[4];
	unsignedlong dataSize;
};

void *createWaveHeader(int fileLength, short channel, int sampleRate,
					   short bitPerSample);

#endif
