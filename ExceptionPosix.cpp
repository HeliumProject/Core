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

#else

  // http://stackoverflow.com/questions/3596781/detect-if-gdb-is-running
  int pid = fork();
  int status;
  int res;

  if (pid == -1)
  {
    perror("fork");
    return -1;
  }

  if (pid == 0)
  {
    int ppid = getppid();

    /* Child */
    if (ptrace(PTRACE_ATTACH, ppid, NULL, 0) == 0)
    {
        /* Wait for the parent to stop and continue it */
      waitpid(ppid, NULL, 0);
      ptrace(PTRACE_CONT, ppid, NULL, 0);

        /* Detach */
      ptrace(PTRACE_DETACH, getppid(), NULL, 0);

        /* We were the tracers, so gdb is not present */
      res = 0;
    }
    else
    {
        /* Trace failed so gdb is present */
      res = 1;
    }
    exit(res);
  }
  else
  {
    waitpid(pid, &status, 0);
    res = WEXITSTATUS(status);
  }

  return res == 1;
#endif
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
