#pragma once

namespace Ephere { namespace Ornatrix
{

//! Using dependency on 3 guides (max)
const int MaxGuideInterpolationCount = 3;

/** Guide dependency struct:
	Mostly used with hair this class allows every hair strand to point to the guide that it depends on.
	Most useful implementation for this is channel inheritance, but it can be also used for updating strand
	shape, export to game engines, and hair hierarchy maintenance
*/
struct GuideDependency
{
	//! Index of MaxGuideInterpolationCount closest guides
	// TODO: Change to StrandId
	unsigned closestRootIndices[MaxGuideInterpolationCount];

	//! Distances to MaxGuideInterpolationCount closest guides
	float closestRootDistances[MaxGuideInterpolationCount];
};

struct GuideDependency2
{
	unsigned strandId;
	float weight;
};

} } // Ephere::Ornatrix
