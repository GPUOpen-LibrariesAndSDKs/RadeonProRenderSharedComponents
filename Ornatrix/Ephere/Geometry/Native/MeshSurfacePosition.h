// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

#include "Ephere/NativeTools/MacroTools.h"
#include "Matrix.h"

namespace Ephere { namespace Geometry
{

struct MeshSurfacePosition
{
	MeshSurfacePosition():
		faceIndex( -1 )
	{
	}

	MeshSurfacePosition( int faceIndexValue, const Vector3f& barycentricCoordinateValue ):
		faceIndex( faceIndexValue ),
		barycentricCoordinate( barycentricCoordinateValue )
	{
	}

	//! Index of the face on the mesh or id of the base strand for propagated strands
	int faceIndex;

	//! Barycentric coordinate on the face
	Vector3f barycentricCoordinate;

	// Checks if strand propagated (strands whose roots are not on mesh surface but on other strands).
	// In such case, the face index is the index of the strand on which the current strand is located, 
	// and the X barycentric value is the position on the strand from 0 to 1.
	EPHERE_NODISCARD bool IsPropagatedStrand() const
	{
		return !barycentricCoordinate.HasNaNElements() && barycentricCoordinate.z() < -0.5f;
	}

	EPHERE_NODISCARD unsigned GetUniquePropagatedStrandAndSideId() const
	{
		// TODO: Need to find a better way of hashing in the side index
		return faceIndex + unsigned( barycentricCoordinate.y() ) * 1000000;
	}
};

struct SurfacePosition
{
	//! Index of the face on the mesh or id of the base strand for propagated strands
	unsigned faceIndex;

	/** Parametric (UV) coordinates along the two leading edges for triangles and quads, or position along strand for propagated strands (y coordinate is not used in such case)
	 * If the .x value is above or equal to 1 then the whole amount represents sub-face index for Ngons or side index for propagated strands and fractional amount is the coordinate value.
	 * NOTE: Whole value of .y coordinate is currently unused but may be assigned to something in the future.
	 **/
	Vector2f surfaceCoordinate;

	void Set( const int faceIndexValue, const int faceSubIndex, const Vector2f& surfaceCoordinateValue )
	{
		faceIndex = faceIndexValue;
		surfaceCoordinate = surfaceCoordinateValue;
		if( faceSubIndex > 0 )
		{
			surfaceCoordinate.x() += float( faceSubIndex );
		}
	}

	void Get( int& faceIndexValue, int& faceSubIndex, Vector2f& surfaceCoordinateValue )
	{
		faceIndexValue = int( faceIndex );
		surfaceCoordinateValue = surfaceCoordinate;
		if( surfaceCoordinate.x() >= 1.0f )
		{
			faceSubIndex = int( surfaceCoordinate.x() );
			surfaceCoordinateValue.x() = floor( surfaceCoordinateValue.x() );
		}
	}

	// Checks if strand is propagated (strands whose roots are not on mesh surface but on other strands).
	// In such case, the face index is the index of the strand on which the current strand is located, and the x surface coordinate is the position on the strand from 0 to 1.
	EPHERE_NODISCARD bool IsPropagatedStrand() const
	{
		return surfaceCoordinate.y() < 0.0f;
	}

	EPHERE_NODISCARD unsigned GetUniquePropagatedStrandAndSideId() const
	{
		// TODO: Need to find a better way of hashing in the side index
		return faceIndex + unsigned( surfaceCoordinate.x() ) * 1000000;
	}
};

} }
