// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

namespace Ephere { namespace Ornatrix {

/** This struct holds information about topology of a simple hair strand
*/
struct StrandTopology
{
	//! Index into vertices array
	unsigned startingVertexIndex;

	//! Number of vertices in this strand
	unsigned vertexCount;
};

} } // Ephere::Ornatrix
