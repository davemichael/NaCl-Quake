#include <pthread.h>

#include <map>
#include <string>
#include <tr1/functional>
#include <vector>

#include "ppapi/cpp/instance.h"

// These are some classes to aid in supporting file opening and reading using
// in-memory buffers to represent files.  FileManager is a singleton that
// manages all the in-memory files.  It is accessed through its public static
// functions.  The idea is in your Instance's Init function, you set up the
// FileManager with your instance and a function to invoke after all files have
// finished downloading.  Then you tell it to 'Fetch' all the files you need.
// It will asynchronously retrieve all the files, and when they have all
// downloaded, it invokes your 'ready' function.
//
// E.g.:
//
//  FileManager::set_pp_instance(this);
//  FileManager::set_ready_func(std::tr1::bind(&MyInstance::FilesFinished,
//                                             this));
//  FileManager::Fetch("image.jpg");
//  FileManager::Fetch("levels/level1.lvl");
//  FileManager::Fetch("levels/level2.lvl");
//  //<etc>
namespace nacl_file {

class Lock {
 public:
  Lock();
  ~Lock();
  void Acquire();
  void Release();
  bool Try();
 private:
  Lock(const Lock&);  // Unimplemented, do not use.
  Lock& operator=(const Lock&);  // Unimplemented, do not use.
  pthread_mutex_t mutex_;
};
class ScopedLock {
 public:
  explicit ScopedLock(Lock* lock)
      : lock_(lock) {
    lock_->Acquire();
  }
  ~ScopedLock() {
    lock_->Release();
  }
  // Take ownership of the lock. This is super sneaky and not usually a good
  // idea; kinda like auto_ptr. But I'm going to do it anyway.
  ScopedLock(const ScopedLock& from)
      : lock_(from.lock_) {
    const_cast<ScopedLock&>(from).lock_ = NULL;
  }
 private:
  ScopedLock& operator=(const ScopedLock&); // Not implemented, do not use.
  Lock* lock_;
};
// A pointer to a T that locks on creation and unlocks on destruction.
template <class T>
class LockedPtr {
 public:
  LockedPtr(T* t_ptr, Lock* lock)
      : pointee_(t_ptr), scoped_lock_(lock) {}
  const T& operator*() const {
    return *pointee_;
  } T& operator*() {
    return *pointee_;
  } const T* operator->() const {
    return pointee_;
  } T* operator->() {
    return pointee_;
  }
  LockedPtr(const LockedPtr& from)
      : pointee_(from.pointee_), scoped_lock_(from.scoped_lock_) {
  }
 private:
  LockedPtr& operator=(const LockedPtr&);  // Unimplemented, do not use.
  T* pointee_;
  ScopedLock scoped_lock_;
};

// A simple struct to represent a file in memory.  It has the name and a vector
// containing the data for the file.
struct File {
  ~File();
  explicit File(const std::string& name_arg);
  std::string name;
  std::vector<uint8_t> data;
  Lock* lock;
 private:
  File(const File&);  // Unimplemented, do not use.
  File& operator=(const File&);  // Unimplemented, do not use.
};

// A handle to a file.  It's really just a pointer to a File and a position
// in that file.
struct FileHandle {
  FileHandle() : position(0), file(NULL) {}
  explicit FileHandle(File* file_arg) : file(file_arg), position(0) {}

  // Read num_bytes from the current position and place it in to buffer.  If
  // there aren't num_bytes bytes left, read what bytes there are.  Returns the
  // number of bytes read, 0 on EOF.
  int Read(void* buffer, size_t num_bytes);

  // Write num_bytes from buffer to the current position in our buffer.  The
  // buffer is grown, if necessary, to accomodate the written bytes.  Returns
  // 0 on success (no failure conditions yet).
  int Write(void* buffer, size_t num_bytes);
  off_t Seek(off_t offset, int whence);
  int Length();

  File* file;
  size_t position;
  int mode;
};

// A singleton to track all files we fetch.
class FileManager {
  FileManager();
  ~FileManager();
  void FileFinished(const std::string& name, std::vector<uint8_t>& data);
  void Progress(int32_t bytes);
  class ResponseHandler {
   public:
    ResponseHandler(const std::string& name_arg);
    void FileFinished(std::vector<uint8_t>& data, int32_t error);
   private:
    std::string name_;
    FileManager* manager_;
  };
  friend struct ResponseHandler;

  typedef std::map<std::string, ResponseHandler> PendingMap;
  // Note, we never delete from this map, so there's currently no place we
  // delete the raw pointer. If we ever add a file deletion operation, we'll
  // need to lock on the file and then delete it from the map.
  typedef std::map<std::string, File*> FileMap;
  FileMap file_map_;
  typedef std::map<int, FileHandle> FileHandleMap;
  FileHandleMap file_handle_map_;
  int last_fd_;
  PendingMap pending_files_;
  std::tr1::function<void()> ready_func_;
  std::tr1::function<void(int32_t)> progress_func_;
  pp::Instance* pp_instance_;
  Lock lock_;

  typedef LockedPtr<FileManager> FileManagerPtr;
  static FileManagerPtr instance();
 public:
  // Fetch the file from the server asynchronously.  |size| is used to
  // preallocate the buffer (i.e., vector.reserve()).  For large files, this
  // can save time, but it is not required.
  static void Fetch(const std::string& file_name, size_t size = 1024u);
  static void AddEmptyFile(const std::string& file_name);
  static bool HasFD(int fd);
  static int GetFD(const std::string& file_name);
  static void Close(int fd);
  static void Dump(const std::string& file_name);
  static FileHandle* GetFileHandle(int fd);
  static void set_ready_func(std::tr1::function<void()> func) {
    instance()->ready_func_ = func;
  }
  static void set_progress_func(std::tr1::function<void(int32_t)> func) {
    instance()->progress_func_ = func;
  }
  static void set_pp_instance(pp::Instance* pp_instance) {
    instance()->pp_instance_ = pp_instance;
  }
  static pp::Instance* pp_instance() {
    return instance()->pp_instance_;
  }
};

}  // namespace nacl_file

