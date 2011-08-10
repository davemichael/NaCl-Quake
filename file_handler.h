// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <ppapi/cpp/file_io.h>
#include <ppapi/cpp/instance.h>
#include <tr1/functional>
#include <vector>

// FileHandler functions are used to read from a file to a vector or write from
// a vector to a file. It then calls you back when it's done.
namespace FileHandler {
  // Read all the data in the file and put the results in data. Assumes the
  // pointers remain valid until the file is done.
  void ReadFromFile(pp::FileIO* file, std::vector<char>* data,
                    std::tr1::function<void (int32_t)> progress_callback,
                    std::tr1::function<void ()> finished_callback);
  // Write all the data in data to the file. Assumes the pointers remain valid
  // until the file is done.
  void WriteToFile(pp::FileIO* file, std::vector<char>* data,
                   std::tr1::function<void (int32_t)> progress_callback,
                   std::tr1::function<void ()> finished_callback);
}

#endif  // FILE_HANDLER_H_

