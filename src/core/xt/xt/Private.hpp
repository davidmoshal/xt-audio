#ifndef XT_PRIVATE_HPP
#define XT_PRIVATE_HPP

#include <xt/XtAudio.h>
#include <xt/api/private/Device.hpp>
#include <xt/api/private/Stream.hpp>
#include <xt/api/private/Service.hpp>
#include <xt/private/BlockingStream.hpp>
#include <xt/private/Shared.hpp>
#include <xt/private/Structs.hpp>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <cstring>
#include <cstdarg>

// ---- forward ----

// ---- internal ----

struct XtAggregate;

struct XtAggregateContext {
  int32_t index;
  XtAggregate* stream;
};

struct XtAggregate: public XtStream {
  int32_t frames;
  XtSystem system;
  int32_t masterIndex;
  std::atomic<int32_t> running;
  XtIOBuffers _weave;
  std::vector<XtChannels> channels;
  std::atomic<int32_t> insideCallbackCount;
  std::vector<XtRingBuffer> inputRings; 
  std::vector<XtRingBuffer> outputRings;
  std::vector<XtAggregateContext> contexts;
  std::vector<std::unique_ptr<XtBlockingStream>> streams;

  virtual ~XtAggregate();
  XT_IMPLEMENT_STREAM();
};

void XT_CALLBACK XtiOnSlaveBuffer(const XtStream* stream, const XtBuffer* buffer, void* user);
void XT_CALLBACK XtiOnMasterBuffer(const XtStream* stream, const XtBuffer* buffer, void* user);

#endif // XT_PRIVATE_HPP