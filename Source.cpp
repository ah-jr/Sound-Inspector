#include <iostream>
#include <conio.h>
#include <tchar.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <sys\timeb.h>
#include <stdbool.h>

#define pi 3.141592
#define MED 2
#define MAX_THREADS 3
#define BUF_SIZE 255
#define FREQ 500

int _ESCAPE = 0;
int SongReady = 0;

void clean_stdin()
{
	while (getchar() != '\n');
}

struct Dados {
	int print[MED];
	int printFourrier[FREQ + 1];
	float F[FREQ];
	float sumX[601];
	float sumX2[1240];
	FILE* fptr;
	FILE* fptr2;
	FILE* fptr3;
	int ThreadDone;
	int ThreadDone2;
	char indexName[100];
	char songName[100];
	SDL_Window* window1, *window2, *window3;
	SDL_Renderer* renderer1, *renderer2, *renderer3;
};

void HideConsole()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

void ShowConsole()
{
	::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}


DWORD WINAPI PlayMusic(void* dataa) {
	Dados* data2 = (Dados*)dataa;
	Dados data = *data2;
	PlaySound(NULL, NULL, SND_ASYNC);
	SongReady++;
	while (SongReady != 4) Sleep(1);

	PlaySound(data.indexName, NULL, SND_ASYNC);

	return 0;
}


DWORD WINAPI WaveGraph(void* dataa) {
	Dados* data2 = (Dados*)dataa;
	Dados data = *data2;
	LARGE_INTEGER frequency;
	LARGE_INTEGER _start;
	LARGE_INTEGER _end;
	double duration;
	QueryPerformanceFrequency(&frequency);

	int n, i, j, f;
	j = 0;
	unsigned char byte;

	SDL_Window* window = data.window1;
	SDL_Renderer* renderer = data.renderer1;
	SDL_bool done = SDL_FALSE;

	SongReady++;
	while (SongReady != 4) Sleep(1);

	Sleep(100);
	while (!feof(data.fptr)) {
		QueryPerformanceCounter(&_start);

		for (f = 0; f < 600; f++) {
			data.sumX[f] = 0;

			for (i = 0; i < MED; i++) {
				for (n = 0; n < 4; n++) fread(&byte, sizeof(unsigned char), 1, data.fptr);
				data.print[i] = byte;
				if (data.print[i] > 127) data.print[i] = data.print[i] - 256;
				data.sumX[f] = data.sumX[f] + data.print[i];
			}
			data.sumX[f] = data.sumX[f] / MED;
		}
		SDL_Event event;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		SDL_bool done = SDL_FALSE;
		for (f = 1; f < 601; f++) {

			SDL_SetRenderDrawColor(renderer, (Uint8)(-1 * 255 * sin(pi * j / (1200) - pi / 2)), (Uint8)(255 * sin(pi * j / (1200) - pi / 2)), (Uint8)(255 * sin(pi * j / (1200) - pi / 2)), SDL_ALPHA_OPAQUE);
			SDL_RenderDrawLine(renderer, 10 + j - 1, 150 - (int)data.sumX[f - 1], 10 + j, 150 - (int)data.sumX[f]);
			j++;
			if (j >= 600) j = 0;
		}

		SDL_RenderPresent(renderer);
		SDL_PollEvent(&event);
		QueryPerformanceCounter(&_end);
		duration = (double)(_end.QuadPart - _start.QuadPart) / frequency.QuadPart;
		while (duration < (float)1 / ((float)22050 / 600)) {
			QueryPerformanceCounter(&_end);
			duration = (double)(_end.QuadPart - _start.QuadPart) / frequency.QuadPart;
		}
		if (GetKeyState(VK_ESCAPE) & 0x80000000) {
			_ESCAPE = 1;
			return 0;
		}
	}
	data2->ThreadDone = 1;
	return 0;
}


