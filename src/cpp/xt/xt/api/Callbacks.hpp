#ifndef XT_API_CALLBACKS_HPP
#define XT_API_CALLBACKS_HPP

/** @file */
/** @cond */
#include <string>
#include <cstdint>
/** @endcond */

namespace Xt {

typedef void (*
OnXRun)(class Stream const& stream, int32_t index, void* user);
typedef void (*
OnError)(struct Location const& location, std::string const& message);
typedef uint32_t (*
OnBuffer)(class Stream const& stream, struct Buffer const& buffer, void* user);
typedef void (*
OnRunning)(class Stream const& stream, bool running, uint64_t error, void* user);

} // namespace Xt
#endif // XT_API_CALLBACKS_HPP