#include <pthread.h>

#include <deque>
#include <map>
#include <set>
#include <string>
#include <tr1/functional>
#include <vector>

#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/file_system.h"
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


// Some synchronization helpers. TODO(dmichael): These really belong in
// separate files.
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
  T* get() { return pointee_; }
  const T* get() const { return pointee_; }
 private:
  LockedPtr& operator=(const LockedPtr&);  // Unimplemented, do not use.
  T* pointee_;
  ScopedLock scoped_lock_;
};

class ThreadSafeRefCount {
 public:
  ThreadSafeRefCount()
      : ref_(0) {
  }

  int32_t AddRef() {
    ScopedLock s(&lock_);
    return ++ref_;
  }

  int32_t Release() {
    ScopedLock s(&lock_);
    return --ref_;
  }

 private:
  int32_t ref_;
  Lock lock_;
};

class FileManager;

// A simple struct to represent a file in memory.  It has the name and a vector
// containing the data for the file.
struct File {
  ~File();
  File(const std::string& name_arg, FileManager* fm);
  std::string name;
  std::vector<uint8_t> data;
  Lock* lock;
  FileManager* file_manager;
  pp::FileRef* file_ref;
  pp::FileIO* file_io;
  std::deque<std::vector<uint8_t> > write_queue;
  void QueueWrite();
  void StartWriteImpl();
  bool exists;
 private:
  bool write_in_progress;
  void StartWrite();
  void WriteFinished();
  File(const File&);  // Unimplemented, do not use.
  File& operator=(const File&);  // Unimplemented, do not use.
};

// A handle to a file.  It's really just a pointer to a File and a position
// in that file.
struct FileHandle {
  FileHandle() : position(0), file(NULL), is_dirty(false) {}
  explicit FileHandle(File* file_arg) : file(file_arg), position(0),
                                        is_dirty(false) {
  }
  ~FileHandle();

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
  bool is_dirty;
};

// A singleton to track all files we fetch.
class FileManager {
 private:
  FileManager(pp::Instance* instance);
  ~FileManager();
  void FileSystemOpened(int32_t success);

  enum FETCH_TIME {
    FETCHED_AT_START, FETCHED_AFTER_START
  };
  // This is only called once the FileSystem is initialized. It starts a fetch
  // from the local file system using FileRef and FileIO. It has an unused
  // int32_t param first just so it can be used as a completion callback.
  void DoFetch(int32_t /* cc_result */, const std::string& name,
               FETCH_TIME fetch_time = FETCHED_AT_START);
  void FileOpened(int32_t result, File* file, FETCH_TIME fetch_time);
  void DirectoryCreated(int32_t result, std::string directory,
                        pp::FileRef* directory_ref);
  void FileQueried(int32_t result, File* file, PP_FileInfo* info);
  void FileReadFinished(File* file);
  void URLRequestFinished(File* file, std::vector<uint8_t>& data);

  class ResponseHandler {
   public:
    ResponseHandler(File* file);
    void FileFinishedDownload(std::vector<uint8_t>& data, int32_t error);
   private:
    File* file_;
  };
  friend struct ResponseHandler;

  typedef std::set<std::string> StringSet;
  // Files in this set are being fetched from local storage (or we're trying).
  // They are removed from the set in FileQueried, when we finally know if the
  // file existed locally.
  StringSet pending_fetch_set_;
  // Directories pending creation.
  StringSet pending_directories_set_;

  typedef std::map<std::string, ResponseHandler> PendingURLMap;

  // Note, we never delete from this map, so there's currently no place we
  // delete the raw pointer. If we ever add a file deletion operation, we'll
  // need to lock on the file and then delete it from the map.
  typedef std::map<std::string, File*> FileMap;
  // Files in this set are currently being downloaded via URLLoader or loaded
  // via FileIO.
  FileMap pending_files_;
  FileMap file_map_;
  typedef std::map<int, FileHandle> FileHandleMap;
  FileHandleMap file_handle_map_;
  int last_fd_;
  std::tr1::function<void()> ready_func_;
  std::tr1::function<void(int32_t)> read_progress_func_;
  std::tr1::function<void(int32_t)> write_progress_func_;
  pp::Instance* instance_;
  Lock lock_;
  pp::CompletionCallbackFactory<FileManager, ThreadSafeRefCount>
      callback_factory_;
  pp::FileSystem file_system_;
  bool file_system_opened_;

  static FileManager* file_manager_instance_;
  typedef LockedPtr<FileManager> FileManagerPtr;
  static FileManagerPtr instance();
 public:
  // Call this first, to init the FileManager.
  static void Init(pp::Instance* instance_arg, int64_t file_sys_size);
  // Fetch the file from the server asynchronously.  |size| is used to
  // preallocate the buffer (i.e., vector.reserve()).  For large files, this
  // can save time, but it is not required.
  static void Fetch(const std::string& file_name, size_t size = 1024u);
  // Open a file some time after startup; the file will be created if it does
  // not exist in the local file system.
  static void OpenNewFile(const std::string& file_name);
  static bool HasFD(int fd);
  static int GetFD(const std::string& file_name, bool create);
  static void Close(int fd);
  static void Dump(const std::string& file_name);
  static FileHandle* GetFileHandle(int fd);
  static void set_ready_func(std::tr1::function<void()> func) {
    instance()->ready_func_ = func;
  }
  static void set_read_progress_func(std::tr1::function<void(int32_t)> func) {
    instance()->read_progress_func_ = func;
  }
  static std::tr1::function<void(int32_t)> read_progress_func() {
    return instance()->read_progress_func_;
  }
  static void set_write_progress_func(std::tr1::function<void(int32_t)> func) {
    instance()->write_progress_func_ = func;
  }
  static std::tr1::function<void(int32_t)> write_progress_func() {
    return instance()->write_progress_func_;
  }
  static void set_pp_instance(pp::Instance* instance_arg) {
    instance()->instance_ = instance_arg;
  }
  static pp::Instance* pp_instance() {
    return instance()->instance_;
  }
};

}  // namespace nacl_file

