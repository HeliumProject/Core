#include "PlatformPch.h"
#include "Exception.h"

#include "Platform/Types.h"
#include "Platform/Trace.h"
#include "Platform/Console.h"
#include "Platform/Process.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#if HELIUM_OS_MAC
# define PTRACE_ATTACH PT_ATTACH
# define PTRACE_CONT PT_CONTINUE
# define PTRACE_DETACH PT_DETACH
#endif

bool Helium::IsDebuggerPresent()
{
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
}

#if !HELIUM_RELEASE && !HELIUM_PROFILE

/// Write a string to any platform-specific debug log output.
///
/// @param[in] pMessage  Message string to write to the debug log.
void Helium::DebugLog( const char* pMessage )
{
	Print( ConsoleColors::Red, stderr, pMessage );
}

#endif  // !HELIUM_RELEASE && !HELIUM_PROFILE
