#include "PlatformPch.h"
#include "Exception.h"

#include "Platform/Types.h"
#include "Platform/Trace.h"
#include "Platform/Console.h"
#include "Platform/Process.h"

#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#if HELIUM_OS_MAC
# include <stdbool.h>
# include <sys/sysctl.h>
#endif

#if HELIUM_OS_LINUX
static void* test_trace(void* ignored)
{
  return (void*)ptrace(PTRACE_TRACEME, 0, NULL, NULL);
}
#endif

bool Helium::IsDebuggerPresent()
{
#if HELIUM_OS_MAC

  int                 junk;
  int                 mib[4];
  struct kinfo_proc   info;
  size_t              size;

  // Initialize the flags so that, if sysctl fails for some bizarre 
  // reason, we get a predictable result.

  info.kp_proc.p_flag = 0;

  // Initialize mib, which tells sysctl the info we want, in this case
  // we're looking for information about a specific process ID.

  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PID;
  mib[3] = getpid();

  // Call sysctl.

  size = sizeof(info);
  junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
  assert(junk == 0);

  // We're being debugged if the P_TRACED flag is set.

  return ( (info.kp_proc.p_flag & P_TRACED) != 0 );

#elif HELIUM_OS_LINUX

  pthread_attr_t attr;
  void* result;
  pthread_t thread;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  if (pthread_create(&thread, &attr, test_trace, NULL) != 0)
  {
    pthread_attr_destroy(&attr);
    return false;
  }
  pthread_attr_destroy(&attr);
  if (pthread_join(thread, &result) != 0)
  {
    return false;
  }

  return result != NULL;

#endif

  return false;
}

#if !HELIUM_RELEASE && !HELIUM_PROFILE

bool Helium::InitializeSymbols(const std::string& path)
{
  return true;
}

bool Helium::GetSymbolsInitialized()
{
  return true;
}

size_t GetStackTrace( void** ppStackTraceArray, size_t stackTraceArraySize, size_t skipCount = 1 )
{
  return 0;
}

void GetAddressSymbol( std::string& rSymbol, void* pAddress )
{
  rSymbol.clear();
}

/// Write a string to any platform-specific debug log output.
///
/// @param[in] pMessage  Message string to write to the debug log.
void Helium::DebugLog( const char* pMessage )
{
  if ( IsDebuggerPresent() )
  {
    fprintf( stderr, "%s", pMessage );
  }
}

#endif  // !HELIUM_RELEASE && !HELIUM_PROFILE
