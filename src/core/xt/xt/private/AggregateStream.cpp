#include <xt/private/AggregateStream.hpp>

XtSystem
XtAggregateStream::GetSystem() const
{ return _system; }

XtFault
XtAggregateStream::GetFrames(int32_t* frames) const
{ return *frames = _frames, 0; }

XtFault
XtAggregateStream::Stop()
{
  XtFault fault;
  if(!XtiCompareExchange(_running, 1, 0)) return 0;
  while(_insideCallbackCount.load() != 0);
  XT_ASSERT(0 <= _masterIndex && _masterIndex < _streams.size());
  if((fault = _streams[_masterIndex]->Stop()) != 0) return fault;
  for(size_t i = 0; i < _streams.size(); i++)
    if(i != static_cast<size_t>(_masterIndex))
      if((fault = _streams[i]->Stop) != 0) return fault;
  return 0;
}

XtFault
XtAggregateStream::Start()
{
  XtFault fault;
  for(size_t i = 0; i < _streams.size(); i++)
  {
    _rings[i].input.Clear();
    _rings[i].output.Clear();
  }
  for(size_t i = 0; i < _streams.size(); i++)
    if(i != static_cast<size_t>(_masterIndex))
      if((fault = _streams[i]->Start()) != 0) return fault;
  if((fault = _streams[_masterIndex]->Start()) != 0)  return fault;
  XT_ASSERT(XtiCompareExchange(_running, 0, 1));
  return 0;
}

XtFault
XtAggregateStream::GetLatency(XtLatency* latency) const 
{
  XtFault fault;
  XtLatency local = { 0 };
  auto invRate = 1000.0 / _params.format.mix.rate;
  for(size_t i = 0; i < _streams.size(); i++)
  {
    if((fault = _streams[i]->GetLatency(&local)) != 0) return fault;
    if(local.input == 0.0 && local.output == 0.0) return 0;
    if(local.input > 0.0)
    {
      local.input += _rings[i].input.Full() * invRate;
      latency->input = local.input > latency->input? local.input: latency->input;
    }      
    if(local.output > 0.0)
    {
      local.output += _rings[i].output.Full() * invRate;
      latency->output = local.output > latency->output? local.output: latency->output;
    }      
  }
  return 0;
}

void XT_CALLBACK XtiOnSlaveBuffer(
  const XtStream* stream, const XtBuffer* buffer, void* user) {

  size_t i;
  int32_t read, written;
  auto ctx = static_cast<XtAggregateContext*>(user);
  int32_t index = ctx->index;
  XtAggregate* aggregate = ctx->stream;
  auto sampleSize = XtiGetSampleSize(aggregate->_params.format.mix.sample);
  XtOnXRun onXRun = aggregate->_params.stream.onXRun;
  XtRingBuffer& inputRing = aggregate->inputRings[index];
  XtRingBuffer& outputRing = aggregate->outputRings[index];
  const XtChannels& channels = aggregate->channels[index];

  aggregate->insideCallbackCount++;
  if(buffer->error != 0) {
    for(i = 0; i < aggregate->streams.size(); i++)
      if(i != static_cast<size_t>(index))
        aggregate->streams[i]->RequestStop();
    aggregate->_params.stream.onBuffer(aggregate, buffer, aggregate->_user);
  } else {

    if(aggregate->running.load() != 1) {
      ZeroBuffer(buffer->output, aggregate->_params.stream.interleaved, 0, channels.outputs, buffer->frames, sampleSize);
    } else {

      if(buffer->input != nullptr) {
        inputRing.Lock();
        written = inputRing.Write(buffer->input, buffer->frames);
        inputRing.Unlock();
        if(written < buffer->frames && onXRun != nullptr)
          onXRun(-1, aggregate->_user);
      }
  
      if(buffer->output != nullptr) {
        outputRing.Lock();
        read = outputRing.Read(buffer->output, buffer->frames);
        outputRing.Unlock();
        if(read < buffer->frames) {
          ZeroBuffer(buffer->output, aggregate->_params.stream.interleaved, read, channels.outputs, buffer->frames - read, sampleSize);
          if(onXRun != nullptr)
            onXRun(-1, aggregate->_user);
        }
      }
    }
  }
  aggregate->insideCallbackCount--;
}

