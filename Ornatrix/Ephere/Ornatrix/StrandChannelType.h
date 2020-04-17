// Must compile with VC 2012 / GCC 4.8 (partial C++11)

#pragma once

namespace Ephere { namespace Ornatrix
{
	/** Specifies the type of component of a hair object to which data is attached.
		For example, a data channel can be created where one data unit is attached to each vertex. Alternatively, a data unit can be attached only to each strand.
	*/
	enum StrandChannelType
	{
		StrandChannelType_Unassigned = -1,

		//! Per-strand data channel
		StrandChannelType_PerStrand = 0,

		//! Per-vertex data channel
		StrandChannelType_PerVertex,

		//! Channel inherited from distribution mesh and interpolated from its per-vertex data
		StrandChannelType_PerDistributionMeshVertex,

		StrandChannelTypeCount
	};

} }
