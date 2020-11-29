#ifndef XT_PRIVATE_SHARED_HPP
#define XT_PRIVATE_SHARED_HPP

#include <xt/api/public/Enums.h>
#include <xt/api/public/Shared.h>
#include <xt/api/public/Structs.h>
#include <cstdint>

#define XT_STRINGIFY(s) #s
#define XT_FAIL(m) XtiFail({__FILE__,  __func__, __LINE__}, m)
#define XT_TRACE(m) XtiTrace({__FILE__,  __func__, __LINE__}, m)
#define XT_ASSERT(c) ((c) || (XT_FAIL("Assertion failed: " #c), 0))

typedef uint32_t XtFault;

bool
XtiCalledOnMainThread();
int32_t
XtiGetPopCount64(uint64_t x);
uint32_t
XtiGetErrorFault(XtError error);
int32_t
XtiGetSampleSize(XtSample sample);
XtError
XtiCreateError(XtSystem system, XtFault fault);
XtServiceError
XtiGetServiceError(XtSystem system, XtFault fault);
void
XtiFail(XtLocation const& location, char const* msg);
void
XtiTrace(XtLocation const& location, char const* msg);
void
XtiCopyString(char const* source, char* buffer, int32_t* size);
void
XtiDeinterleave(void** dst, void const* src, int32_t frames, int32_t channels, int32_t size);
void
XtiInterleave(void* dst, void const* const* src, int32_t frames, int32_t channels, int32_t size);

#endif // XT_PRIVATE_SHARED_HPP