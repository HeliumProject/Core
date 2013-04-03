#include "PlatformPch.h"
#include "Exception.h"

#include "Platform/Types.h"
#include "Platform/Trace.h"
#include "Platform/Console.h"
#include "Platform/Process.h"

#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

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
      if (ptrace(PTRACE_ATTACH, ppid, NULL, NULL) == 0)
      {
          /* Wait for the parent to stop and continue it */
          waitpid(ppid, NULL, 0);
          ptrace(PTRACE_CONT, NULL, NULL);

          /* Detach */
          ptrace(PTRACE_DETACH, getppid(), NULL, NULL);

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
void Helium::DebugLog( const tchar_t* pMessage )
{
	Print( ConsoleColors::Red, stderr, pMessage );
}

#endif  // !HELIUM_RELEASE && !HELIUM_PROFILE
