#ifndef _SYSTEMLINUX_H
#define _SYSTEMLINUX_H

/* windows needs this for calls to several libraries.  linux does not. */
#define __stdcall

#define _finite std::isfinite
#endif /* _SYSTEMLINUX_H */
