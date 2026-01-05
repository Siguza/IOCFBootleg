#ifndef _GNU_SOURCE
#   define _GNU_SOURCE 1
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CoreFoundation/CoreFoundation.h>

const CFBooleanRef kCFBooleanTrue  = &(const struct CFBoolean){ kCFTypeBoolean, 0xffffffff, true };
const CFBooleanRef kCFBooleanFalse = &(const struct CFBoolean){ kCFTypeBoolean, 0xffffffff, false };

const struct CFCallbacks kCFTypeArrayCallBacks =
{
    .retain  = CFRetain,
    .release = CFRelease,
    .equal   = CFEqual,
};
const struct CFCallbacks kCFTypeSetCallBacks =
{
    .retain  = CFRetain,
    .release = CFRelease,
    .equal   = CFEqual,
};
const struct CFCallbacks kCFTypeDictionaryKeyCallBacks =
{
    .retain  = CFRetain,
    .release = CFRelease,
    .equal   = CFEqual,
};
const struct CFCallbacks kCFTypeDictionaryValueCallBacks =
{
    .retain  = CFRetain,
    .release = CFRelease,
    .equal   = CFEqual,
};

CFTypeID CFGetTypeID(CFTypeRef cf)
{
    return ((const struct CFBase*)cf)->type;
}

