#pragma once

#if defined( __cplusplus ) && __cplusplus >= 201703L
#	define EPHERE_NODISCARD [[nodiscard]]
#else
#	define EPHERE_NODISCARD
#endif

#define EPHERE_NO_UTILITIES
