// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

#ifdef AUTODESK_MAYA

#if defined( _MSC_VER ) && !defined( __clang__ )
#	pragma warning( push, 3 )
#else
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wold-style-cast"
#	pragma clang diagnostic ignored "-Wduplicate-enum"
#	pragma clang diagnostic ignored "-Wreserved-id-macro"
#	pragma clang diagnostic ignored "-Wdocumentation"
#	pragma clang diagnostic ignored "-Wdeprecated"
#	pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#	pragma clang diagnostic ignored "-Wextra-semi"
#endif

// Needed by MTypes.h for intptr_t on Linux
#include <cstdint>

#include <maya/MBoundingBox.h>
#include <maya/MEulerRotation.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MImage.h>
#include <maya/MIntArray.h>
#include <maya/MMatrix.h>
#include <maya/MMeshIntersector.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPoint.h>

#if defined( _MSC_VER ) && !defined( __clang__ )
#	pragma warning( pop )
#else
#	pragma GCC diagnostic pop
#endif

#include <vector>
#include <map>
#include <memory>

#if !defined(_MSC_VER) || _MSC_VER > 1600
#include <mutex>
#endif

namespace Ephere { namespace Plugins
{
	namespace Autodesk { namespace Maya { namespace Ornatrix
	{
		class TextureAccess;
		class TriangleMeshAccess;
		class CurvesAccess;

	} } }

	struct MayaMap
	{
		MayaMap()
		{
		}

		explicit MayaMap( const MPlug& plug )
		{
			mapPlug = plug;
		}

		MPlug mapPlug;
#if !defined(_MSC_VER) || _MSC_VER > 1600
		mutable std::mutex blocker;
#endif
	};

typedef MPoint HostVector2;					// actually 4
typedef MPoint HostVector3;					// actually 4
typedef MPoint HostTextureCoordinate;		// actually 4
typedef MMatrix HostXform3;					// actually 4x4
typedef MObject HostTriangleMesh;				// MFn::Type = kMesh?
typedef MObject HostPolygonMesh;
typedef MObject HostCurves;					// MFn::Type = kNurbsCurve
typedef MIntArray HostTriangleFace;			// 3 indices of triangle vertices
typedef MIntArray HostTriangleTextureFace;	// 3 indices of triangle vertices
typedef MBoundingBox HostAxisAlignedBox3;
typedef std::vector<bool> HostElementSelection;
typedef MayaMap HostTextureMap;
typedef MImage HostImage;
typedef Ephere::Plugins::Autodesk::Maya::Ornatrix::TextureAccess* HostTextureAccessHandle;
typedef Ephere::Plugins::Autodesk::Maya::Ornatrix::TriangleMeshAccess* HostTriangleMeshAccessHandle;
typedef Ephere::Plugins::Autodesk::Maya::Ornatrix::CurvesAccess* HostCurvesAccessHandle;
typedef MMeshIntersector* HostMeshIntersector;

inline void Invert( HostElementSelection& selection )
{
	selection.flip();
}

inline HostXform3 HostInverse( const HostXform3& xform )
{
	return xform.inverse();
}

inline HostVector3 HostTransform( const HostXform3& xform, const HostVector3& point )
{
	return point * xform;
}

inline HostAxisAlignedBox3 HostTransform( const HostXform3& xform, const HostAxisAlignedBox3& box )
{
	HostAxisAlignedBox3 result( box );
	result.transformUsing( xform );
	return result;
}

inline HostVector3 HostBoundingBoxMin( const HostAxisAlignedBox3& box )
{
	return box.min();
}

inline HostVector3 HostBoundingBoxMax( const HostAxisAlignedBox3& box )
{
	return box.max();
}

inline void HostBoundingBoxExpand( HostAxisAlignedBox3& box, const HostAxisAlignedBox3& other )
{
	box.expand( other );
}

} } // Ephere::Plugins

#include "Ephere/Ornatrix/Types.h"

namespace Ephere { namespace Plugins { namespace Autodesk { namespace Maya { namespace Ornatrix
{

struct MayaTextureMap final : Ephere::Ornatrix::ITextureMap
{
	explicit MayaTextureMap( const MPlug& plug )
		: map( plug )
	{
	}

	std::shared_ptr<Ephere::Ornatrix::IMapSampler> CreateSampler( std::vector<Ephere::Ornatrix::TextureCoordinate> textureCoordinatesAtStrandRoots, bool invertValues ) const override;

	MayaMap map;

	static std::shared_ptr<MayaTextureMap> Create( MObject const& node, MObject const& attribute )
	{
		MPlug plug( node, attribute );
		return plug.isConnected() ? std::make_shared<MayaTextureMap>( plug ) : std::shared_ptr<MayaTextureMap>();
	}
};

struct MayaMapSampler final : Ephere::Ornatrix::IMapSampler
{
	MayaMapSampler( MayaTextureMap const& map, std::vector<Ephere::Ornatrix::TextureCoordinate> textureCoordinatesAtStrandRoots, bool invertValues );

	float GetValue( int strandIndex, float positionAlongStrand = 0.0f ) const override;

	Ephere::Ornatrix::Vector3 GetValue3( int strandIndex, float positionAlongStrand = 0.0f ) const override;

	const Ephere::Ornatrix::ITextureMap& GetMap() const override
	{
		return *map;
	}

	MayaTextureMap const* map;
	bool invertValues;
	std::vector<Ephere::Ornatrix::Vector3> perStrandValues;
	std::map<unsigned, MImage> textureMapTiles;
};

struct MayaTriangleMesh final : Ephere::Ornatrix::ITriangleMesh
{
	explicit MayaTriangleMesh( TriangleMeshAccess& meshAccess )
		: meshAccess( &meshAccess )
	{
	}

	void PrepareVertexColorSet( int colorSetIndex ) override;

	int GetVertexColorSetCount() const override;

	Ephere::Ornatrix::Vector3 GetVertexColor( int faceIndex, Ephere::Ornatrix::Vector3 const& barycentricCoordinate, int colorSetIndex ) const override;


	TriangleMeshAccess* meshAccess;
};

} } } } }

#ifdef MAYA_VERSION
#	include "Ephere/NativeTools/Types.h"
	EPHERE_DECLARE_TYPE_SIZE( Ephere::Plugins::HostVector3, 32 )
#endif

#endif
