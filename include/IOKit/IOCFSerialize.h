#ifndef _BOOTLEG_IOCFSERIALIZE
#define _BOOTLEG_IOCFSERIALIZE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CoreFoundation/CoreFoundation.h>

#define IOKIT_SERVER_VERSION 20140421

enum
{
    kIOCFSerializeToBinary = 0x00000001U,
};

CFDataRef IOCFSerialize(CFTypeRef object, CFOptionFlags options);
CFTypeRef IOCFUnserializeBinary(const char *buffer, size_t bufferSize, CFAllocatorRef allocator, CFOptionFlags options, CFStringRef *errorString);
CFTypeRef IOCFUnserializeWithSize(const char *buffer, size_t bufferSize, CFAllocatorRef allocator, CFOptionFlags options, CFStringRef *errorString);

#endif /* _BOOTLEG_IOCFSERIALIZE */