CFTypeRef CFRetain(CFTypeRef cf)
{
    struct CFBase *base = (struct CFBase*)cf;
    uint32_t oldval = base->refcnt;
    do
    {
        if(oldval == 0xffffffff)
        {
            return cf;
        }
    }
    while(!__c11_atomic_compare_exchange_strong(&base->refcnt, &oldval, oldval + 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
    return cf;
}

void CFRelease(CFTypeRef cf)
{
    struct CFBase *base = (struct CFBase*)cf;
    uint32_t oldval = base->refcnt;
    do
    {
        if(oldval == 0xffffffff)
        {
            return;
        }
    }
    while(!__c11_atomic_compare_exchange_strong(&base->refcnt, &oldval, oldval - 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
    if(oldval == 0)
    {
        switch(base->type)
        {
            case kCFTypeData:
            {
                struct CFData *data = (struct CFData*)base;
                if(data->bytes)
                {
                    free(data->bytes);
                }
                break;
            }
            case kCFTypeArray:
            case kCFTypeSet:
            {
                struct CFArray *arr = (struct CFArray*)base;
                if(arr->callbacks && arr->callbacks->release)
                {
                    void (*release)(const void*) = arr->callbacks->release;
                    for(CFIndex i = 0; i < arr->length; ++i)
                    {
                        release(arr->elements[i]);
                    }
                }
                free(arr->elements);
                break;
            }
            case kCFTypeDictionary:
            {
                struct CFDictionary *dict = (struct CFDictionary*)base;
                if(dict->keyCallbacks && dict->keyCallbacks->release)
                {
                    void (*release)(const void*) = dict->keyCallbacks->release;
                    for(CFIndex i = 0; i < dict->length; ++i)
                    {
                        release(dict->elements[i].key);
                    }
                }
                if(dict->valueCallbacks && dict->valueCallbacks->release)
                {
                    void (*release)(const void*) = dict->valueCallbacks->release;
                    for(CFIndex i = 0; i < dict->length; ++i)
                    {
                        release(dict->elements[i].value);
                    }
                }
                free(dict->elements);
                break;
            }
            default:
                break;
        }
        free(base);
    }
}

static Boolean NullEqual(CFTypeRef cf1, CFTypeRef cf2)
{
    return cf1 == cf2;
}

Boolean CFEqual(CFTypeRef cf1, CFTypeRef cf2)
{
    if(cf1 == cf2)
        return true;

    CFTypeID type = CFGetTypeID(cf1);
    if(type != CFGetTypeID(cf2))
        return false;

    switch(type)
    {
        case kCFTypeString:
            return strcmp(((const struct CFString*)cf1)->str, ((const struct CFString*)cf2)->str) == 0;
        case kCFTypeData:
        {
            const struct CFData *dt1 = cf1,
                                *dt2 = cf2;
            return dt1->length == dt2->length && memcmp(dt1->bytes, dt2->bytes, dt1->length) == 0;
        }
        case kCFTypeNumber:
        {
            const struct CFNumber *num1 = cf1,
                                  *num2 = cf2;
            if(num1->numType != num2->numType)
                return false;
            switch(num1->numType)
            {
                case kCFNumberLongLongType:
                    return num1->value.l == num2->value.l;
                case kCFNumberDoubleType:
                    return num1->value.d == num2->value.d;
                default:
                    return false;
            }
        }
        default:
            return false;
    }
}

Boolean CFBooleanGetValue(CFBooleanRef boolean)
{
    return ((const struct CFBoolean*)boolean)->value;
}

CFStringRef CFStringCreateWithCString(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding)
{
    size_t len = strlen(cStr);
    return CFStringCreateWithBytes(alloc, (const UInt8*)cStr, len, encoding, false);
}

CFStringRef CFStringCreateWithCStringNoCopy(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding, CFAllocatorRef contentsDeallocator)
{
    if(encoding != kCFStringEncodingUTF8)
        abort();
    if(contentsDeallocator)
        abort();

    struct CFString *str = malloc(sizeof(struct CFString));
    if(str)
    {
        str->type = kCFTypeString;
        str->refcnt = 1;
        str->str = cStr;
    }
    return str;
}

CFStringRef CFStringCreateWithFormat(CFAllocatorRef alloc, CFDictionaryRef formatOptions, CFStringRef format, ...)
{
    if(formatOptions != NULL)
        abort();

    va_list ap;
    va_start(ap, format);
    char *buf = NULL;
    int r = vasprintf(&buf, ((const struct CFString*)format)->str, ap);
    va_end(ap);

    if(r < 0)
        return NULL;

    CFStringRef str = CFStringCreateWithCString(alloc, buf, kCFStringEncodingUTF8);
    free(buf);
    return str;
}

CFStringRef CFStringCreateWithBytes(CFAllocatorRef alloc, const UInt8 *bytes, CFIndex numBytes, CFStringEncoding encoding, Boolean isExternalRepresentation)
{
    if(encoding != kCFStringEncodingUTF8)
        abort();
    if(isExternalRepresentation)
        abort();

    struct CFString *str = malloc(sizeof(struct CFString) + numBytes + 1);
    if(str)
    {
        char *buf = (char*)(str + 1);
        memcpy(buf, bytes, numBytes);
        buf[numBytes] = '\0';
        str->type = kCFTypeString;
        str->refcnt = 1;
        str->str = buf;
    }
    return str;
}

CFIndex CFStringGetLength(CFStringRef theString)
{
    return strlen(((const struct CFString*)theString)->str);
}

const char* CFStringGetCStringPtr(CFStringRef theString, CFStringEncoding encoding)
{
    if(encoding != kCFStringEncodingUTF8)
        return NULL;

    return ((const struct CFString*)theString)->str;
}

CFIndex CFStringGetBytes(CFStringRef theString, CFRange range, CFStringEncoding encoding, UInt8 lossByte, Boolean isExternalRepresentation, UInt8 *buffer, CFIndex maxBufLen, CFIndex *usedBufLen)
{
    if(encoding != kCFStringEncodingUTF8)
        abort();

    if(range[1] <= range[0])
    {
        if(usedBufLen)
            *usedBufLen = 0;
        return 0;
    }
    CFIndex num = range[1] - range[0];
    if(num > maxBufLen)
        num = maxBufLen;
    if(buffer)
        memcpy(buffer, ((const struct CFString*)theString)->str, num);
    if(usedBufLen) *usedBufLen = num;
    return num;
}

CFDataRef CFStringCreateExternalRepresentation(CFAllocatorRef alloc, CFStringRef theString, CFStringEncoding encoding, UInt8 lossByte)
{
    if(encoding != kCFStringEncodingUTF8)
        abort();

    return CFDataCreate(NULL, (const UInt8*)((const struct CFString*)theString)->str, CFStringGetLength(theString));
}

CFDataRef CFDataCreate(CFAllocatorRef allocator, const UInt8 *bytes, CFIndex length)
{
    struct CFData *data = malloc(sizeof(struct CFData));
    if(data)
    {
        void *buf = malloc(length);
        if(buf)
        {
            memcpy(buf, bytes, length);
            data->type = kCFTypeData;
            data->refcnt = 1;
            data->bytes = buf;
            data->length = length;
            data->capacity = length;
        }
        else
        {
            free(data);
            data = NULL;
        }
    }
    return data;
}

CFMutableDataRef CFDataCreateMutable(CFAllocatorRef allocator, CFIndex capacity)
{
    struct CFData *data = malloc(sizeof(struct CFData));
    if(data)
    {
        data->type = kCFTypeData;
        data->refcnt = 1;
        data->bytes = NULL;
        data->length = 0;
        data->capacity = capacity;

        if(capacity)
        {
            data->bytes = malloc(capacity);
            if(!data->bytes)
            {
                free(data);
                data = NULL;
            }
        }
    }
    return data;
}

void CFDataAppendBytes(CFMutableDataRef theData, const UInt8 *bytes, CFIndex length)
{
    // TODO: thread safety
    if(!length)
        return;

    struct CFData *data = theData;
    if(data->capacity > 0)
    {
        if(length > (data->capacity - data->length))
            abort();
    }
    else
    {
        void *buf = realloc(data->bytes, data->length + length);
        if(!buf)
            abort();
        data->bytes = buf;
    }
    if(bytes)
        memcpy((void*)((uintptr_t)data->bytes + data->length), bytes, length);
    else
        memset((void*)((uintptr_t)data->bytes + data->length), 0, length);
    data->length += length;
}

void CFDataIncreaseLength(CFMutableDataRef theData, CFIndex extraLength)
{
    CFDataAppendBytes(theData, NULL, extraLength);
}

CFIndex CFDataGetLength(CFDataRef theData)
{
    return ((struct CFData*)theData)->length;
}

const UInt8* CFDataGetBytePtr(CFDataRef theData)
{
    return ((struct CFData*)theData)->bytes;
}

CFNumberRef CFNumberCreate(CFAllocatorRef allocator, CFNumberType theType, const void *valuePtr)
{
    struct CFNumber *num = malloc(sizeof(struct CFNumber));
    if(num)
    {
        num->type = kCFTypeNumber;
        num->refcnt = 1;
        switch(theType)
        {
#define MAPTYPE(enumtype, ctype, internaltype, field)       \
            case enumtype:                                  \
                num->value.field = *(const ctype*)valuePtr; \
                num->numType = internaltype;                \
                break;

            MAPTYPE(kCFNumberSInt8Type,     int8_t,         kCFNumberLongLongType, l)
            MAPTYPE(kCFNumberSInt16Type,    int16_t,        kCFNumberLongLongType, l)
            MAPTYPE(kCFNumberSInt32Type,    int32_t,        kCFNumberLongLongType, l)
            MAPTYPE(kCFNumberSInt64Type,    int64_t,        kCFNumberLongLongType, l)
            MAPTYPE(kCFNumberCharType,      char,           kCFNumberLongLongType, l)
            MAPTYPE(kCFNumberIntType,       int,            kCFNumberLongLongType, l)
            MAPTYPE(kCFNumberLongType,      long,           kCFNumberLongLongType, l)
            MAPTYPE(kCFNumberLongLongType,  long long,      kCFNumberLongLongType, l)
            MAPTYPE(kCFNumberCGFloatType,   CGFLOAT_TYPE,   kCFNumberDoubleType,   d)
            MAPTYPE(kCFNumberFloat32Type,   float,          kCFNumberDoubleType,   d)
            MAPTYPE(kCFNumberFloat64Type,   double,         kCFNumberDoubleType,   d)
            MAPTYPE(kCFNumberFloatType,     float,          kCFNumberDoubleType,   d)
            MAPTYPE(kCFNumberDoubleType,    double,         kCFNumberDoubleType,   d)

#undef MAPTYPE

            default:
                abort();
                break;
        }
    }
    return num;
}

CFNumberType CFNumberGetType(CFNumberRef number)
{
    return ((const struct CFNumber*)number)->numType;
}

Boolean CFNumberIsFloatType(CFNumberRef number)
{
    return ((const struct CFNumber*)number)->numType == kCFNumberDoubleType;
}

Boolean CFNumberGetValue(CFNumberRef number, CFNumberType theType, void *valuePtr)
{
    const struct CFNumber *num = number;
    switch(theType)
    {
#define MAPTYPE(enumtype, ctype, internaltype, field)       \
        case enumtype:                                      \
            if(num->numType != internaltype)                \
                return false;                               \
            *(ctype*)valuePtr = (ctype)num->value.field;    \
            return true;

        MAPTYPE(kCFNumberSInt8Type,     int8_t,         kCFNumberLongLongType, l)
        MAPTYPE(kCFNumberSInt16Type,    int16_t,        kCFNumberLongLongType, l)
        MAPTYPE(kCFNumberSInt32Type,    int32_t,        kCFNumberLongLongType, l)
        MAPTYPE(kCFNumberSInt64Type,    int64_t,        kCFNumberLongLongType, l)
        MAPTYPE(kCFNumberCharType,      char,           kCFNumberLongLongType, l)
        MAPTYPE(kCFNumberIntType,       int,            kCFNumberLongLongType, l)
        MAPTYPE(kCFNumberLongType,      long,           kCFNumberLongLongType, l)
        MAPTYPE(kCFNumberLongLongType,  long long,      kCFNumberLongLongType, l)
        MAPTYPE(kCFNumberCGFloatType,   CGFLOAT_TYPE,   kCFNumberDoubleType,   d)
        MAPTYPE(kCFNumberFloat32Type,   float,          kCFNumberDoubleType,   d)
        MAPTYPE(kCFNumberFloat64Type,   double,         kCFNumberDoubleType,   d)
        MAPTYPE(kCFNumberFloatType,     float,          kCFNumberDoubleType,   d)
        MAPTYPE(kCFNumberDoubleType,    double,         kCFNumberDoubleType,   d)

#undef MAPTYPE

        default:
            abort();
            return false;
    }
}

CFDateRef CFDateCreate(CFAllocatorRef allocator, CFAbsoluteTime at)
{
    struct CFDate *date = malloc(sizeof(struct CFDate));
    if(date)
    {
        date->type = kCFTypeDate;
        date->refcnt = 1;
        date->time = at;
    }
    return date;
}

CFAbsoluteTime CFDateGetAbsoluteTime(CFDateRef theDate)
{
    return ((struct CFDate*)theDate)->time;
}

CFMutableArrayRef CFArrayCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFArrayCallBacks *callBacks)
{
    struct CFArray *arr = malloc(sizeof(struct CFArray));
    if(arr)
    {
        arr->type = kCFTypeArray;
        arr->refcnt = 1;
        arr->length = 0;
        arr->capacity = capacity;
        arr->callbacks = callBacks;
        arr->elements = malloc(capacity * sizeof(*arr->elements));
        if(!arr->elements)
        {
            free(arr);
            arr = NULL;
        }
    }
    return arr;
}

void CFArrayAppendValue(CFMutableArrayRef theArray, const void *value)
{
    // TODO: thread safety
    struct CFArray *arr = theArray;
    if(arr->callbacks && arr->callbacks->retain)
    {
        arr->callbacks->retain(value);
    }
    if(arr->length == arr->capacity)
    {
        CFIndex newCapacity = arr->capacity * 2;
        void *newElements = realloc(arr->elements, newCapacity * sizeof(*arr->elements));
        if(!newElements)
            abort();
        arr->elements = newElements;
        arr->capacity = newCapacity;
    }
    arr->elements[arr->length++] = value;
}

CFIndex CFArrayGetCount(CFArrayRef theArray)
{
    return ((const struct CFArray*)theArray)->length;
}

const void* CFArrayGetValueAtIndex(CFArrayRef theArray, CFIndex idx)
{
    return ((const struct CFArray*)theArray)->elements[idx];
}

void CFArrayApplyFunction(CFArrayRef theArray, CFRange range, CFArrayApplierFunction applier, void *context)
{
    const struct CFArray *arr = theArray;
    for(CFIndex i = range[0]; i < range[1]; ++i)
    {
        applier(arr->elements[i], context);
    }
}

CFMutableSetRef CFSetCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFSetCallBacks *callBacks)
{
    struct CFArray *set = malloc(sizeof(struct CFArray));
    if(set)
    {
        set->type = kCFTypeSet;
        set->refcnt = 1;
        set->length = 0;
        set->capacity = capacity;
        set->callbacks = callBacks;
        set->elements = malloc(capacity * sizeof(*set->elements));
        if(!set->elements)
        {
            free(set);
            set = NULL;
        }
    }
    return set;
}

void CFSetAddValue(CFMutableSetRef theSet, const void *value)
{
    // TODO: thread safety
    struct CFArray *set = theSet;
    Boolean (*equal)(const void*, const void*) = set->callbacks && set->callbacks->equal ? set->callbacks->equal : NullEqual;
    for(CFIndex i = 0; i < set->length; ++i)
    {
        if(equal(set->elements[i], value))
        {
            return;
        }
    }
    if(set->callbacks && set->callbacks->retain)
    {
        set->callbacks->retain(value);
    }
    if(set->length == set->capacity)
    {
        CFIndex newCapacity = set->capacity * 2;
        void *newElements = realloc(set->elements, newCapacity * sizeof(*set->elements));
        if(!newElements)
            abort();
        set->elements = newElements;
        set->capacity = newCapacity;
    }
    set->elements[set->length++] = value;
}

CFIndex CFSetGetCount(CFSetRef theSet)
{
    return CFArrayGetCount(theSet);
}

void CFSetGetValues(CFSetRef theSet, const void **values)
{
    const struct CFArray *set = theSet;
    memcpy(values, set->elements, set->length * sizeof(*set->elements));
}

void CFSetApplyFunction(CFSetRef theSet, CFSetApplierFunction applier, void *context)
{
    const struct CFArray *set = theSet;
    for(CFIndex i = 0; i < set->length; ++i)
    {
        applier(set->elements[i], context);
    }
}

CFMutableDictionaryRef CFDictionaryCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks)
{
    struct CFDictionary *dict = malloc(sizeof(struct CFDictionary));
    if(dict)
    {
        dict->type = kCFTypeDictionary;
        dict->refcnt = 1;
        dict->length = 0;
        dict->capacity = capacity;
        dict->keyCallbacks = keyCallBacks;
        dict->valueCallbacks = valueCallBacks;
        dict->elements = malloc(capacity * sizeof(*dict->elements));
        if(!dict->elements)
        {
            free(dict);
            dict = NULL;
        }
    }
    return dict;
}

static void CFDictionaryEnterValue(CFMutableDictionaryRef theDict, const void *key, const void *value, bool addOnly)
{
    // TODO: thread safety
    struct CFDictionary *dict = theDict;
    Boolean (*equal)(const void*, const void*) = dict->keyCallbacks && dict->keyCallbacks->equal ? dict->keyCallbacks->equal : NullEqual;
    if(dict->valueCallbacks && dict->valueCallbacks->retain)
    {
        dict->valueCallbacks->retain(value);
    }
    for(CFIndex i = 0; i < dict->length; ++i)
    {
        if(equal(dict->elements[i].key, key))
        {
            if(!addOnly)
            {
                if(dict->valueCallbacks && dict->valueCallbacks->release)
                {
                    dict->valueCallbacks->release(dict->elements[i].value);
                }
                dict->elements[i].value = value;
            }
            return;
        }
    }
    if(dict->keyCallbacks && dict->keyCallbacks->retain)
    {
        dict->keyCallbacks->retain(key);
    }
    if(dict->length == dict->capacity)
    {
        CFIndex newCapacity = dict->capacity * 2;
        void *newElements = realloc(dict->elements, newCapacity * sizeof(*dict->elements));
        if(!newElements)
            abort();
        dict->elements = newElements;
        dict->capacity = newCapacity;
    }
    CFIndex idx = dict->length++;
    dict->elements[idx].key = key;
    dict->elements[idx].value = value;
}

void CFDictionarySetValue(CFMutableDictionaryRef theDict, const void *key, const void *value)
{
    CFDictionaryEnterValue(theDict, key, value, false);
}

void CFDictionaryAddValue(CFMutableDictionaryRef theDict, const void *key, const void *value)
{
    CFDictionaryEnterValue(theDict, key, value, true);
}

CFIndex CFDictionaryGetCount(CFDictionaryRef theDict)
{
    return ((const struct CFDictionary*)theDict)->length;
}

const void* CFDictionaryGetValue(CFDictionaryRef theDict, const void *key)
{
    const struct CFDictionary *dict = theDict;
    Boolean (*equal)(const void*, const void*) = dict->keyCallbacks && dict->keyCallbacks->equal ? dict->keyCallbacks->equal : NullEqual;
    for(CFIndex i = 0; i < dict->length; ++i)
    {
        if(equal(dict->elements[i].key, key))
        {
            return dict->elements[i].value;
        }
    }
    return NULL;
}

void CFDictionaryGetKeysAndValues(CFDictionaryRef theDict, const void **keys, const void **values)
{
    const struct CFDictionary *dict = theDict;
    for(CFIndex i = 0; i < dict->length; ++i)
    {
        if(keys)
            keys[i] = dict->elements[i].key;
        if(values)
            values[i] = dict->elements[i].value;
    }
}

void CFDictionaryApplyFunction(CFDictionaryRef theDict, CFDictionaryApplierFunction applier, void *context)
{
    const struct CFDictionary *dict = theDict;
    for(CFIndex i = 0; i < dict->length; ++i)
    {
        applier(dict->elements[i].key, dict->elements[i].value, context);
    }
}
