// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

#include "Ephere/Geometry/Native/Matrix.h"
#include "Ephere/Plugins/CommonTypes.h"
#include "Ephere/Ornatrix/Types.h"

namespace Ephere { namespace Plugins { namespace Ornatrix
{

/** Surface dependency:
	This class is optional for hair but mandatory to guides. It is required to update root position based
	on the deforming surface, to get correct UVW coordinates, and surface normal to generate strand transformations
*/
struct SurfaceDependency
{
	SurfaceDependency()
		: group( 0 ),
		face( unsigned( -1 ) ),
		barycentricCoordinate( HostVector3( 0, 0, 0 ) )
	{
	}

	SurfaceDependency( unsigned short group, unsigned face, HostVector3 barycentricCoordinate )
		: group( group ),
		face( face ),
		barycentricCoordinate( barycentricCoordinate )
	{
	}

	//! Default group used for all strands
	static const unsigned short DefaultGroup = 0;

	//! Strand group index
	unsigned short group;

	//! Index into mesh face list
	unsigned face;

	//! Barycentric coordinates of this root on face
	HostVector3 barycentricCoordinate;
};

} } } // Ephere::Plugins::Ornatrix
