#include "nacl_file.h"

#include <errno.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include <tr1/functional>

#include "geturl_handler.h"
#include "nacl/nacl_inttypes.h"

//#define PRINTF(...)
#define PRINTF(...) printf(__VA_ARGS__)

extern "C" {
int __wrap_open(const char* pathname, int mode, int perms);
int __wrap_close(int dd);
int __wrap_read(int, void*, size_t);
off_t __wrap_lseek(int, off_t, int);
int __real_write(int, void*, size_t);
int __wrap_write(int, void*, size_t);
int nacl_file_write(int, void*, size_t);
int nacl_file_length(int fd);
}

namespace {
  std::string StripPath(const std::string& file_name) {
    if ((file_name.length() > 2) && (file_name.substr(0,2) == "./")) {
      return file_name.substr(2);
    }
    return file_name;
  }
}

// NOTE:  HACK:  None of this is thread-safe.  The idea is that all files must
// be completely downloaded before they are accessed, so there is no thread
// contention.
namespace nacl_file {

File::File(const std::string& name_arg, std::vector<uint8_t>& data_arg)
    : name(StripPath(name_arg)) {
  data.swap(data_arg);
  PRINTF("Created 'File' object for %s, data is %zd bytes.\n", name.c_str(), data.size());
}

int FileHandle::Read(void* buffer, size_t num_bytes) {
  PRINTF("Attempting read of %s, position %zd, size %zd, %zd bytes.\n", file->name.c_str(), position, file->data.size(), num_bytes);
  if (position == file->data.size()) {
    PRINTF("Returning 0; EOF.\n");
    return 0;  // EOF
  }
  int bytes_to_read(std::min(num_bytes, file->data.size() - position));
  memcpy(buffer, &file->data[position], bytes_to_read);
  position += bytes_to_read;
  PRINTF("Read %zd bytes successfully, setting position to %zd.\n", bytes_to_read, position);
  return bytes_to_read;
}

// WARNING, Write is pretty much untested and probably doesn't work.
int FileHandle::Write(void* buffer, size_t num_bytes) {
  PRINTF("Attempting write of %s, position %zd, size %zd, %zd bytes.\n", file->name.c_str(), position, file->data.size(), num_bytes);
  PRINTF("Attempting write of %s, position %zd, size %zd, %zd bytes.\n", file->name.c_str(), position, file->data.size(), num_bytes);
  if (position + num_bytes > file->data.size()) {
    file->data.resize(position + num_bytes);
  }
  memcpy(&file->data[position], buffer, num_bytes);
  position += num_bytes;
  PRINTF("Wrote %zd bytes successfully, setting position to %zd.\n", num_bytes, position);
  PRINTF("Wrote %zd bytes successfully, setting position to %zd.\n", num_bytes, position);
  return 0;
}

off_t FileHandle::Seek(off_t offset, int whence) {
  PRINTF("Seeking from %zd with offset %ld, size=%zd, whence=", position, static_cast<long>(offset), file->data.size());
  switch (whence) {
    case SEEK_SET:
      PRINTF("SEEK_SET");
      position = offset;
      break;
    case SEEK_CUR:
      PRINTF("SEEK_CUR");
      position += offset;
      break;
    case SEEK_END:
      PRINTF("SEEK_END");
      position = static_cast<off_t>(file->data.size()) - offset - 1;
      break;
    default:
      break;
  }
  PRINTF("\nNew position is %zd.\n", position);
  return position;
}

FileManager::FileManager()
    : last_fd_(2) { // start with empty spots for stdout, stdin, and stderr
  PRINTF("Constructing FileManager.\n");
}
FileManager::~FileManager() {}
void FileManager::FileFinished(const std::string& file_name_arg,
                               std::vector<uint8_t>& data) {
  std::string name = StripPath(file_name_arg);
  file_map_[name] = File(name, data);
  pending_files_.erase(name);
  if (pending_files_.empty()) {
    ready_func_();
  }
}

FileManager::ResponseHandler::ResponseHandler(const std::string& name_arg,
                                              FileManager* manager_arg)
      : name_(StripPath(name_arg)), manager_(manager_arg) {
  PRINTF("Creating ResponseHandler.\n");
}

void FileManager::ResponseHandler::FileFinished(std::vector<uint8_t>& data,
                                                int32_t error) {
  PRINTF("Finished downloading %s, %zd bytes, error: %"NACL_PRId32".\n", name_.c_str(), data.size(), error);
  manager_->FileFinished(StripPath(name_), data);
  // NOTE:  manager_ deletes us in FileFinished, so |this| is no longer
  //        valid here.
}

FileManager& FileManager::instance() {
  static FileManager file_manager_instance;
  return file_manager_instance;
}

void FileManager::Fetch(const std::string& file_name_arg,
                        size_t size) {
  PRINTF("Requesting %s\n", file_name_arg.c_str());
  std::string file_name = StripPath(file_name_arg);
  FileManager& self = instance();
  std::pair<PendingMap::iterator, bool> iter_success_pair;
  iter_success_pair = self.pending_files_.insert(
      PendingMap::value_type(file_name,
                             ResponseHandler(file_name, &self)));
  PendingMap::iterator iter = iter_success_pair.first;
  bool success = iter_success_pair.second;
  // If it was successful, then this is the first time the file has been
  // added, and we should start a URL request.
  if (success) {
    GetURLHandler* handler = GetURLHandler::Create(self.pp_instance_,
                                                   file_name,
                                                   size);
    if (!handler->Start(NewGetURLCallback(&iter->second,
                                          &ResponseHandler::FileFinished))) {
      // Private destructor;  can't follow the example.
      // delete handler;
    }
  }
}

bool FileManager::HasFD(int fd) {
  const FileManager& self(instance());
  FileHandleMap::const_iterator iter(self.file_handle_map_.find(fd));
  return (iter != self.file_handle_map_.end());
}

int FileManager::GetFD(const std::string& file_name_arg) {
  FileManager& self(instance());
  std::string file_name(StripPath(file_name_arg));
  FileMap::iterator iter = self.file_map_.find(file_name);
  if (iter != self.file_map_.end()) {
    int fd = ++self.last_fd_;
    self.file_handle_map_[fd] = FileHandle(&iter->second);
    return fd;
  }
  PRINTF("ERROR file not found, %s.\n", file_name_arg.c_str());
  errno = ENOENT;
  return -1;
}

void FileManager::AddEmptyFile(const std::string& file_name_arg) {
  FileManager& self(instance());
  std::string file_name(StripPath(file_name_arg));
  std::vector<uint8_t> empty_vec;
  if (self.file_map_.find(file_name) == self.file_map_.end()) {
    self.file_map_.insert(FileMap::value_type(file_name,
                                              File(file_name, empty_vec)));
  } else {
    PRINTF("AddEmptyFile:  File %s already exists.\n", file_name.c_str());
  }
}

void FileManager::Close(int fd) {
  instance().file_handle_map_.erase(fd);
}

void FileManager::Dump(const std::string& file_name_arg) {
  FileManager& self(instance());
  std::string file_name(StripPath(file_name_arg));
  FileMap::iterator iter = self.file_map_.find(file_name);
  if (iter != self.file_map_.end()) {
    std::vector<uint8_t>& data = iter->second.data;
    for (size_t i = 0u; i < data.size(); ++i) {
      if (!(i % 16u))
        std::printf("0%zo", i);
      if (!(i&1u))
        std::printf(" ");
      std::printf("%02"NACL_PRIx8, data[i]);
      if ((i%16u) == 15)
        std::printf("\n");
    }
    std::printf("\n");
    return;
  }
  std::printf("ERROR can't dump; file not found: %s.\n",
              file_name_arg.c_str());
}

FileHandle* FileManager::GetFileHandle(int fd) {
  PRINTF("Getting file for FD %d.\n", fd);
  FileManager& self(instance());
  FileHandleMap::iterator iter = self.file_handle_map_.find(fd);
  if (iter != self.file_handle_map_.end())
    return &iter->second;

  return NULL;
}

}  // namespace nacl_file

