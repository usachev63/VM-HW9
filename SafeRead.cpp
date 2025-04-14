#include "SafeRead.h"
#include <cstdlib>
#include <setjmp.h>
#include <signal.h>

static struct sigaction oldSegvHandler, oldBusHandler;
static const void *volatile theSafeReadArgument;
static jmp_buf jmpBuf;

static void handler(int signal, siginfo_t *info, void *ucontext) {
  if (theSafeReadArgument == nullptr || info->si_addr != theSafeReadArgument) {
    switch (signal) {
    case SIGSEGV:
      oldSegvHandler.sa_sigaction(signal, info, ucontext);
      return;
    case SIGBUS:
      oldBusHandler.sa_sigaction(signal, info, ucontext);
      return;
    default:
      abort();
    }
  }

  siglongjmp(jmpBuf, 1);
}

static void setUpSignalHandlers() {
  struct sigaction a = {0};
  a.sa_sigaction = handler;
  a.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV, &a, &oldSegvHandler);
  sigaction(SIGBUS, &a, &oldBusHandler);
}

namespace {

class SignalHandlerSetupper {
public:
  SignalHandlerSetupper() { setUpSignalHandlers(); }
} setupper;

} // namespace

std::optional<uint8_t> safe_read_uint8(const uint8_t *p) {
  if (p == nullptr)
    return std::nullopt;
  theSafeReadArgument = p;
  int ret = sigsetjmp(jmpBuf, 1);
  if (ret == 0) {
    uint8_t val = *p;
    theSafeReadArgument = 0;
    return val;
  }
  theSafeReadArgument = 0;
  return std::nullopt;
}
