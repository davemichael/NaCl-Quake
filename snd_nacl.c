#include "quakedef.h"
#include <pthread.h>

#include <stdio.h>
//#include "SDL_audio.h"
//#include "SDL_byteorder.h"

#if defined(STANDALONE)
#include "native_client/common/standalone.h"
#else
#include <nacl/nacl_av.h>
#endif

static dma_t the_shm;
volatile int snd_thread_running = 0;
volatile int paint_audio_thread_exited = 0;
static pthread_t snd_thread_id = 0;

void* nacl_paint_audio_thread(void *userdata) {
unsigned char buffer[32 * 1024];
int count = 0;
int rval = 0;
	paint_audio_thread_exited = 0;
	while(rval == 0) {
		volatile dma_t *cshm = shm;
		if (cshm) {
			rval = nacl_audio_stream(buffer, &count);
			cshm->buffer = buffer;
			cshm->samplepos += count/(cshm->samplebits/8)/2;
			S_PaintChannels (cshm->samplepos);
		}
		if (0 == snd_thread_running) break;
	}
	paint_audio_thread_exited = 1;
}


qboolean SNDDMA_Init(void)
{
	#define AUDIO_S16LSB 0x8010
	int audio_freq = 44100;
	int audio_samples = 4096;
	int audio_format = AUDIO_S16LSB;
	int audio_channels = 2;
	int obtained_samples;
	extern volatile dma_t *shm;

	/* Set up the desired format */
	shm = NULL;
	snd_thread_running = 1;

	/* create audio thread */
	Con_Printf("Spawning audio thread...\n");
	int p = pthread_create(&snd_thread_id, NULL, nacl_paint_audio_thread, NULL);
	if (0 != p) {
		Con_Printf("Unable to create audio thread!\n");
		snd_thread_running = 0;
		return 0;
	}
	Con_Printf("...done spawning pthread!\n");


	Con_Printf("Audio requested %d samples\n", audio_samples);

	/* Open the audio device */
	if ( nacl_audio_init(NACL_AUDIO_FORMAT_STEREO_44K, audio_samples, &obtained_samples) < 0 ) {
        	Con_Printf("Couldn't init nacl audio\n");
		snd_thread_running = 0;
		return 0;
	}

	audio_samples = obtained_samples;

	Con_Printf("Audio obtained %d samples\n", audio_samples);

	/* Fill the audio DMA information block */
	the_shm.splitbuffer = 0;
	the_shm.samplebits = (audio_format & 0xFF);
	the_shm.speed = audio_freq;
	the_shm.channels = audio_channels;
	the_shm.samples = audio_samples * the_shm.channels;
	the_shm.samplepos = 0;
	the_shm.submission_chunk = 1;
	the_shm.buffer = NULL;

	/* activate */
	shm = &the_shm;

	return 1;
}

int SNDDMA_GetDMAPos(void)
{
	return shm->samplepos;
}

void SNDDMA_Shutdown(void)
{
	if (snd_thread_running)
	{
		snd_thread_running = 0;
		nacl_audio_shutdown();
		while (paint_audio_thread_exited == 0)
			sched_yield();
	}
}