int __wrap_open(char const *pathname, int oflags, int perms) {
  PRINTF("nacl_open, file %s mode %d perms %d\n", pathname, oflags, perms);
  if (oflags & O_WRONLY) {
    // TODO: Make appending fail if append flag is not provided.
    PRINTF("Write mode.\n");
    std::printf("Opening %s for writing.\n", pathname);
    // AddEmptyFile will do nothing if the file already exists.
    nacl_file::FileManager::AddEmptyFile(StripPath(pathname));
  }
  int fd = nacl_file::FileManager::GetFD(StripPath(pathname));
  PRINTF("Returning %d\n", fd);
  return fd;
}

int __wrap_close(int fd) {
  PRINTF("nacl_close, fd %d\n", fd);
  nacl_file::FileManager::Close(fd);
  return 0;
}

int __wrap_read(int fd, void* buffer, size_t num_bytes) {
  PRINTF("nacl_read, fd %d, num_bytes %zd\n", fd, num_bytes);
  nacl_file::FileHandle* file = nacl_file::FileManager::GetFileHandle(fd);
  if (file) {
    return file->Read(buffer, num_bytes);
  }
  PRINTF("No file found for fd %d.\n", fd);
  errno = ENOENT;
  return -1;
}

int nacl_file_write(int fd, void* buffer, size_t num_bytes) {
  PRINTF("nacl_file_write, fd %d, num_bytes %zd\n", fd, num_bytes);
  PRINTF("nacl_file_write, fd %d, num_bytes %zd\n", fd, num_bytes);
  nacl_file::FileHandle* file = nacl_file::FileManager::GetFileHandle(fd);
  if (file) {
    return file->Write(buffer, num_bytes);
  }
  PRINTF("No file found for fd %d.\n", fd);
  errno = ENOENT;
  return -1;
}

int __wrap_write(int fd, void* buffer, size_t num_bytes) {
  if ((fd >= 0) && (fd <=2)) {
    return __real_write(fd, buffer, num_bytes);
  } else if (nacl_file::FileManager::HasFD(fd)) {
    return nacl_file_write(fd, buffer, num_bytes);
  } else {
    return __real_write(fd, buffer, num_bytes);
  }
}

off_t __wrap_lseek(int fd, off_t offset, int whence) {
  PRINTF("__wrap_seek, fd %d\n", fd);
  nacl_file::FileHandle* file = nacl_file::FileManager::GetFileHandle(fd);
  if (file) {
    return file->Seek(offset, whence);
  }
  errno = ENOENT;
  return -1;
}

int nacl_file_length(int fd) {
  PRINTF("__wrap_length, fd %d\n", fd);
  nacl_file::FileHandle* handle = nacl_file::FileManager::GetFileHandle(fd);
  if (handle) {
    int size = static_cast<int>(handle->file->data.size());
    PRINTF("__wrap_length is %d\n", size);
    return size;
  }
  errno = ENOENT;
  return -1;
}

