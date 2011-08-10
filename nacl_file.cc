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

#include <string>
#include <tr1/functional>
#include <tr1/memory>

#include "file_handler.h"
#include "geturl_handler.h"
#include "nacl/nacl_inttypes.h"
#include "ppapi/c/pp_file_info.h"
#include "ppapi/c/ppb_file_io.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/cpp/module.h"

#define PRINTF(...)
//#define PRINTF(...) printf(__VA_ARGS__)

#define CHECK(arg) assert(arg)

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
  std::string GetParent(const std::string& file_name) {
    size_t last_slash = file_name.find_last_of('/');
    if (last_slash == std::string::npos)
      return std::string("");
    return file_name.substr(0, last_slash);
  }
  std::string AddSlash(const std::string& str) {
    if ((str.length() > 0) && (str[0] == '/'))
      return str;
    return std::string("/") + str;
  }

  enum FETCH_TIME {
    FETCHED_AT_START, FETCHED_AFTER_START
  };
}

namespace nacl_file {

Lock::Lock() {
  pthread_mutex_init(&mutex_, NULL);
}
Lock::~Lock() {
  pthread_mutex_destroy(&mutex_);
}
void Lock::Acquire() {
  pthread_mutex_lock(&mutex_);
}
void Lock::Release() {
  pthread_mutex_unlock(&mutex_);
}
bool Lock::Try() {
  int rv = pthread_mutex_trylock(&mutex_);
  return (rv == 0);
}

File::File(const std::string& name_arg, FileManager* fm)
    : name(StripPath(name_arg)), lock(new Lock()), file_ref(NULL),
      file_io(NULL), write_in_progress(false), file_manager(fm), exists(true) {
  PRINTF("Created 'File' object for %s.\n", name.c_str());
}

void File::QueueWrite() {
  ScopedLock scoped_lock(lock);
  // Copy the whole file on to our write queue.
  write_queue.push_back(data);
  StartWrite();
}

namespace {
void StartWriteThunk(void* file_ptr, int32_t /*result*/) {
  reinterpret_cast<File*>(file_ptr)->StartWriteImpl();
}
}

void File::StartWrite() {
  pp::Module::Get()->core()->CallOnMainThread(
    0,
    pp::CompletionCallback(StartWriteThunk, this));
}

void File::StartWriteImpl() {
  ScopedLock scoped_lock(lock);
  if (!write_in_progress && !write_queue.empty()) {
    write_in_progress = true;
    FileHandler::WriteToFile(
        file_io,
        reinterpret_cast<std::vector<char>*>(&write_queue.front()),
        file_manager->write_progress_func(),
        std::tr1::bind(&File::WriteFinished, this));
  }
}

void File::WriteFinished() {
  ScopedLock scoped_lock(lock);
  write_queue.pop_front();
  if (write_queue.empty())
    write_in_progress = false;
  else
    StartWrite();
}

File::~File() {
  delete lock;
}

int FileHandle::Length() {
  ScopedLock lock(file->lock);
  int size = static_cast<int>(file->data.size());
  PRINTF("Length is %d\n", size);
  return size;
}

int FileHandle::Read(void* buffer, size_t num_bytes) {
  ScopedLock lock(file->lock);
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

int FileHandle::Write(void* buffer, size_t num_bytes) {
  ScopedLock lock(file->lock);
  PRINTF("Attempting write of %s, position %zd, size %zd, %zd bytes.\n", file->name.c_str(), position, file->data.size(), num_bytes);
  if (position + num_bytes > file->data.size()) {
    file->data.resize(position + num_bytes);
  }
  memcpy(&file->data[position], buffer, num_bytes);
  position += num_bytes;
  is_dirty = true;
  PRINTF("Wrote %zd bytes successfully, setting position to %zd.\n", num_bytes, position);
  return num_bytes;
}

// We're being closed.
FileHandle::~FileHandle() {
  // If we made the file dirty, flush it. Note QueueWrite locks for us.
  if (is_dirty) {
    file->QueueWrite();
  }
}

off_t FileHandle::Seek(off_t offset, int whence) {
  ScopedLock lock(file->lock);
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
      position = static_cast<off_t>(file->data.size()) - offset;
      break;
    default:
      break;
  }
  PRINTF("\nNew position is %zd.\n", position);
  return position;
}

FileManager::FileManager(pp::Instance* instance)
    : last_fd_(2),  // start with empty spots for stdout, stdin, and stderr
      instance_(instance),
      callback_factory_(this),
      file_system_(instance, PP_FILESYSTEMTYPE_LOCALPERSISTENT),
      //file_system_(instance, PP_FILESYSTEMTYPE_LOCALTEMPORARY),
      file_system_opened_(false) {
  PRINTF("Constructing FileManager.\n");
}

FileManager::~FileManager() {}

void FileManager::FileReadFinished(File* file) {
  if (!file->data.size())
    file->exists = false;
  file->lock->Release();
  pending_files_.erase(file->name);
  if (pending_files_.empty() && pending_fetch_set_.empty() && ready_func_) {
    ready_func_();
  }
}
void FileManager::URLRequestFinished(File* file,
                                     std::vector<uint8_t>& data) {
  // Put the data in the file and unlock any waiting reads/writes/etc.
  file->data.swap(data);
  file->lock->Release();
  file->QueueWrite();
  pending_files_.erase(file->name);
  if (pending_files_.empty() && ready_func_) {
    ready_func_();
  }
}

FileManager::ResponseHandler::ResponseHandler(File* file)
    : file_(file) {
  PRINTF("Creating ResponseHandler.\n");
}

void FileManager::ResponseHandler::FileFinishedDownload(
    std::vector<uint8_t>& data, int32_t error) {
  PRINTF("Finished downloading %s, %zd bytes, error: %"NACL_PRId32".\n", file_->name.c_str(), data.size(), error);
  if (error)
    file_->exists = false;
  FileManager::instance()->URLRequestFinished(file_, data);
  delete this;
}

FileManager::FileManagerPtr FileManager::instance() {
  CHECK(file_manager_instance_);
  FileManagerPtr ptr(file_manager_instance_, &file_manager_instance_->lock_);
  return ptr;
}

FileManager* FileManager::file_manager_instance_ = NULL;
void FileManager::Init(pp::Instance* instance_arg, int64_t file_sys_size) {
  if (!file_manager_instance_) {
    file_manager_instance_ = new FileManager(instance_arg);
  }
  FileManagerPtr self(instance());
  self->file_system_.Open(file_sys_size,
      self->callback_factory_.NewCallback(
          &nacl_file::FileManager::FileSystemOpened));
}

void FileManager::Fetch(const std::string& file_name_arg,
                        size_t size) {
  PRINTF("Requesting %s\n", file_name_arg.c_str());
  std::string file_name = StripPath(file_name_arg);
  FileManagerPtr self(instance());
  std::pair<StringSet::iterator, bool> iter_success_pair;
  iter_success_pair = self->pending_fetch_set_.insert(file_name);
  bool success = iter_success_pair.second;
  // If it was successful, then this is the first time the file has been
  // added, and we should start a local file request.
  if (success) {
    // And add an empty, locked File to our map, so reads/writes will block
    // until the file is ready.
    std::pair<FileMap::iterator, bool> file_iter_success_pair;
    file_iter_success_pair = self->file_map_.insert(
        FileMap::value_type(file_name,
                            new File(file_name, self.get())));
    if (file_iter_success_pair.second) {
      file_iter_success_pair.first->second->lock->Acquire();
    }
    // If this Fetch is happening after the file system has already been opened,
    // we can go ahead and start fetching.
    if (self->file_system_opened_) {
      pp::Module::Get()->core()->CallOnMainThread(
          0,
          self->callback_factory_.NewCallback(
              &nacl_file::FileManager::DoFetch,
              file_name,
              FETCHED_AFTER_START));
    }
  }
}

void FileManager::DirectoryCreated(int32_t result, std::string name,
                                   pp::FileRef* directory_ref) {
  PRINTF("DirectoryCreated, result=%"NACL_PRId32", name=%s, resource=%"
         NACL_PRId32"\n",
         result, name.c_str(), directory_ref->pp_resource());
  pending_directories_set_.erase(name);
  if (pending_directories_set_.empty()) {
    // No files have been fetched yet, because the file system was not open
    // at the time they were requested.
    StringSet::iterator iter = pending_fetch_set_.begin();
    for (; iter != pending_fetch_set_.end(); ++iter) {
      DoFetch(0, *iter);
    }
  }
  delete directory_ref;
}

// Called when FileSystem has been opened.
void FileManager::FileSystemOpened(int32_t success) {
  PRINTF("File system opened, success=%"NACL_PRId32", file_system=%"NACL_PRId32
         "\n", success, file_system_.pp_resource());
  CHECK(success == PP_OK);
  file_system_opened_ = true;
  // Make the directories we will need. We just assume these will either work
  // if the directories don't exist yet, or will fail if they exist (but we
  // don't care; we just want them to exist). We don't do completion callbacks.
  StringSet::iterator iter(pending_fetch_set_.begin());
  std::set<std::string> directories;
  // Find all unique directory names.
  for (; iter != pending_fetch_set_.end(); ++iter) {
    directories.insert(GetParent(*iter));
  }
  // Make them, assuming either success or they already existed.
  for (iter = directories.begin(); iter != directories.end(); ++iter) {
    pp::FileRef* directory_ref = new pp::FileRef(file_system_,
                                                 AddSlash(*iter).c_str());
    PRINTF("Making directory %s, FileRef resource=%"NACL_PRId32"\n",
           iter->c_str(), directory_ref->pp_resource());
    pending_directories_set_.insert(*iter);
    directory_ref->MakeDirectoryIncludingAncestors(
        callback_factory_.NewCallback(
            &nacl_file::FileManager::DirectoryCreated, *iter, directory_ref));
  }
}

void FileManager::FileQueried(int32_t result, File* file, PP_FileInfo* info) {
  PRINTF("FileQueried: result=%"NACL_PRId32", file=%s, size=%"NACL_PRId64"\n",
         result, file->name.c_str(), info->size);
  CHECK(result == PP_OK);
  pending_fetch_set_.erase(file->name);
  // Now, finally, we can figure out if the file existed.
  int64_t size = info->size;
  delete info;
  std::pair<FileMap::iterator, bool> iter_success;
  iter_success = pending_files_.insert(FileMap::value_type(file->name, file));
  if (iter_success.second) {
    if (size > 0) {
      // File existed, so we'll read from local.
      file->data.resize(size);
      FileHandler::ReadFromFile(
          file->file_io,
          reinterpret_cast<std::vector<char>* >(&file->data),
          read_progress_func_,
          std::tr1::bind(&FileManager::FileReadFinished, this, file));
    } else {
      ResponseHandler* req_handler = new ResponseHandler(file);
      GetURLHandler* handler = GetURLHandler::Create(instance_, file->name);
      handler->set_progress_func(read_progress_func_);
      handler->Start(NewGetURLCallback(req_handler,
                                       &ResponseHandler::FileFinishedDownload));
    }
  }
}

void FileManager::FileOpened(int32_t result, File* file,
                             FETCH_TIME fetch_time) {
  PRINTF("FileOpened, result:%"NACL_PRId32", file:%s\n",
         result, file->name.c_str());
  CHECK(result == PP_OK);
  if (fetch_time == FETCHED_AT_START) {
    PP_FileInfo* file_info = new PP_FileInfo;
    file->file_io->Query(file_info,
                         callback_factory_.NewCallback(
                             &nacl_file::FileManager::FileQueried,
                             file,
                             file_info));
  } else {
    // If we're fetching after start, we don't bother with URL loading; we just
    // read all of the file, whatever it is.
    FileHandler::ReadFromFile(
        file->file_io,
        reinterpret_cast<std::vector<char>* >(&file->data),
        NULL,  // No progress required.
        std::tr1::bind(&Lock::Release, file->lock));  // Just unlock when done.
  }
}

void FileManager::DoFetch(int32_t /* cc_result */, const std::string& file_name,
                          FETCH_TIME fetch_time) {
  PRINTF("DoFetch: file_name=%s\n", file_name.c_str());
  // Start fetching the file from file_system_.
  pp::FileRef* ref = new pp::FileRef(file_system_, AddSlash(file_name).c_str());
  pp::FileIO* io = new pp::FileIO(instance_);
  // Assume if we got here that there is already a File in the map.
  File* file = file_map_[file_name];
  file->file_ref = ref;
  file->file_io = io;
  io->Open(
      *ref,
      PP_FILEOPENFLAG_WRITE | PP_FILEOPENFLAG_CREATE | PP_FILEOPENFLAG_READ,
      callback_factory_.NewCallback(&nacl_file::FileManager::FileOpened,
                                    file,
                                    fetch_time));
}

bool FileManager::HasFD(int fd) {
  FileManagerPtr self(instance());
  FileHandleMap::const_iterator iter(self->file_handle_map_.find(fd));
  return (iter != self->file_handle_map_.end());
}

int FileManager::GetFD(const std::string& file_name_arg, bool create) {
  FileManagerPtr self(instance());
  std::string file_name(StripPath(file_name_arg));
  FileMap::iterator iter = self->file_map_.find(file_name);
  if (iter != self->file_map_.end()) {
    if (create)
      iter->second->exists = true;
    if (iter->second->exists) {
      int fd = ++self->last_fd_;
      self->file_handle_map_[fd] = FileHandle(iter->second);
      return fd;
    }
  }
  PRINTF("ERROR file not found, %s.\n", file_name_arg.c_str());
  errno = ENOENT;
  return -1;
}


void FileManager::Close(int fd) {
  instance()->file_handle_map_.erase(fd);
}

void FileManager::Dump(const std::string& file_name_arg) {
  FileManagerPtr self(instance());
  std::string file_name(StripPath(file_name_arg));
  FileMap::iterator iter = self->file_map_.find(file_name);
  if (iter != self->file_map_.end()) {
    std::vector<uint8_t>& data = iter->second->data;
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
  FileManagerPtr self(instance());
  FileHandleMap::iterator iter = self->file_handle_map_.find(fd);
  if (iter != self->file_handle_map_.end())
    return &iter->second;

  return NULL;
}

}  // namespace nacl_file

int __wrap_open(char const *pathname, int oflags, int perms) {
  PRINTF("nacl_open, file %s mode %d perms %d\n", pathname, oflags, perms);
  bool create = (oflags & O_WRONLY);
  /*if (create) {
    PRINTF("Opening %s for writing.\n", pathname);
    // Fetch will do nothing if the file already exists in our map.
    nacl_file::FileManager::Fetch(StripPath(pathname));
  }*/
  int fd = nacl_file::FileManager::GetFD(StripPath(pathname), create);
  PRINTF("Returning %d\n", fd);
  return fd;
}

int __wrap_close(int fd) {
  PRINTF("nacl_close, fd %d\n", fd);
  nacl_file::FileHandle* file = nacl_file::FileManager::GetFileHandle(fd);
  if (file) {
    PRINTF("Closing %s\n", file->file->name.c_str());
  }
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
  PRINTF("nacl_file_length, fd %d\n", fd);
  nacl_file::FileHandle* file = nacl_file::FileManager::GetFileHandle(fd);
  if (file) {
    return file->Length();
  }
  errno = ENOENT;
  return -1;
}

