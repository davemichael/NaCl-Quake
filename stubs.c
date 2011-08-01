#include <sys/types.h>

// These are some do-nothing stubs to make the link succeed.
int unlink(const char* pathname) {
  return -1;
}

int kill (pid_t pid, int sig) {
  return -1;
}
