/**
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <Exception.h>
#include "io/validation.h"
#include "io/FileStream.h"
#include "io/InputStream.h"
#include "io/OutputStream.h"
namespace org {
namespace apache {
namespace nifi {
namespace minifi {
namespace io {

FileStream::FileStream(const std::string &path, bool append)
    : logger_(logging::LoggerFactory<FileStream>::getLogger()),
      path_(path),
      offset_(0) {
  file_stream_ = std::unique_ptr<std::fstream>(new std::fstream());
  if (append) {
    file_stream_->open(path.c_str(), std::fstream::in | std::fstream::out | std::fstream::app | std::fstream::binary);
    file_stream_->seekg(0, file_stream_->end);
    file_stream_->seekp(0, file_stream_->end);
    std::streamoff len = file_stream_->tellg();
    length_ = len > 0 ? gsl::narrow<size_t>(len) : 0;
    seek(offset_);
  } else {
    file_stream_->open(path.c_str(), std::fstream::out | std::fstream::binary);
    length_ = 0;
  }
}

FileStream::FileStream(const std::string &path, uint32_t offset, bool write_enable)
    : logger_(logging::LoggerFactory<FileStream>::getLogger()),
      path_(path),
      offset_(offset) {
  file_stream_ = std::unique_ptr<std::fstream>(new std::fstream());
  if (write_enable) {
    file_stream_->open(path.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
  } else {
    file_stream_->open(path.c_str(), std::fstream::in | std::fstream::binary);
  }
  file_stream_->seekg(0, file_stream_->end);
  file_stream_->seekp(0, file_stream_->end);
  std::streamoff len = file_stream_->tellg();
  if (len > 0) {
    length_ = gsl::narrow<size_t>(len);
  } else {
    length_ = 0;
  }
  seek(offset);
}

void FileStream::close() {
  std::lock_guard<std::mutex> lock(file_lock_);
  file_stream_.reset();
}

void FileStream::seek(uint64_t offset) {
  std::lock_guard<std::mutex> lock(file_lock_);
  offset_ = gsl::narrow<size_t>(offset);
  file_stream_->clear();
  file_stream_->seekg(offset_);
  file_stream_->seekp(offset_);
}

int FileStream::write(const uint8_t *value, int size) {
  gsl_Expects(size >= 0);
  if (size == 0) {
    return 0;
  }
  if (!IsNullOrEmpty(value)) {
    std::lock_guard<std::mutex> lock(file_lock_);
    if (file_stream_->write(reinterpret_cast<const char*>(value), size)) {
      offset_ += size;
      if (offset_ > length_) {
        length_ = offset_;
      }
      if (!file_stream_->flush()) {
        return -1;
      }
      return size;
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}

int FileStream::read(uint8_t *buf, int buflen) {
  gsl_Expects(buflen >= 0);
  if (buflen == 0) {
    return 0;
  }
  if (!IsNullOrEmpty(buf)) {
    std::lock_guard<std::mutex> lock(file_lock_);
    if (!file_stream_) {
      return -1;
    }
    file_stream_->read(reinterpret_cast<char*>(buf), buflen);
    if ((file_stream_->rdstate() & (file_stream_->eofbit | file_stream_->failbit)) != 0) {
      file_stream_->clear();
      file_stream_->seekg(0, file_stream_->end);
      file_stream_->seekp(0, file_stream_->end);
      auto tellg_result = file_stream_->tellg();
      if (tellg_result < 0) {
        logging::LOG_ERROR(logger_) << "Tellg call on file stream failed.";
        return -1;
      }
      size_t len = gsl::narrow<size_t>(tellg_result);
      size_t ret = len - offset_;
      offset_ = len;
      length_ = len;
      logging::LOG_DEBUG(logger_) << path_ << " eof bit, ended at " << offset_;
      return gsl::narrow<int>(ret);
    } else {
      offset_ += buflen;
      file_stream_->seekp(offset_);
      return buflen;
    }

  } else {
    return -1;
  }
}

} /* namespace io */
} /* namespace minifi */
} /* namespace nifi */
} /* namespace apache */
} /* namespace org */