DWORD WINAPI WaveGraph2(void* dataa) {
	Dados* data2 = (Dados*)dataa;
	Dados data = *data2;
	LARGE_INTEGER frequency;
	LARGE_INTEGER _start;
	LARGE_INTEGER _end;
	double duration;
	QueryPerformanceFrequency(&frequency);

	int n, i, j, f;
	j = 0;
	unsigned char byte;

	SDL_Window* window = data.window2;
	SDL_Renderer* renderer = data.renderer2;


	SongReady++;
	while (SongReady != 4) Sleep(1);

	Sleep(100);
	while (!feof(data.fptr2)) {
		QueryPerformanceCounter(&_start);

		for (i = 0; i < FREQ; i++) {
			for (n = 0; n < 2 * 4; n++) fread(&byte, sizeof(unsigned char), 1, data.fptr2);
			data.printFourrier[i] = byte;
			if (data.printFourrier[i] > 127) data.printFourrier[i] = data.printFourrier[i] - 256;
		}


		for (f = 0; f < FREQ; f++) {
			data.F[f] = 0;
			for (i = 0; i < FREQ; i++) {
				data.F[f] =(float)(data.F[f] + data.printFourrier[i] * cos(-2 * pi * (f) * (i) / (float)(2 * FREQ)));
			}
			data.F[f] = data.F[f] / FREQ;
			if (data.F[f] < 0) data.F[f] = data.F[f] * (-1);
			data.F[f] = data.F[f] * 100;
			if (data.F[f] > 100) data.F[f] = data.F[f] * (float)2 / 3;
			if (data.F[f] > 150) data.F[f] = data.F[f] * (float)3 / 5;
			if (data.F[f] > 200) data.F[f] = data.F[f] * (float)2 / 5;

		}

		data.F[0] = (float) 0.05;

		SDL_Event event;
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		for (f = 1; f < FREQ; f++) {
			SDL_SetRenderDrawColor(renderer, (Uint8) (255 * sin(pi * 2 * f / (float)(2 * 1240) - pi / 2)), (Uint8) 255, (Uint8) (-255 * sin(pi * 2 * f / (float)(2 * 1240) - pi / 2)), SDL_ALPHA_OPAQUE);

			SDL_RenderDrawLine(renderer, 10 + 2 * (f - 1), 400 - (int)data.F[f - 1], 10 + 2 * f, 400 - (int)data.F[f]);
		}

		SDL_RenderPresent(renderer);
		SDL_PollEvent(&event);
		QueryPerformanceCounter(&_end);
		duration = (double)(_end.QuadPart - _start.QuadPart) / frequency.QuadPart;
		while (duration < (float)1 / ((float)22050 / FREQ)) {
			QueryPerformanceCounter(&_end);
			duration = (double)(_end.QuadPart - _start.QuadPart) / frequency.QuadPart;
		}
		if (GetKeyState(VK_ESCAPE) & 0x80000000) {
			_ESCAPE = 1; 
			return 0;
		}
	}
	printf("Spectrum B\n");
	data2->ThreadDone2 = 1;
	return 0;
}


