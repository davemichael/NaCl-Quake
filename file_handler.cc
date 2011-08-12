// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "file_handler.h"

#include "nacl/nacl_inttypes.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/cpp/completion_callback.h"
#include "nacl/nacl_inttypes.h"

#include <stdio.h>
#include <cstdlib>

#define PRINTF(...)
//#define PRINTF(...) printf(__VA_ARGS__)

#define CHECK(arg) assert(arg)

namespace {
  const int64_t kChunkSize = 4096;

  class ReadInProgress {
   public:
    ReadInProgress(pp::FileIO* file, std::vector<char>* data,
                   std::tr1::function<void (int32_t)> progress_callback,
                   std::tr1::function<void ()> finished_callback)
        : file_(file), data_(data), progress_callback_(progress_callback),
          finished_callback_(finished_callback), offset_(0),
          callback_factory_(this) {
      ReadMore();
    }
   private:
    void ReadMore() {
      PRINTF("ReadMore: size=%"NACL_PRId64", offset_=%"NACL_PRId64"\n",
             data_->size(), offset_);
      bool chunking = true;
      if (data_->size() <= (offset_ + kChunkSize))
        data_->resize(offset_ + kChunkSize);
      else
        chunking = false;
      file_->Read(offset_,
                  &data_->at(offset_),
                  (chunking ? kChunkSize : data_->size() - offset_),
                  callback_factory_.NewCallback(
                      &ReadInProgress::BytesWereRead));
    }
    void BytesWereRead(int64_t bytes_read) {
      PRINTF("BytesWereRead: bytes_read=%"NACL_PRId64", size=%"NACL_PRId64
             ", offset_=%"NACL_PRId64"\n",
             bytes_read, data_->size(), offset_);
      CHECK(bytes_read >= 0);
      offset_ += bytes_read;
      if (progress_callback_)
        progress_callback_(static_cast<int32_t>(bytes_read));
      if (bytes_read == 0) {
        // We may have over-grown the vector to handle a full chunk, so right-
        // size it.
        data_->resize(offset_);
        if (finished_callback_)
          finished_callback_();
        delete this;
      } else {
        ReadMore();
      }
    }

    pp::FileIO* file_;
    std::vector<char>* data_;
    std::tr1::function<void (int32_t)> progress_callback_;
    std::tr1::function<void ()> finished_callback_;
    int64_t offset_;
    pp::CompletionCallbackFactory<ReadInProgress> callback_factory_;
  };

  class WriteInProgress {
   public:
    WriteInProgress(pp::FileIO* file, std::vector<char>* data,
                    std::tr1::function<void (int32_t)> progress_callback,
                    std::tr1::function<void ()> finished_callback)
        : file_(file), data_(data), progress_callback_(progress_callback),
          finished_callback_(finished_callback), offset_(0),
          callback_factory_(this) {
      // reset the length to 0; we're replacing the contents.
      file_->SetLength(
          0,
          callback_factory_.NewCallback(&WriteInProgress::FileLengthWasSet));
    }
   private:
    void WriteMore() {
      PRINTF("WriteMore: size=%"NACL_PRId64", offset_=%"NACL_PRId64"\n",
             data_->size(), offset_);
      if (offset_ < data_->size()) {
        file_->Write(offset_,
                     &data_->at(offset_),
                     (data_->size() - offset_),
                     callback_factory_.NewCallback(
                         &WriteInProgress::BytesWereWritten));
      }
    }
    void FileLengthWasSet(int32_t result) {
      PRINTF("FileLengthWasSet result=%"NACL_PRId32"\n", result);
      // TODO(dmichael): Fails on Mac Canary, Linux Chrome 14.
      //CHECK(result == 0);
      BytesWereWritten(0);
    }
    void BytesWereWritten(int64_t bytes_written) {
      PRINTF("BytesWereWritten: bytes_written=%"NACL_PRId64", size=%"NACL_PRId64
             ", offset_=%"NACL_PRId64"\n",
             bytes_written, data_->size(), offset_);
      CHECK(bytes_written >= 0);
      offset_ += bytes_written;
      if (progress_callback_)
        progress_callback_(static_cast<int32_t>(bytes_written));
      if (offset_ >= data_->size()) {
        // We read it all!
        if (finished_callback_)
          finished_callback_();
        delete this;
      } else {
        WriteMore();
      }
    }

    pp::FileIO* file_;
    std::vector<char>* data_;
    std::tr1::function<void (int32_t)> progress_callback_;
    std::tr1::function<void ()> finished_callback_;
    int64_t offset_;
    pp::CompletionCallbackFactory<WriteInProgress> callback_factory_;
  };
}  // namespace

namespace FileHandler {
  void ReadFromFile(pp::FileIO* file, std::vector<char>* data,
                    std::tr1::function<void (int32_t)> progress_callback,
                    std::tr1::function<void ()> finished_callback) {
    new ReadInProgress(file, data, progress_callback, finished_callback);
  }
  void WriteToFile(pp::FileIO* file, std::vector<char>* data,
                   std::tr1::function<void (int32_t)> progress_callback,
                   std::tr1::function<void ()> finished_callback) {
    new WriteInProgress(file, data, progress_callback, finished_callback);
  }
}

