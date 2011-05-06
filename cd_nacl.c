/*
  The NaCl version of Quake does not support CDAudio, so just provide stubs
*/


#include "quakedef.h"

void CDAudio_Stop() {
}

void CDAudio_Pause() {
}

void CDAudio_Resume() {
}

void CDAudio_Update() {
}

void CDAudio_Play(byte track, qboolean looping) {
}

int CDAudio_Init() {
  return -1;
}

void CDAudio_Shutdown() {
}

