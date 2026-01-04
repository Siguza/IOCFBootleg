#ifndef _BOOTLEG_COREFOUNDATION
#define _BOOTLEG_COREFOUNDATION

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool Boolean;
typedef uint8_t UInt8;
typedef int8_t SInt8;
typedef int16_t SInt16;
typedef int32_t SInt32;
typedef int64_t SInt64;
typedef uint32_t CFOptionFlags;
typedef size_t CFIndex;
typedef CFIndex CFRange[2];
typedef double CFAbsoluteTime;

#ifndef FALSE
#   define FALSE false
#endif
#ifndef TRUE
#   define TRUE true
#endif

#if _LP64__
#   define CGFLOAT_TYPE double
#else
#   define CGFLOAT_TYPE float
#endif

#define kCFAbsoluteTimeIntervalSince1970 978307200.0l

typedef enum
{
    kCFTypeNull,
    kCFTypeBoolean,
    kCFTypeString,
    kCFTypeData,
    kCFTypeNumber,
    kCFTypeDate,
    kCFTypeArray,
    kCFTypeSet,
    kCFTypeDictionary,
} CFTypeID;

typedef enum
{
    kCFStringEncodingUTF8,
    kCFStringEncodingMacRoman,
} CFStringEncoding;

typedef enum
{
    kCFNumberSInt8Type,
    kCFNumberSInt16Type,
    kCFNumberSInt32Type,
    kCFNumberSInt64Type,
    kCFNumberCharType,
    kCFNumberShortType,
    kCFNumberIntType,
    kCFNumberLongType,
    kCFNumberLongLongType,
    kCFNumberFloat32Type,
    kCFNumberFloat64Type,
    kCFNumberCGFloatType,
    kCFNumberFloatType,
    kCFNumberDoubleType,
} CFNumberType;

#define CFNullGetTypeID()       kCFTypeNull
#define CFBooleanGetTypeID()    kCFTypeBoolean
#define CFStringGetTypeID()     kCFTypeString
#define CFDataGetTypeID()       kCFTypeData
#define CFNumberGetTypeID()     kCFTypeNumber
#define CFDateGetTypeID()       kCFTypeDate
#define CFArrayGetTypeID()      kCFTypeArray
#define CFSetGetTypeID()        kCFTypeSet
#define CFDictionaryGetTypeID() kCFTypeDictionary

struct CFCallbacks
{
    const void* (*retain)(const void*);
    void (*release)(const void*);
    Boolean (*equal)(const void*, const void*);
};

typedef const void* CFTypeRef;
typedef const void* CFBooleanRef;
typedef const void* CFStringRef;
typedef const void* CFDataRef;
typedef const void* CFNumberRef;
typedef const void* CFDateRef;
typedef const void* CFArrayRef;
typedef const void* CFSetRef;
typedef const void* CFDictionaryRef;
typedef void* CFMutableDataRef;
typedef void* CFMutableArrayRef;
typedef void* CFMutableSetRef;
typedef void* CFMutableDictionaryRef;
typedef void* CFAllocatorRef;
typedef struct CFCallbacks CFArrayCallBacks;
typedef struct CFCallbacks CFSetCallBacks;
typedef struct CFCallbacks CFDictionaryKeyCallBacks;
typedef struct CFCallbacks CFDictionaryValueCallBacks;

extern const CFBooleanRef kCFBooleanTrue;
extern const CFBooleanRef kCFBooleanFalse;

extern const struct CFCallbacks kCFTypeArrayCallBacks;
extern const struct CFCallbacks kCFTypeSetCallBacks;
extern const struct CFCallbacks kCFTypeDictionaryKeyCallBacks;
extern const struct CFCallbacks kCFTypeDictionaryValueCallBacks;

struct CFBase
{
    CFTypeID type;
    _Atomic uint32_t refcnt;
};

struct CFBoolean
{
    CFTypeID type;
    _Atomic uint32_t refcnt;
    bool value;
};

struct CFString
{
    CFTypeID type;
    _Atomic uint32_t refcnt;
    const char *str;
};

struct CFData
{
    CFTypeID type;
    _Atomic uint32_t refcnt;
    void *bytes;
    CFIndex length;
    CFIndex capacity;
};

struct CFNumber
{
    CFTypeID type;
    _Atomic uint32_t refcnt;
    CFNumberType numType;
    union
    {
        unsigned long long l;
        double d;
    } value;
};

struct CFDate
{
    CFTypeID type;
    _Atomic uint32_t refcnt;
    double time;
};

struct CFArray
{
    CFTypeID type;
    _Atomic uint32_t refcnt;
    const void **elements;
    CFIndex length;
    CFIndex capacity;
    const struct CFCallbacks *callbacks;
};

