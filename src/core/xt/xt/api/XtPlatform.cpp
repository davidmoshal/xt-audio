#include <xt/api/XtPlatform.h>
#include <xt/private/Platform.hpp>

void XT_CALL
XtPlatformDestroy(XtPlatform* p)
{
  XT_ASSERT(XtiCalledOnMainThread());
  delete p; 
  XtPlatform::instance = nullptr;
}

XtService const* XT_CALL 
XtPlatformGetService(XtPlatform const* p, XtSystem system)
{
  XT_ASSERT(p != nullptr);
  XT_ASSERT(XtiCalledOnMainThread());
  XT_ASSERT(XtSystemALSA <= system && system <= XtSystemDSound);
  return p->GetService(system);
}

XtSystem XT_CALL 
XtPlatformSetupToSystem(XtPlatform const* p, XtSetup setup)
{
  XT_ASSERT(p != nullptr);
  XT_ASSERT(XtiCalledOnMainThread());
  XT_ASSERT(XtSetupProAudio <= setup && setup <= XtSetupConsumerAudio);
  return p->SetupToSystem(setup);
}

void XT_CALL 
XtPlatformGetSystems(XtPlatform const* p, XtSystem* buffer, int32_t* size)
{
  XT_ASSERT(p != nullptr);
  XT_ASSERT(XtiCalledOnMainThread());
  return p->GetSystems(buffer, size);
}