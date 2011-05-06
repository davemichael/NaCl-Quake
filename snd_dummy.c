
#include <stdio.h>
#include "SDL_audio.h"
#include "SDL_byteorder.h"
#include "quakedef.h"

static dma_t the_shm;
static int snd_inited;

extern int desired_speed;
extern int desired_bits;

static void paint_audio(void *unused, Uint8 *stream, int len)
{}

qboolean SNDDMA_Init(void)
{
	return 1;
}

int SNDDMA_GetDMAPos(void)
{
	return 0;
}

void SNDDMA_Shutdown(void)
{
}

