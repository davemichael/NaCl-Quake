// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "geturl_handler.h"

#include <ppapi/c/pp_errors.h>
#include <ppapi/c/ppb_instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/url_response_info.h>
#include <ppapi/cpp/var.h>
#include <cstdlib>

namespace {
// HTTP status values.
const int32_t kHTTPStatusOK = 200;
}  // namespace

GetURLHandler* GetURLHandler::Create(pp::Instance* instance,
                                     const std::string& url,
                                     size_t expected_size) {
  return new GetURLHandler(instance, url, expected_size);
}

GetURLHandler::GetURLHandler(pp::Instance* instance,
                             const std::string& url,
                             size_t expected_size)
    : url_(url),
      url_request_(instance),
      url_loader_(instance),
      cc_factory_(this) {
  url_request_.SetURL(url);
  url_request_.SetMethod("GET");
  // TODO(dmichael): Why does this crash?
  //url_request_.SetCustomContentTransferEncoding("compress, gzip");
  url_response_body_.reserve(expected_size);
}

GetURLHandler::~GetURLHandler() {
}

bool GetURLHandler::Start(SharedURLCallbackExecutor url_callback) {
  url_callback_ = url_callback;
  pp::CompletionCallback cc = cc_factory_.NewCallback(&GetURLHandler::OnOpen);
  int32_t result = url_loader_.Open(url_request_, cc);

  return result == PP_OK || result == PP_OK_COMPLETIONPENDING;
}

void GetURLHandler::OnOpen(int32_t result) {
  if (result != PP_OK) {
    url_callback_->Execute(url_response_body_, result);
  } else {
    // Process the response, validating the headers to confirm successful
    // loading.
    pp::URLResponseInfo url_response(url_loader_.GetResponseInfo());
    if (url_response.is_null()) {
      url_callback_->Execute(url_response_body_, PP_ERROR_FILENOTFOUND);
      return;
    }
    int32_t status_code = url_response.GetStatusCode();
    if (status_code != kHTTPStatusOK) {
      url_callback_->Execute(url_response_body_, PP_ERROR_FILENOTFOUND);
      return;
    }
    ReadBody();
  }
}

void GetURLHandler::OnRead(int32_t result) {
  if (result > 0) {
    // In this case, |result| is the number of bytes read.  Copy these into
    // the local buffer and continue.
    int32_t num_bytes = result < kBufferSize ? result : sizeof(buffer_);
    url_response_body_.reserve(url_response_body_.size() + num_bytes);
    url_response_body_.insert(url_response_body_.end(),
                              buffer_,
                              buffer_ + num_bytes);
    if (progress_func_) {
      progress_func_(num_bytes);
    }
    ReadBody();
  } else {
    // Either the end of the file was reached (|result| == PP_OK) or there
    // was an error.  Execute the callback in either case.
    url_callback_->Execute(url_response_body_, result);
  }
}

void GetURLHandler::ReadBody() {
  // Reads the response body (asynchronous) into this->buffer_.
  // OnRead() will be called when bytes are received or when an error occurs.
  // Look at <ppapi/c/dev/ppb_url_loader> for more details.
  pp::CompletionCallback cc = cc_factory_.NewCallback(&GetURLHandler::OnRead);
  int32_t res = url_loader_.ReadResponseBody(buffer_,
                                             sizeof(buffer_),
                                             cc);
}

