#ifndef _BOOTLEG_SYSLOG
#define _BOOTLEG_SYSLOG

// Just need to silence these calls from IOKit code.

#define syslog(...) do {} while(0)

#define __unused

#include <string.h>

// Has nothing to do with syslog, really, but we might as well dump these here.

#ifndef __linux__

static inline void bcopy(const void *src, void *dst, size_t len)
{
    memcpy(dst, src, len);
}

static inline void bzero(void *s, size_t n)
{
    memset(s, 0, n);
}

#endif /* __linux__ */

#endif /* _BOOTLEG_SYSLOG */
