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

// A simple struct to represent a file in memory.  It has the name and a vector
// containing the data for the file.
struct File {
  File() {}
  File(const std::string& name_arg, std::vector<uint8_t>& data_arg);
  std::string name;
  std::vector<uint8_t> data;
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
  // TODO UNTESTED
  int Write(void* buffer, size_t num_bytes);
  off_t Seek(off_t offset, int whence);
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
    ResponseHandler(const std::string& name_arg, FileManager* manager_arg);
    void FileFinished(std::vector<uint8_t>& data, int32_t error);
   private:
    std::string name_;
    FileManager* manager_;
  };
  friend struct ResponseHandler;

  typedef std::map<std::string, ResponseHandler> PendingMap;
  typedef std::map<std::string, File> FileMap;
  FileMap file_map_;
  typedef std::map<int, FileHandle> FileHandleMap;
  FileHandleMap file_handle_map_;
  int last_fd_;
  PendingMap pending_files_;
  std::tr1::function<void()> ready_func_;
  std::tr1::function<void(int32_t)> progress_func_;
  pp::Instance* pp_instance_;

  static FileManager& instance();
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
    instance().ready_func_ = func;
  }
  static void set_progress_func(std::tr1::function<void(int32_t)> func) {
    instance().progress_func_ = func;
  }
  static void set_pp_instance(pp::Instance* pp_instance) {
    instance().pp_instance_ = pp_instance;
  }
  static pp::Instance* pp_instance() {
    return instance().pp_instance_;
  }
};

}  // namespace nacl_file

