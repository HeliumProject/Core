<a href="http://heliumproject.github.io/">![Helium Core](https://raw.github.com/HeliumProject/Core/master/Documentation/Helium.png)</a>

Platform is a very basic C++ wrapper of basic operating system components including:
* Asserts
* File system (i/o and directories)
* Color console printing
* Network sockets
* Atomic operations
* Synchronization primitives (mutex, condition)
* Memory allocators (optional)

## Design

Platform is designed to provide minimal functional wrappers, and not designed to go to a lot of effort to create convenience to the user.  This is designed to make Platform easier to port since it is the most fundamental component of the Helium stack.

Platform currently supports:
* Microsoft Windows
* Apple macOS
* GNU/Linux

However, Platform trys to consume POSIX APIs wherever possible (and reasonable), so porting to other POSIX platforms isn't a ground-up implementation.

## Implementation

Note that Platform is designed to utilize UTF-8 strings in memory, even on Windows machines.  Platform will convert to and from UTF-16 when calling Unicode Win32 APIs.
