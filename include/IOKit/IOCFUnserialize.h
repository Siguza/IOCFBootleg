#ifndef _BOOTLEG_IOCFUNSERIALIZE
#define _BOOTLEG_IOCFUNSERIALIZE

#include <CoreFoundation/CoreFoundation.h>

extern CFTypeRef IOCFUnserialize(const char *buf, CFAllocatorRef allocator, CFOptionFlags options, CFStringRef *err);

#endif /* _BOOTLEG_IOCFUNSERIALIZE */
