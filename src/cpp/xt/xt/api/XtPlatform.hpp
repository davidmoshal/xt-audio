#ifndef XT_API_PLATFORM_HPP
#define XT_API_PLATFORM_HPP

/** @file */
/** @cond */
#include <xt/cpp/Core.hpp>
#include <xt/api/Enums.hpp>
#include <xt/api/XtService.hpp>

#include <vector>
#include <memory>
/** @endcond */

namespace Xt {

using namespace Detail;

class Platform final
{
/** @cond */
  friend class Audio;
/** @endcond */
  XtPlatform* const _p;
  Platform(XtPlatform* p): _p(p) { }
public:
  ~Platform();
  std::vector<System> GetSystems();
  System SetupToSystem(Setup setup) const;
  std::unique_ptr<Service> GetService(System system);
};

inline
Platform::~Platform()
{ XtPlatformDestroy(_p); }

inline System
Platform::SetupToSystem(Setup setup) const
{ 
  auto coreSetup = static_cast<XtSetup>(setup);
  auto result = XtPlatformSetupToSystem(_p, coreSetup);
  return static_cast<System>(result); 
}

inline std::vector<System> 
Platform::GetSystems() 
{
  int32_t size = 0;
  XtPlatformGetSystems(_p, nullptr, &size);
  std::vector<System> result(static_cast<size_t>(size));
  auto coreSystems = reinterpret_cast<XtSystem*>(result.data());
  XtPlatformGetSystems(_p, coreSystems, &size);
  return result;
}

inline std::unique_ptr<Service> 
Platform::GetService(System system) 
{
  auto coreSystem = static_cast<XtSystem>(system);
  XtService const* service = XtPlatformGetService(_p, coreSystem);
  if(!service) return std::unique_ptr<Service>();
  return std::unique_ptr<Service>(new Service(service));
}

} // namespace Xt
#endif // XT_API_PLATFORM_HPP