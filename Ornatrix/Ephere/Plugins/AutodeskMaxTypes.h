/*************************************************************************************************\
*
* THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)
*
\*************************************************************************************************/

#pragma once

#ifdef AUTODESK_3DSMAX

#pragma warning( push, 2 )

// Needed by older Max headers
#include <algorithm>
using std::min;

#include <max.h>
#include <point3.h>
#include <matrix3.h>
#include <mesh.h>
#include <box3.h>
#include <bitarray.h>
#include <object.h>
#include <surf_api.h>
#include <simpshp.h>
#include <spline3d.h>

#include <memory>
#include <vector>

#pragma warning( pop )

#ifndef ASSERT
#	define ASSERT DbgAssert
#endif

namespace Ephere { namespace Plugins
{

struct MaxTriangleMeshAccess
{
	MaxTriangleMeshAccess():
		mesh_( nullptr ),
		hasMeshWeakPointer_( false ),
		isReadOnly( false ),
		isEditingTopology( false ),
		isEditingGeometry( false ),
		isEditingNormals( false ),
		faceVertexCount( 3 )
	{
	}

	explicit MaxTriangleMeshAccess( Mesh* mesh ):
		mesh_( mesh ),
		hasMeshWeakPointer_( false ),
		isReadOnly( false ),
		isEditingTopology( false ),
		isEditingGeometry( false ),
		isEditingNormals( false ),
		faceVertexCount( 3 )
	{
	}

	explicit MaxTriangleMeshAccess( const std::shared_ptr<Mesh>& meshSharedPointer ) :
		mesh_( meshSharedPointer.get() ),
		meshWeakPointer_( meshSharedPointer ),
		hasMeshWeakPointer_( true ),
		isReadOnly( false ),
		isEditingTopology( false ),
		isEditingGeometry( false ),
		isEditingNormals( false ),
		faceVertexCount( 3 )
	{
	}

	Mesh* Get() const
	{
		return hasMeshWeakPointer_ && meshWeakPointer_.expired() ? static_cast<Mesh*>( nullptr ) : mesh_;
	}

	Mesh& operator*() const
	{
		ASSERT( *this != nullptr );
		return *mesh_;
	}

	Mesh* operator->() const
	{
		ASSERT( *this != nullptr );
		return mesh_;
	}

	bool operator==( std::nullptr_t ) const
	{
		return hasMeshWeakPointer_ ? meshWeakPointer_.expired() : mesh_ == nullptr;
	}

	bool operator!=( std::nullptr_t ) const
	{
		return hasMeshWeakPointer_ ? !meshWeakPointer_.expired() : mesh_ != nullptr;
	}

	void Reset()
	{
		mesh_ = nullptr;
		meshWeakPointer_.reset();
		hasMeshWeakPointer_ = false;
	}


private:

	// Warning! The order of members must not be changed be because clients depend on it
	Mesh* mesh_;
	std::weak_ptr<Mesh> meshWeakPointer_;
	bool hasMeshWeakPointer_;

public:
	bool isReadOnly;
	bool isEditingTopology;
	bool isEditingGeometry;
	bool isEditingNormals;

	//! This is set by user in SetFaceCount and is used to alter multiple triangles at the same time in subsequent calls
	int faceVertexCount;
};

struct MaxCurveWrapper
{
	MaxCurveWrapper()
		: nurbsCurve( nullptr ),
		spline( nullptr ),
		simpleShape( nullptr ),
		curveIndex( -1 )
	{
	}

	explicit MaxCurveWrapper( NURBSCurve& obj )
		: nurbsCurve( &obj ),
		spline( nullptr ),
		simpleShape( nullptr ),
		curveIndex( -1 )
	{
	}

	explicit MaxCurveWrapper( Spline3D& obj )
		: nurbsCurve( nullptr ),
		spline( &obj ),
		simpleShape( nullptr ),
		curveIndex( -1 )
	{
	}

	explicit MaxCurveWrapper( SimpleShape& obj, const int index )
		: nurbsCurve( nullptr ),
		spline( nullptr ),
		simpleShape( &obj ),
		curveIndex( index )
	{
	}

	static bool FromMaxObject( Object&, TimeValue, std::vector<MaxCurveWrapper>& result, std::vector<std::shared_ptr<NURBSSet>>& nurbsSets );

	NURBSCurve* nurbsCurve;
	Spline3D* spline;
	SimpleShape* simpleShape;
	int curveIndex;

