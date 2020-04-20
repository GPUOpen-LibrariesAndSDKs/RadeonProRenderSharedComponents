// Must compile with VC 2012 / GCC 4.8 (partial C++11)

#pragma once

namespace Ephere { namespace Ornatrix
{
	struct StrandChannelName
	{
		// Enum preferred to static const int, see https://stackoverflow.com/questions/5391973/undefined-reference-to-static-const-int
		enum : int { MaximumNameLength = 16 };

		wchar_t name[MaximumNameLength];
	};

} }
