// -*-C++-*-
// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#ifndef EXAMPLE_FLOW_ERROR_H_
#define EXAMPLE_FLOW_ERROR_H_

#include <stdarg.h>
#include <stdio.h>

#include "util/disallow_copy_and_assign.h"

namespace flow {

// error.cc
enum ErrorKind { kError, kNote };
class ErrorWriter {
 public:
  ErrorWriter(const char* path, FILE* error_stream);
  virtual ~ErrorWriter();
  void Error(unsigned int line, unsigned int column, ErrorKind kind);
  void FormatArgs(const char* format, va_list ap);
  void Format(const char* format, ...);
 private:
  class ErrorWriterImpl;
  ErrorWriterImpl* impl_;
  DISALLOW_COPY_AND_ASSIGN(ErrorWriter);
};
}  // namespace flow
#endif  // EXAMPLE_FLOW_ERROR_H_
