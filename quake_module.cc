// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <cstdio>

#include <ppapi/cpp/module.h>

#include "quake_instance.h"

namespace nacl_quake {

// The Module class.  The browser calls the CreateInstance() method to create
// an instance of you NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-ppapi-nacl-srpc".
class QuakeModule : public pp::Module {
 public:
  QuakeModule() : pp::Module() {}
  virtual ~QuakeModule() {}

  // Create and return a QuakeInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    std::printf("Creating instance.\n");
    return new QuakeInstance(instance);
  }
};

}  // namespace nacl_quake

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
Module* CreateModule() {
  std::printf("Creating module.\n");
  return new nacl_quake::QuakeModule();
}
}  // namespace pp
