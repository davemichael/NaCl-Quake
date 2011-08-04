// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "quake_instance.h"

#include <stdlib.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <tr1/functional>

extern "C" {
#include <SDL_video.h>
extern int quake_main(int argc, char* argv[]);
#include <SDL.h>
#include <SDL_nacl.h>
}

#include "nacl_file.h"
#include "ppapi/cpp/audio.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/var.h"

//#define PRINTF(...)
#define PRINTF(...) printf(__VA_ARGS__)

namespace {
  int32_t kBytesPerProgressUpdate = 300000;
}

using namespace std::tr1::placeholders;

namespace nacl_quake {

QuakeInstance::QuakeInstance(PP_Instance instance)
    : pp::Instance(instance),
      quake_main_thread_(NULL),
      width_(0),
      height_(0),
      bytes_since_last_progress_(0) {
  PRINTF("Created instance.\n");
  // Tell the browser we want to handle all Mouse & Keyboard events.
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD);
}

QuakeInstance::~QuakeInstance() {
  SDL_Event quit_event;
  quit_event.type = SDL_QUIT;
  SDL_PushEvent(&quit_event);
  if (quake_main_thread_) {
    pthread_join(quake_main_thread_, NULL);
  }
}

void QuakeInstance::DidChangeView(const pp::Rect& position,
                                const pp::Rect& clip) {
  PRINTF("Changing view, (%d, %d) to (%d, %d).\n", width_, height_,
         position.size().width(), position.size().height());
  if (position.size().width() == width() &&
      position.size().height() == height())
    return;  // Size didn't change, no need to update anything.

  // HACK HACK HACK.  Apparently this is so we only initialize video once.
  if (width_ && height_)
    return;

  width_ = position.size().width();
  height_ = position.size().height();

  SDL_NACL_SetInstance(pp_instance(), width_, height_);
  int lval = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  assert(lval >= 0);
}

void QuakeInstance::FilesFinished() {
  if (bytes_since_last_progress_) {
    PostMessage(bytes_since_last_progress_);
    bytes_since_last_progress_ = 0;
  }
}

void QuakeInstance::DownloadedBytes(int32_t bytes) {
  bytes_since_last_progress_ += bytes;
  if (bytes_since_last_progress_ >= kBytesPerProgressUpdate) {
    PostMessage(bytes_since_last_progress_);
    bytes_since_last_progress_ = 0;
  }
}

bool QuakeInstance::Init(uint32_t argc, const char* argn[], const char* argv[]) {
  PRINTF("Init called.  Setting up files.\n");
  using nacl_file::FileManager;
  FileManager::set_pp_instance(this);
  FileManager::set_ready_func(std::tr1::bind(&QuakeInstance::FilesFinished,
                                             this));
  FileManager::set_progress_func(std::tr1::bind(&QuakeInstance::DownloadedBytes,
                                                this, _1));
//#define USEPAK
#ifdef USEPAK
  FileManager::Fetch("id1/config.cfg", 1724u);
  FileManager::Fetch("id1/pak0.pak", 18689235u);
#else 
#include "file_list.h"
  size_t i = 0;
  while (file_list[i]) {
    FileManager::Fetch(file_list[i]);
    ++i;
  }
#endif 
  // Launch the quake 'main' thread.
  pthread_create(&quake_main_thread_, NULL, LaunchQuake, this);
  return true;
}

bool QuakeInstance::HandleInputEvent(const pp::InputEvent& event) {
  // TODO: Why does SDL need it to be non-const?
  SDL_NACL_PushEvent(const_cast<pp::InputEvent*>(&event));
  return true;
}

void* QuakeInstance::LaunchQuake(void* param) {
  PRINTF("Launching Quake.\n");
  quake_main(0, NULL);

  return NULL;
}

}  // namespace nacl_quake