void XT_CALLBACK XtiOnMasterBuffer(
  const XtStream* stream, const XtBuffer* buffer, void* user) {

  size_t i;
  void* appInput;
  void* appOutput;
  void* ringInput;
  void* ringOutput;
  XtRingBuffer* thisInRing;
  XtRingBuffer* thisOutRing;
  const XtStream* thisStream;
  const XtFormat* thisFormat;
  int32_t c, read, written, totalChannels;

  auto ctx = static_cast<XtAggregateContext*>(user);
  int32_t index = ctx->index;
  XtAggregate* aggregate = ctx->stream;
  int32_t sampleSize = XtiGetSampleSize(aggregate->_params.format.mix.sample);
  XtBool interleaved = aggregate->_params.stream.interleaved;
  const XtFormat* format = &aggregate->_params.format;
  XtOnXRun onXRun = aggregate->_params.stream.onXRun;
  XtRingBuffer& inputRing = aggregate->inputRings[index];
  XtRingBuffer& outputRing = aggregate->outputRings[index];
  const XtChannels& channels = aggregate->channels[index];

  for(i = 0; i < aggregate->streams.size(); i++)
    if(i != static_cast<size_t>(aggregate->masterIndex))
      static_cast<XtBlockingStream*>(aggregate->streams[i].get())->ProcessBuffer(false);

  XtiOnSlaveBuffer(stream, buffer, user);
  if(buffer->error != 0) {
    return;
  }

  if(aggregate->running.load() != 1)
    return;

  aggregate->insideCallbackCount++;

  ringInput = interleaved? 
    static_cast<void*>(&(aggregate->_weave.input.interleaved[0])):
    &(aggregate->_weave.input.nonInterleaved[0]);
  ringOutput = interleaved? 
    static_cast<void*>(&(aggregate->_weave.output.interleaved[0])):
    &(aggregate->_weave.output.nonInterleaved[0]);
  appInput = format->channels.inputs == 0?
    nullptr:
    interleaved? 
    static_cast<void*>(&(aggregate->_buffers.input.interleaved[0])):
    &(aggregate->_buffers.input.nonInterleaved[0]);
  appOutput = format->channels.outputs == 0?
    nullptr:
    interleaved? 
    static_cast<void*>(&(aggregate->_buffers.output.interleaved[0])):
    &(aggregate->_buffers.output.nonInterleaved[0]);

  totalChannels = 0;
  for(i = 0; i < aggregate->streams.size(); i++) {
    thisInRing = &aggregate->inputRings[i];
    thisStream = aggregate->streams[i].get();
    thisFormat = &aggregate->streams[i]->_params.format;
    if(thisFormat->channels.inputs > 0) {
      thisInRing->Lock();
      read = thisInRing->Read(ringInput, buffer->frames);
      thisInRing->Unlock();
      if(read < buffer->frames) {
        ZeroBuffer(ringInput, interleaved, read, thisFormat->channels.inputs, buffer->frames - read, sampleSize);
        if(onXRun != nullptr)
          onXRun(-1, aggregate->_user);
      }
      for(c = 0; c < thisFormat->channels.inputs; c++)
        Weave(appInput, ringInput, interleaved, format->channels.inputs, thisFormat->channels.inputs, totalChannels + c, c, buffer->frames, sampleSize);
      totalChannels += thisFormat->channels.inputs;
    }
  }

  XtBuffer appBuffer = *buffer;
  appBuffer.input = appInput;
  appBuffer.output = appOutput;
  aggregate->_params.stream.onBuffer(aggregate, &appBuffer, aggregate->_user);

  totalChannels = 0;
  for(i = 0; i < aggregate->streams.size(); i++) {
    thisOutRing = &aggregate->outputRings[i];
    thisStream = aggregate->streams[i].get();
    thisFormat = &aggregate->streams[i]->_params.format;
    if(thisFormat->channels.outputs > 0) {
      for(c = 0; c < thisFormat->channels.outputs; c++)
        Weave(ringOutput, appOutput, interleaved, thisFormat->channels.outputs, format->channels.outputs, c, totalChannels + c, buffer->frames, sampleSize);
      totalChannels += thisFormat->channels.outputs;
      thisOutRing->Lock();
      written = thisOutRing->Write(ringOutput, buffer->frames);
      thisOutRing->Unlock();
      if(written < buffer->frames && onXRun != nullptr)
        onXRun(-1, aggregate->_user);
    }
  }

  aggregate->insideCallbackCount--;
}