	Point3 Evaluate( const float positionAlongCurve, const bool useNormalizedSpace = false ) const;

private:

	static bool GetSplineShapeCurves( Object& obj, TimeValue t, std::vector<MaxCurveWrapper>& result );

	//! This is used internally
	std::shared_ptr<Object> ownedObject_;

};

typedef Point2 HostVector2;
typedef Point3 HostVector3;
typedef UVVert HostTextureCoordinate;
typedef Matrix3 HostXform3;
typedef Mesh HostTriangleMesh;
typedef MNMesh HostPolygonMesh;
typedef Face HostTriangleFace;
typedef TVFace HostTriangleTextureFace;
typedef Box3 HostAxisAlignedBox3;
typedef BitArray HostElementSelection;
typedef Texmap HostTextureMap;
typedef void* HostImage; // Used in Maya so far has/needs no 3dsmax eqivalent
typedef void* HostMeshIntersector; // Used in Maya only
typedef MaxTriangleMeshAccess HostTriangleMeshAccessHandle;
typedef MaxCurveWrapper HostCurves;

inline void Invert( HostElementSelection& selection )
{
	selection = ~selection;
}

inline HostXform3 HostInverse( const HostXform3& xform )
{
	return Inverse( xform );
}

inline HostVector3 HostTransform( const HostXform3& xform, const HostVector3& point )
{
	return xform * point;
}

inline HostAxisAlignedBox3 HostTransform( const HostXform3& xform, const HostAxisAlignedBox3& box )
{
	return box * xform;
}

inline HostVector3 HostBoundingBoxMin( const HostAxisAlignedBox3& box )
{
	return box.Min();
}

inline HostVector3 HostBoundingBoxMax( const HostAxisAlignedBox3& box )
{
	return box.Max();
}

inline void HostBoundingBoxExpand( HostAxisAlignedBox3& box, const HostAxisAlignedBox3& other )
{
	box += other;
}

} } // Ephere::Plugins

template <typename C>
std::basic_ostream<C>& operator <<( std::basic_ostream<C>& stream, const Point3& vector )
{
	stream << '[' << ' ' << vector.x << ' ' << vector.y << ' ' << vector.z << ' ' << ']';
	return stream;
}

#include "Ephere/Ornatrix/Types.h"

namespace Ephere { namespace Plugins { namespace Autodesk { namespace Max { namespace Ornatrix
{

struct MaxTextureMap final : Ephere::Ornatrix::ITextureMap
{
	explicit MaxTextureMap( Texmap const& map )
		: map( &map )
	{
	}

	std::shared_ptr<Ephere::Ornatrix::IMapSampler> CreateSampler( std::vector<Ephere::Ornatrix::TextureCoordinate> textureCoordinatesAtStrandRoots, bool invertValues ) const override;

	Texmap const* map;

	static std::shared_ptr<MaxTextureMap> Create( Texmap const* map )
	{
		return map != nullptr ? std::make_shared<MaxTextureMap>( *map ) : std::shared_ptr<MaxTextureMap>();
	}
};

struct MaxMapSampler final : Ephere::Ornatrix::IMapSampler
{
	MaxMapSampler( MaxTextureMap const& map, std::vector<Ephere::Ornatrix::TextureCoordinate> textureCoordinatesAtStrandRoots, bool invertValues );

	float GetValue( int strandIndex, float positionAlongStrand = 0.0f ) const override;

	Ephere::Ornatrix::Vector3 GetValue3( int strandIndex, float positionAlongStrand = 0.0f ) const override;

	const Ephere::Ornatrix::ITextureMap& GetMap() const override
	{
		return *map;
	}

	MaxTextureMap const* map;
	bool invertValues;
	std::vector<UVVert> textureCoordinates;
};

struct MaxTriangleMesh final : Ephere::Ornatrix::ITriangleMesh
{
	MaxTriangleMesh( MaxTriangleMeshAccess const& mesh )
		: meshAccess( mesh )
	{
	}

	void PrepareVertexColorSet( int colorSetIndex ) override;

	int GetVertexColorSetCount() const override;

	Ephere::Ornatrix::Vector3 GetVertexColor( int faceIndex, Ephere::Ornatrix::Vector3 const& barycentricCoordinate, int colorSetIndex ) const override;


	MaxTriangleMeshAccess meshAccess;
};

} } } } }

#endif
