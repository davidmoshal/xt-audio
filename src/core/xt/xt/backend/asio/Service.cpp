#if XT_ENABLE_ASIO
#include <xt/shared/Win32.hpp>
#include <xt/private/Platform.hpp>
#include <xt/backend/asio/Shared.hpp>
#include <xt/backend/asio/Private.hpp>

XtFault
AsioService::GetFormatFault() const
{ return XT_ASE_Format; }

XtFault
AsioService::OpenDeviceList(XtEnumFlags flags, XtDeviceList** list) const
{ *list = new AsioDeviceList; return ASE_OK; }

XtServiceCaps
AsioService::GetCapabilities() const
{
  auto result = XtServiceCapsTime
    | XtServiceCapsLatency
    | XtServiceCapsFullDuplex
    | XtServiceCapsChannelMask
    | XtServiceCapsControlPanel;
  return static_cast<XtServiceCaps>(result);
}

XtFault
AsioService::GetDefaultDeviceId(XtBool output, XtBool* valid, char* buffer, int32_t* size) const
{
  AsioDeviceList list;
  if(list._drivers.asioGetNumDev() == 0) return ASE_OK;
  *valid = XtTrue;
  return list.GetId(0, buffer, size);
}

XtFault
AsioService::OpenDevice(char const* id, XtDevice** device) const
{  
  HRESULT hr;
  CLSID classId;
  CComPtr<IASIO> asio;
  auto wideId = XtiUtf8ToWideString(id);
  XT_VERIFY_COM(CLSIDFromString(wideId.data(), &classId));
  XT_VERIFY_COM(CoCreateInstance(classId, nullptr, CLSCTX_ALL, classId, reinterpret_cast<void**>(&asio)));
  if(!asio->init(XtPlatform::instance->_window)) return ASE_NotPresent;
  *device = new AsioDevice(asio);
  return ASE_OK;
}

#endif // XT_ENABLE_ASIO