DWORD WINAPI Volume(void* dataa) {
	Dados* data2 = (Dados*)dataa;
	Dados data = *data2;
	LARGE_INTEGER frequency;
	LARGE_INTEGER _start;
	LARGE_INTEGER _end;
	double duration, SoundVolumeR, SoundVolumeL;
	QueryPerformanceFrequency(&frequency);

	int n, i, j, f;
	j = 0;
	unsigned char byte;

	SDL_Window* window = data.window3;
	SDL_Renderer* renderer = data.renderer3;
	SDL_bool done = SDL_FALSE;

	SongReady++;
	while (SongReady != 4) Sleep(1);

	Sleep(100);
	while (!feof(data.fptr3)) {
		QueryPerformanceCounter(&_start);

		for (f = 0; f < 600; f++) {
			data.sumX[f] = 0;

			for (n = 0; n < 2; n++) fread(&byte, sizeof(unsigned char), 1, data.fptr3);
			data.sumX2[f] = byte;
			for (n = 0; n < 2; n++) fread(&byte, sizeof(unsigned char), 1, data.fptr3);
			data.sumX[f] = byte;
			if (data.sumX[f] > 127) data.sumX[f] = data.sumX[f] - 256;
			if (data.sumX2[f] > 127) data.sumX2[f] = data.sumX2[f] - 256;
				
			data.sumX[f] = abs(data.sumX[f]);
			data.sumX2[f] = abs(data.sumX2[f]);
		}
		SDL_Event event;

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		SoundVolumeL = 0;
		SoundVolumeR = 0;
		for (f = 0; f < 300; f++) {
			if (data.sumX2[f] > SoundVolumeR) SoundVolumeR = data.sumX2[f];
			if (data.sumX[f] > SoundVolumeL) SoundVolumeL = data.sumX[f];
			//SoundVolumeR = SoundVolumeR + data.sumX2[f];
			//SoundVolumeL = SoundVolumeL + data.sumX[f];
		}
		SoundVolumeR = SoundVolumeR * 1.5;
		SoundVolumeL = SoundVolumeL * 1.5;
		//SoundVolumeR = SoundVolumeR / 100;
		//SoundVolumeL = SoundVolumeL / 100;
		if (SoundVolumeR > 200) SoundVolumeR = 200;
		if (SoundVolumeL > 200) SoundVolumeL = 200;

		SDL_bool done = SDL_FALSE;

		SDL_SetRenderDrawColor(renderer, (Uint8)(-1 * 255 * sin(pi * j / (1200) - pi / 2)), (Uint8)(255 * sin(pi * j / (1200) - pi / 2)), (Uint8)(255 * sin(pi * j / (1200) - pi / 2)), SDL_ALPHA_OPAQUE);
		for (i = 0; i < 30; i++)
		{
			SDL_RenderDrawLine(renderer, 30 + i, 250, 30 + i, 250 - (int)SoundVolumeR);
			SDL_RenderDrawLine(renderer, 90 + i, 250, 90 + i, 250 - (int)SoundVolumeL);
		}

		SDL_RenderPresent(renderer);
		SDL_PollEvent(&event);
		QueryPerformanceCounter(&_end);
		duration = (double)(_end.QuadPart - _start.QuadPart) / frequency.QuadPart;
		while (duration < (float)1 / ((float)22050 / 300)) {
			QueryPerformanceCounter(&_end);
			duration = (double)(_end.QuadPart - _start.QuadPart) / frequency.QuadPart;
		}
		if (GetKeyState(VK_ESCAPE) & 0x80000000) {
			_ESCAPE = 1;
			return 0;
		}
	}
	printf("Thread 1 Done");
	data2->ThreadDone = 1;
	return 0;
}


