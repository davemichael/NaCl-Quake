// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_PI_GENERATOR_H_
#define EXAMPLES_PI_GENERATOR_H_

#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/size.h"
#include <pthread.h>

#include <map>
#include <vector>

namespace nacl_quake {

// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurence of the <embed> tag that has these
// attributes:
//     type="application/x-nacl"
//     nacl="nacl_quake.nmf"
class QuakeInstance : public pp::Instance {
 public:
  explicit QuakeInstance(PP_Instance instance);
  virtual ~QuakeInstance();

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual bool HandleInputEvent(const pp::InputEvent& event);

  // Update the graphcs context to the new size, and regnerate |pixel_buffer_|
  // to fit the new size as well.
  // Start up the LaunchQuake() thread.  HACK HACK HACK
  virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);

  void FilesFinished();
  // Called whenever the nexe downloads some bytes via url loader, or reads some
  // bytes from a file.
  void BytesWereRead(int32_t bytes);
  void BytesWereWritten(int32_t bytes);

  int width() const {
    return width_;
  }
  int height() const {
    return height_;
  }

 private:
  pthread_t quake_main_thread_;
  int width_;
  int height_;
  int32_t bytes_since_last_progress_;

  static void* LaunchQuake(void* param);
};

}  // namespace nacl_quake

#endif  // EXAMPLES_PI_GENERATOR_H_
