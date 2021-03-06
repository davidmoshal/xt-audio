#include <xt/shared/Shared.hpp>
#include <xt/private/Stream.hpp>
#include <xt/aggregate/Stream.hpp>

void*
XtAggregateStream::GetHandle() const
{ return _streams[_masterIndex]->GetHandle(); }
XtSystem
XtAggregateStream::GetSystem() const
{ return _streams[_masterIndex]->GetSystem(); }
XtFault
XtAggregateStream::GetFrames(int32_t* frames) const
{ return *frames = _frames, 0; }
void
XtAggregateStream::StopMasterBuffer()
{ _streams[_masterIndex]->StopMasterBuffer(); }
XtFault 
XtAggregateStream::ProcessBuffer()
{ return _streams[_masterIndex]->ProcessBuffer(); }
XtFault
XtAggregateStream::StartMasterBuffer()
{ return _streams[_masterIndex]->StartMasterBuffer(); }
XtFault
XtAggregateStream::BlockMasterBuffer(XtBool* ready)
{ return _streams[_masterIndex]->BlockMasterBuffer(ready); }

void
XtAggregateStream::StopSlaveBuffer()
{
  _streams[_masterIndex]->StopSlaveBuffer();
  for(size_t i = 0; i < _streams.size(); i++)
    if(i != static_cast<size_t>(_masterIndex))
      _streams[i]->StopSlaveBuffer();
}

XtFault
XtAggregateStream::PrefillOutputBuffer()
{
  XtFault fault;
  for(size_t i = 0; i < _streams.size(); i++)
    if((fault = _streams[i]->PrefillOutputBuffer()) != 0) return fault;
  return 0;
}

XtFault
XtAggregateStream::StartSlaveBuffer()
{
  XtFault fault;
  for(size_t i = 0; i < _streams.size(); i++)
  {
    _rings[i].input.Clear();
    _rings[i].output.Clear();
  }

  auto guard = XtiGuard([this] { StopSlaveBuffer(); });
  for(size_t i = 0; i < _streams.size(); i++)
    if(i != static_cast<size_t>(_masterIndex))
      if((fault = _streams[i]->StartSlaveBuffer()) != 0) return fault;
  if((fault = _streams[_masterIndex]->StartSlaveBuffer()) != 0) return fault;
  guard.Commit();
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