int main(int argc, char* argv[]) {

	int scanReturn;
	char onemore = 'Y';
	int n, j;
	unsigned char byte;
	int done, fileOK;
	int graphType;
	DWORD WaveID, SpectrumID, PlayID, VolumeID;
	Dados data;

	printf("===================================    SOUND INSPECTOR    ====================================\n");
	printf("                                    made by: A. H. Junior                                     \n");
	printf("\n");

	while (onemore == 'Y' || onemore == 'y') {

		HWND console = GetConsoleWindow();
		RECT r;
		GetWindowRect(console, &r); //stores the console's current dimensions
		MoveWindow(console, 10, 10, 800, 800, TRUE);

		j = 0;
		data.ThreadDone = 0;
		data.ThreadDone2 = 0;
		graphType = 1;
		fileOK = 0;
		done = 0;

		while (!fileOK) {
			printf("Copy your audio file to the \"WAV files\" folder and enter its title: ");

			scanReturn = scanf("%s", data.songName);

			strcpy(data.indexName, "WAV files/");
			strcat(data.indexName, data.songName);

			if (!(data.indexName[strlen(data.indexName) - 1] == 'v'
				&& data.indexName[strlen(data.indexName) - 2] == 'a'
				&& data.indexName[strlen(data.indexName) - 3] == 'w'
				&& data.indexName[strlen(data.indexName) - 4] == '.')) strcat(data.indexName, ".wav");

			if ((data.fptr = fopen(data.indexName, "rb")) == NULL)
				printf("File doesn't exist! Try again:\n");
			else {
				data.fptr2 = fopen(data.indexName, "rb");
				data.fptr3 = fopen(data.indexName, "rb");
				fileOK = 1;
			}
		}


		do {
			if (!(graphType == 1 || graphType == 2 || graphType == 3 || graphType == 4)) printf("\nINVALID OPTION!\n\n");
			else printf("Graph Type: \n");
			printf("1: Wave Inspector \n");
			printf("2: Frequency Spectrum \n");
			printf("3: Volume \n");
			printf("4: All \n");
			printf("Option: ");
			clean_stdin();
			scanReturn = scanf("%d", &graphType);
			if (!scanReturn) graphType = 5;
		} while (!(graphType == 1 || graphType == 2 || graphType == 3 || graphType == 4));

		printf("\nATTENTION: When the audio begins, press ESC to close the graphs.\n");
		printf("Press ENTER to start.\n\n");

		Sleep(500);
		while (!(GetAsyncKeyState(VK_RETURN) & 0x80000000)) Sleep(10);

		// Descartar o HEADER:
		for (n = 1; n < (17 + 2 * 16); n++) fread(&byte, sizeof(unsigned char), 1, data.fptr);
		for (n = 1; n < (17 + 2 * 16); n++) fread(&byte, sizeof(unsigned char), 1, data.fptr2);
		for (n = 1; n < (17 + 2 * 16); n++) fread(&byte, sizeof(unsigned char), 1, data.fptr3);
		// =============================================================================

		HideConsole();


		if (SDL_Init(SDL_INIT_VIDEO) == 0) {
			HANDLE thread = CreateThread(NULL, 0, PlayMusic, &data, 0, &PlayID);


			switch (graphType) {
				case 1:
				{
					data.window1 = SDL_CreateWindow("WaveForm", 100, 100, 640, 300, 0);
					data.renderer1 = SDL_CreateRenderer(data.window1, -1, SDL_RENDERER_ACCELERATED);
					SongReady++; 
					SongReady++;
					HANDLE threadGraph = CreateThread(NULL, 0, WaveGraph, &data, 0, &WaveID);
					break;
				}

				case 2:
				{
					data.window2 = SDL_CreateWindow("Spectrum", 100, 500, 1050, 500, 0);
					data.renderer2 = SDL_CreateRenderer(data.window2, -1, SDL_RENDERER_ACCELERATED);
					SongReady++;
					SongReady++;
					HANDLE threadGraph2 = CreateThread(NULL, 0, WaveGraph2, &data, 0, &SpectrumID);
					break;
				}
				case 3:
				{
					data.window3 = SDL_CreateWindow("Volume", 800, 100, 150, 300, 0);
					data.renderer3 = SDL_CreateRenderer(data.window3, -1, SDL_RENDERER_ACCELERATED);
					SongReady++;
					SongReady++;
					HANDLE threadGraph = CreateThread(NULL, 0, Volume, &data, 0, &WaveID);
					break;
				}
				case 4:
				{
					data.window1 = SDL_CreateWindow("WaveForm", 100, 100, 640, 300, 0);
					data.renderer1 = SDL_CreateRenderer(data.window1, -1, SDL_RENDERER_ACCELERATED);
					data.window2 = SDL_CreateWindow("Spectrum", 100, 500, 1050, 500, 0);
					data.renderer2 = SDL_CreateRenderer(data.window2, -1, SDL_RENDERER_ACCELERATED);
					data.window3 = SDL_CreateWindow("Volume", 800, 100, 150, 300, 0);
					data.renderer3 = SDL_CreateRenderer(data.window3, -1, SDL_RENDERER_ACCELERATED);
					HANDLE threadGraph = CreateThread(NULL, 0, Volume, &data, 0, &WaveID);
					HANDLE threadGraph2 = CreateThread(NULL, 0, WaveGraph2, &data, 0, &SpectrumID);
					HANDLE threadGraph3 = CreateThread(NULL, 0, WaveGraph, &data, 0, &VolumeID);
					break;
				}
			}

			while (!done){
				Sleep(10);
				if (_ESCAPE || (data.ThreadDone && data.ThreadDone2)) 
					done = 1;
			}
		}

		PlaySound(NULL, NULL, SND_ASYNC);

		SDL_DestroyRenderer(data.renderer1);
		SDL_DestroyRenderer(data.renderer2);
		SDL_DestroyRenderer(data.renderer3);
		SDL_DestroyWindow(data.window1);
		SDL_DestroyWindow(data.window2);
		SDL_DestroyWindow(data.window3);
		fclose(data.fptr);
		fclose(data.fptr2);
		fclose(data.fptr3);
		
		SDL_Quit();
		SongReady = 0;
		_ESCAPE = 0;

		ShowConsole();

		printf("One more song? Y/N: ");
		scanReturn = scanf(" %c", &onemore);
	}
	
	return 0;
}
