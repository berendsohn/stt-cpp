// Copyright (c) 2012--2014 David Eisenstat <eisenstatdavid@gmail.com>
//     and Brown University
// Released under http://opensource.org/licenses/MIT
// May 2014 version

#include "example/flow/error.h"

#include <assert.h>

namespace flow {

class ErrorWriter::ErrorWriterImpl {
 public:
  ErrorWriterImpl(const char* path, FILE* error_stream);
  void Error(unsigned int line, unsigned int column, ErrorKind kind);
  void FormatArgs(const char* format, va_list ap);
  void Format(const char* format, ...);
 private:
  const char* path_;
  FILE* error_stream_;
  DISALLOW_COPY_AND_ASSIGN(ErrorWriterImpl);
};

ErrorWriter::ErrorWriterImpl::ErrorWriterImpl(const char* path,
                                              FILE* error_stream)
    : path_(path), error_stream_(error_stream) {
}

void ErrorWriter::ErrorWriterImpl::Error(unsigned int line,
                                         unsigned int column,
                                         ErrorKind kind) {
  Format("%s:", path_);
  if (line != 0) {
    Format("%u:", line);
    if (column != 0) Format("%u:", column);
  }
  if (kind == kError) {
    Format(" error: ");
  } else if (kind == kNote) {
    Format(" note: ");
  } else {
    assert(false);
  }
}

void ErrorWriter::ErrorWriterImpl::FormatArgs(const char* format, va_list ap) {
  vfprintf(error_stream_, format, ap);
}

void ErrorWriter::ErrorWriterImpl::Format(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  FormatArgs(format, ap);
  va_end(ap);
}

ErrorWriter::ErrorWriter(const char* path, FILE* error_stream)
    : impl_(new ErrorWriterImpl(path, error_stream)) {
}

ErrorWriter::~ErrorWriter() {
  delete impl_;
}

void ErrorWriter::Error(unsigned int line,
                        unsigned int column,
                        ErrorKind kind) {
  impl_->Error(line, column, kind);
}

void ErrorWriter::FormatArgs(const char* format, va_list ap) {
  impl_->FormatArgs(format, ap);
}

void ErrorWriter::Format(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  FormatArgs(format, ap);
  va_end(ap);
}
}  // namespace flow
