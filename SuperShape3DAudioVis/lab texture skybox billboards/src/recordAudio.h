#pragma once
#include <Windows.h>
#include <mutex>
#define MAXS 100000
#define MAXSAMPLE 100000
class captureAudio
{
public:
	BYTE data[MAXS];
	int size;
	captureAudio()
	{
		size = 0;
		//data = NULL;
	}
	void writeAudio(BYTE *data_, int size_);
	void readAudio(BYTE *data_, int &size_);
};

void start_recording();