struct CFDictionary
{
    CFTypeID type;
    _Atomic uint32_t refcnt;
    struct
    {
        const void *key;
        const void *value;
    } *elements;
    CFIndex length;
    CFIndex capacity;
    const struct CFCallbacks *keyCallbacks;
    const struct CFCallbacks *valueCallbacks;
};

#define CFSTR(s) \
({ \
    static struct CFString ss = { kCFTypeString, 0xffffffff, s }; \
    &ss; \
})

#define CFRangeMake(min, max) ((CFRange){ min, max })

#define kCFAllocatorNull NULL
#define kCFAllocatorDefault NULL

typedef void (*CFArrayApplierFunction)(const void *value, void *context);
typedef void (*CFSetApplierFunction)(const void *value, void *context);
typedef void (*CFDictionaryApplierFunction)(const void *key, const void *value, void *context);

CFTypeID CFGetTypeID(CFTypeRef cf);
CFTypeRef CFRetain(CFTypeRef cf);
void CFRelease(CFTypeRef cf);

Boolean CFEqual(CFTypeRef cf1, CFTypeRef cf2);

Boolean CFBooleanGetValue(CFBooleanRef boolean);

CFStringRef CFStringCreateWithCString(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding);
CFStringRef CFStringCreateWithCStringNoCopy(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator);
CFStringRef CFStringCreateWithFormat(CFAllocatorRef alloc, CFDictionaryRef formatOptions, CFStringRef format, ...);
CFStringRef CFStringCreateWithBytes(CFAllocatorRef alloc, const UInt8 *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean isExternalRepresentation);
CFIndex CFStringGetLength(CFStringRef theString);
const char* CFStringGetCStringPtr(CFStringRef theString, CFStringEncoding encoding);
CFIndex CFStringGetBytes(CFStringRef theString, CFRange range, CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation, UInt8 *buffer, CFIndex maxBufLen, CFIndex *usedBufLen);
CFDataRef CFStringCreateExternalRepresentation(CFAllocatorRef alloc, CFStringRef theString, CFStringEncoding encoding, UInt8 lossByte);

CFDataRef CFDataCreate(CFAllocatorRef allocator, const UInt8 *bytes, CFIndex length);
CFMutableDataRef CFDataCreateMutable(CFAllocatorRef allocator, CFIndex capacity);
void CFDataAppendBytes(CFMutableDataRef theData, const UInt8 *bytes, CFIndex length);
void CFDataIncreaseLength(CFMutableDataRef theData, CFIndex extraLength);
CFIndex CFDataGetLength(CFDataRef theData);
const UInt8* CFDataGetBytePtr(CFDataRef theData);

CFNumberRef CFNumberCreate(CFAllocatorRef allocator, CFNumberType theType, const void *valuePtr);
CFNumberType CFNumberGetType(CFNumberRef number);
Boolean CFNumberIsFloatType(CFNumberRef number);
Boolean CFNumberGetValue(CFNumberRef number, CFNumberType theType, void *valuePtr);

CFDateRef CFDateCreate(CFAllocatorRef allocator, CFAbsoluteTime at);
CFAbsoluteTime CFDateGetAbsoluteTime(CFDateRef theDate);

CFMutableArrayRef CFArrayCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFArrayCallBacks *callBacks);
void CFArrayAppendValue(CFMutableArrayRef theArray, const void *value);
CFIndex CFArrayGetCount(CFArrayRef theArray);
const void* CFArrayGetValueAtIndex(CFArrayRef theArray, CFIndex idx);
void CFArrayApplyFunction(CFArrayRef theArray, CFRange range, CFArrayApplierFunction applier, void *context);

CFMutableSetRef CFSetCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFSetCallBacks *callBacks);
void CFSetAddValue(CFMutableSetRef theSet, const void *value);
CFIndex CFSetGetCount(CFSetRef theSet);
void CFSetGetValues(CFSetRef theSet, const void **values);
void CFSetApplyFunction(CFSetRef theSet, CFSetApplierFunction applier, void *context);

CFMutableDictionaryRef CFDictionaryCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks);
void CFDictionarySetValue(CFMutableDictionaryRef theDict, const void *key, const void *value);
void CFDictionaryAddValue(CFMutableDictionaryRef theDict, const void *key, const void *value);
CFIndex CFDictionaryGetCount(CFDictionaryRef theDict);
const void* CFDictionaryGetValue(CFDictionaryRef theDict, const void *key);
void CFDictionaryGetKeysAndValues(CFDictionaryRef theDict, const void **keys, const void **values);
void CFDictionaryApplyFunction(CFDictionaryRef theDict, CFDictionaryApplierFunction applier, void *context);

#endif /* _BOOTLEG_COREFOUNDATION */
