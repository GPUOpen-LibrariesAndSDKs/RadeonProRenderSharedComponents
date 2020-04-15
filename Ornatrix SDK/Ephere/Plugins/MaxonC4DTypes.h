/*************************************************************************************************\
*
* THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)
*
\*************************************************************************************************/

#pragma once

#ifdef MAXON_C4D

#include "Ephere/Ornatrix/Types.h"

#ifdef _MSC_VER
#	pragma warning( push )
#	pragma warning( disable : 4100 )
#endif
#include "c4d.h"
#ifdef _MSC_VER
#	pragma warning( pop )
#endif

#include <vector>
#include <memory>
#include <iostream>
#include <sstream>

#if (API_VERSION >= 20000) && !defined(_MAXON_API_COMPATIBILITY_H)
#define COPYFLAGS_0 COPYFLAGS::NONE
#define INITRENDERRESULT_OK INITRENDERRESULT::OK
#endif

#if (API_VERSION < 20000)
#define C4D_MutexDeclare(name)		AutoAlloc<::Semaphore> name
#define C4D_MutexLock(name,thread)	(name)->Lock(static_cast<BaseThread*>(thread))
#define C4D_MutexUnlock(name)		(name)->Unlock()
#else
#define C4D_MutexDeclare(name)		maxon::Spinlock name
#define C4D_MutexLock(name,thread)	(name).Lock()
#define C4D_MutexUnlock(name)		(name).Unlock()
#endif

namespace Ephere::Plugins::Maxon::C4D {
	
	// From C4DUtilities.h
	BaseObject* GetBaseLinkObject( BaseDocument* doc, const BaseLink* baseLink );
	std::string C4DRenderResultToString( const INITRENDERRESULT type );

	namespace Ornatrix {
		class TriangleMesh;
		class TriangleMeshAccess;
		
		// From OxC4DUtilities.h
		BaseTag* GetDistributionMeshTag( BaseTag* originalTag, const TriangleMeshAccess* meshAccessHandle );
	}
}

namespace Ephere::Plugins {

using Maxon::C4D::Ornatrix::TriangleMesh;
using Maxon::C4D::Ornatrix::TriangleMeshAccess;

class HostVector : public Vector64
{
public:
	HostVector() : Vector64(0.0) {}
	HostVector(Float64 v) : Vector64(v) {}
	HostVector(Float64 x, Float64 y) : Vector64(x, y, 0.0) {}
	HostVector(Float64 x, Float64 y, Float64 z) : Vector64(x, y, z) {}
	HostVector(Float64 x, Float64 y, Float32 z) : Vector64(x, y, Float64(z)) {}
	HostVector(Float32 v) : Vector64(Float64(v)) {}
	HostVector(Float32 x, Float32 y) : Vector64(Float64(x), Float64(y), 0.0) {}
	HostVector(Float32 x, Float32 y, Float32 z) : Vector64(Float64(x), Float64(y), Float64(z)) {}
	HostVector(Float32 x, Float32 y, Float64 z) : Vector64(Float64(x), Float64(y), z) {}
	HostVector(Int32 v) : Vector64(Float64(v)) {}
	HostVector(Int32 x, Int32 y) : Vector64(Float64(x), Float64(y), 0.0) {}
	HostVector(Int32 x, Int32 y, Int32 z) : Vector64(Float64(x), Float64(y), Float64(z)) {}
	HostVector(Int64 v) : Vector64(Float64(v)) {}
	HostVector(Int64 x, Int64 y) : Vector64(Float64(x), Float64(y), 0.0) {}
	HostVector(Int64 x, Int64 y, Int64 z) : Vector64(Float64(x), Float64(y), Float64(z)) {}
	HostVector(const Vector64 &v) : Vector64(v) {}
	HostVector(const Vector32 &v) : Vector64(v) {}
template <typename T>
	HostVector(const Geometry::Matrix<2,1,T> &v) : Vector64( Float64(v.x()), Float64(v.y()), 0.0 ) {}
	template <typename T>
	HostVector(const Geometry::Matrix<3,1,T> &v) : Vector64( Float64(v.x()), Float64(v.y()), Float64(v.z()) ) {}

	friend std::ostream& operator<<(std::ostream& os, const HostVector& v)
	{
		os << '(' << v.x << ',' << v.y << ',' << v.z << ')';
		return os;  
	}
	
	const std::string ToString()
	{
		std::stringstream result;
		result << (*this);
		return result.str();
	}

//	operator const Vector64()
//	{
//		return Vector64( x, y, z );
//	}

	// Need to overload because superclass operators will set auto to Vector64
	// cloned from External/Maxon/Cinema4D/SDK/RXX/OSX/frameworks/cinema.framework/source/ge_vector.h
	const HostVector& operator += ( Float64 s )
	{
		x += s;
		y += s;
		z += s;
		return *this;
	}
	const HostVector& operator += ( const HostVector& v )
	{
		Vector64::operator += ( v );
		return *this;
	}
	const HostVector& operator -= ( Float64 s )
	{
		x -= s;
		y -= s;
		z -= s;
		return *this;
	}
	const HostVector& operator -= ( const HostVector& v )
	{
		Vector64::operator -= ( v );
		return *this;
	}
	const HostVector& operator *= ( Float64 s )
	{
		Vector64::operator *= ( s );
		return *this;
	}
	const HostVector& operator /= ( Float64 s )
	{
		Vector64::operator /= ( s );
		return *this;
	}
	const HostVector& operator *= ( const HostVector& v )
	{
		Vector64::operator *= ( v );
		return *this;
	}
	friend const HostVector operator ! ( const HostVector& v )
	{
		Float64 l = Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		if (l == 0.0)
			return HostVector(0.0);
		l = 1.0 / l;
		return HostVector(v.x * l, v.y * l, v.z * l);
	}
	friend const HostVector operator * ( Float64 s, const HostVector& v )
	{
		return HostVector(v.x * s, v.y * s, v.z * s);
	}
//#if (API_VERSION < 20000)
	friend const HostVector operator * ( const HostVector& v, Float32 s )
	{
		return HostVector(v.x * Float64(s), v.y * Float64(s), v.z * Float64(s) );
	}
	friend const HostVector operator * ( const HostVector& v, Float64 s )
	{
		return HostVector(v.x * s, v.y * s, v.z * s);
	}
	friend const HostVector operator / ( const HostVector& v, Float64 s )
	{
		if (s != 0.0)
		{
			s = 1.0 / s;
			return HostVector(v.x * s, v.y * s, v.z * s);
		}
		return HostVector(0.0);
	}
//#endif
	friend const HostVector operator * ( const HostVector& v1, const HostVector& v2 )
	{
		return HostVector(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
	}
	friend const HostVector operator + ( Float64 s, const HostVector& v )
	{
		return HostVector(v.x + s, v.y + s, v.z + s);
	}
	friend const HostVector operator + ( const HostVector& v, Float64 s )
	{
		return HostVector(v.x + s, v.y + s, v.z + s);
	}
	friend const HostVector operator + ( const HostVector& v1, const HostVector& v2 )
	{
		return HostVector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
	}
	friend const HostVector operator - ( Float64 s, const HostVector& v )
	{
		return HostVector(s - v.x, s - v.y, s - v.z);
	}
	friend const HostVector operator - ( const HostVector& v, Float64 s )
	{
		return HostVector(v.x - s, v.y - s, v.z - s);
	}
	friend const HostVector operator - ( const HostVector& v1, const HostVector& v2 )
	{
		return HostVector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	}
	friend const HostVector operator - ( const HostVector& v )
	{
		return HostVector(-v.x, -v.y, -v.z);
	}
};

class PolygonObjectWrapper
{
public:
	
	PolygonObjectWrapper()
	{
		Make( nullptr, nullptr );
	}
	
	PolygonObjectWrapper( PolygonObject* polygon, BaseObject *originalObject, const Matrix xform=Matrix() ):
		xform_( xform )
	{
		Make( polygon, originalObject );
	}
	
	PolygonObjectWrapper( const PolygonObjectWrapper& src )
	{
		Set( src );
	}

	void Set( const PolygonObjectWrapper& src )
	{
		polygon_ = src.GetPolygonPointer();
		baseLink_ = src.GetBaseLinkPointer();
		mesh_ = src.GetMeshPointer();
	}
	
	void Reset()
	{
		polygon_.reset();
		baseLink_.reset();
		mesh_.reset();
		MakeMesh();	// initialize empty mesh
	}
	
	const std::shared_ptr<PolygonObject>& GetPolygonPointer() const
	{
		return polygon_;
	}
	
	PolygonObject* GetPolygon() const
	{
		return polygon_.get();
	}
	
	const std::shared_ptr<BaseLink>& GetBaseLinkPointer() const
	{
		return baseLink_;
	}
	
	BaseLink* GetBaseLink() const
	{
		return baseLink_.get();
	}
	
	const std::shared_ptr<TriangleMesh>& GetMeshPointer() const
	{
		return mesh_;
	}
	
	TriangleMesh* GetMesh() const
	{
		return mesh_.get();
	}
	
	Matrix64 GetMatrix( BaseDocument* doc ) const
	{
		return GetMatrix( doc, false );
	}
	
	Matrix64 GetLocalMatrix( BaseDocument* doc ) const
	{
		return GetMatrix( doc, true );
	}
	
	Matrix GetXformMatrix() const
	{
		return xform_;
	}
	

private:
	
	Matrix64 GetMatrix( BaseDocument* doc, bool local ) const
	{
		Matrix64 result;
		if ( baseLink_ != nullptr )
		{
			auto linkedObject = Maxon::C4D::GetBaseLinkObject( doc, baseLink_.get() );
			if ( linkedObject != nullptr )
			{
				result = local ? linkedObject->GetMl() : linkedObject->GetMg();
				return result;
			}
		}
		if (polygon_ != nullptr )
		{
			result = local ? polygon_->GetMl() : polygon_->GetMg();
		}
		return result;
	}
	
	void Make( PolygonObject* polygon, BaseObject *originalObject )
	{
		// Clone polygon object into a new shared pointer
		if ( polygon != nullptr )
		{
			std::shared_ptr<PolygonObject> newPolygon( static_cast<PolygonObject*>( polygon->GetClone( COPYFLAGS_0, nullptr ) ),
													  [](PolygonObject* polygon) { PolygonObject::Free( polygon ); } );	// deleter
			polygon_ = newPolygon;
		}
		// Create empty polygon to be filled
		else
		{
			std::shared_ptr<PolygonObject> newPolygon( PolygonObject::Alloc( 0, 0 ),
													  [](PolygonObject* polygon) { PolygonObject::Free( polygon ); } );	// deleter
			polygon_ = newPolygon;
		}
		if ( polygon_ == nullptr )
		{
			GePrint( "PolygonObjectWrapper:: Error cloning polygon"_s );
		}

		// Clone base link into a new shared pointer
		if ( originalObject != nullptr )
		{
			std::shared_ptr<BaseLink> newBaseLink( static_cast<BaseLink*>( BaseLink::Alloc() ),
												  [](BaseLink* baseLink) { BaseLink::Free( baseLink ); } );	// deleter
			if ( newBaseLink != nullptr )
			{
				newBaseLink->SetLink( originalObject );
			}
			else
			{
				GePrint( "PolygonObjectWrapper:: Error creating shared link"_s );
			}
			baseLink_ = newBaseLink;
		}

		// Parse object into a TriangleMesh
		MakeMesh();
		if ( mesh_ == nullptr )
		{
			GePrint( "PolygonObjectWrapper:: Error parsing mesh"_s );
		}
	}
	
	// Defined in TriangleMeshAccess.cpp
	void MakeMesh();
	
	// A single cached polygon
	std::shared_ptr<PolygonObject> polygon_;
	
	// BaseLink to the source object
	// May be different than the polygon (like a primitive or selection tag)
	std::shared_ptr<BaseLink> baseLink_;
	
	// The parsed TriangleMesh
	std::shared_ptr<TriangleMesh> mesh_;
	
	// Transform applied to all points on parsing
	Matrix xform_;
};

class BaseShaderWrapper
{
public:
	
	BaseShaderWrapper() :
	shader_( nullptr ),
	bitmap_( nullptr ),
	meshAccessHandle_(nullptr),
	defaultValue_( 1.0f )
	{
	}

	BaseShaderWrapper( BaseShaderWrapper& src, BaseDocument* doc, BaseThread* thread ) :
	shader_( nullptr ),
	bitmap_( nullptr ),
	meshAccessHandle_(nullptr),
	defaultValue_( src.GetDefaultValue() )
	{
		Initialize( src.GetBaseShader(), doc, thread );
	}
	
	BaseShaderWrapper( BaseShader* src, BaseDocument* doc, BaseThread* thread, Float32 defaultValue=1.0f ) :
	shader_( nullptr ),
	bitmap_( nullptr ),
	meshAccessHandle_(nullptr),
	defaultValue_( defaultValue )
	{
		Initialize( src, doc, thread );
	}
	
	~BaseShaderWrapper()
	{
		Free();
	}
	
	bool Initialize( BaseShader* src, BaseDocument* doc, BaseThread* thread )
	{
		Free();
		if ( src == nullptr )
		{
			return false;
		}
		
		document_ = ( doc != nullptr ? doc : GetActiveDocument() );
		InitRenderStruct irs( document_ );
		
		thread_ = thread;
		if ( thread_ != nullptr )
		{
			irs.thread = thread_;
		}
		
		colorSpace_ = ( irs.linear_workflow ? COLORSPACETRANSFORMATION_LINEAR_TO_SRGB : COLORSPACETRANSFORMATION_NONE );

		shaderError_ = src->InitRender( irs );
		if ( shaderError_ != INITRENDERRESULT_OK )
		{
			shader_ = nullptr;
			GePrint( String( "BaseShaderWrapper:: InitRender() " ) + String( Maxon::C4D::C4DRenderResultToString( shaderError_ ).c_str() ) );
			return false;
		}
		
		shader_ = src;

		if ( shader_ != nullptr )
		{
			// Some shaders need to be baked
			if ( shader_->IsInstanceOf( Xgradient ) || shader_->IsInstanceOf( Xvertexmap ) )
			{
				// Alloc bitmap to bake in CommonImplementation::Evaluate( const HostTextureMap&, std::map<unsigned, HostImage>& )
				bitmap_ = BaseBitmap::Alloc();
			}
		}

		return true;
	}
	
	HostVector Evaluate( const HostVector& uv ) const
	{
		if ( shader_ == nullptr )
		{
			return defaultValue_;
		}
		ChannelData channelData;
		channelData.p = ::Vector( uv );
		channelData.t = ( document_ != nullptr ? document_->GetTime().Get() : 0 );
		
		C4D_MutexLock(lock_,thread_);
		auto sample = shader_->Sample( &channelData );
		C4D_MutexUnlock(lock_);

		auto result = TransformColor( sample, colorSpace_ ).Clamp01();
		return HostVector( result );
	}
	
	BaseShader* GetBaseShader() const
	{
		return shader_;
	}
	
	BaseBitmap* GetBitmap() const
	{
		return bitmap_;
	}
	
	BaseDocument* GetDocument() const
	{
		return document_;
	}
	
	bool HasBaseShader() const
	{
		return ( shader_ != nullptr );
	}
	
	float GetDefaultValue() const
	{
		return defaultValue_;
	}
	
	INITRENDERRESULT GetShaderError() const
	{
		return shaderError_;
	}
	
	void SetDistributionMesh( const TriangleMeshAccess* meshAccessHandle )
	{
		meshAccessHandle_ = meshAccessHandle;
	}
	
	// the vertex map shader uses a tag that could be converted when parsing the distribution mesh
	BaseTag* GetDistributionMeshTag( BaseTag* originalTag ) const
	{
		return Maxon::C4D::Ornatrix::GetDistributionMeshTag( originalTag, meshAccessHandle_ );
	}

private:
	
	void Free()
	{
		if ( bitmap_ != nullptr )
		{
			BaseBitmap::Free( bitmap_ );
			bitmap_ = nullptr;
		}
		if ( shader_ != nullptr )
		{
			shader_->FreeRender();
			shader_ = nullptr;
		}
	}

	BaseShader* shader_;
	BaseBitmap* bitmap_;
	BaseDocument* document_;
	BaseThread* thread_;
	const TriangleMeshAccess* meshAccessHandle_;

	INITRENDERRESULT shaderError_ = INITRENDERRESULT_OK;

	float defaultValue_;
	
	COLORSPACETRANSFORMATION colorSpace_;

	mutable C4D_MutexDeclare(lock_);
};

class BaseBitmapWrapper
{
public:
	
	BaseBitmapWrapper()
	{
	}
	
	BaseBitmapWrapper( const BaseBitmapWrapper& source )
	{
		source.GetBitmap().CopyTo( bitmap_ );
	}
	
	BaseBitmapWrapper& operator=( const BaseBitmapWrapper& source )
	{
		source.GetBitmap().CopyTo( bitmap_ );
		return *this;
	}
	
	const BaseBitmap& GetBitmap() const
	{
		return bitmap_;
	}
	
	BaseBitmap& GetBitmap()
	{
		return bitmap_;
	}
	
private:
	
	AutoAlloc<BaseBitmap> bitmap_;
};


struct C4DCurveWrapper
{
	C4DCurveWrapper()
	: splineObject_( nullptr ),
	splineInstance_( nullptr ),
	segmentIndex_( unsigned( -1 ) )
	{
	}
	
	explicit C4DCurveWrapper( SplineObject& obj, Matrix objectToWorldTransform, unsigned segmentIndex=0 )
	: splineObject_( &obj ),
	splineInstance_( nullptr ),
	startTransform_( ( ~objectToWorldTransform * ~obj.GetMg()) * objectToWorldTransform ),
	segmentIndex_( segmentIndex )
	{
	}
	
	explicit C4DCurveWrapper( std::shared_ptr<SplineObject> instance, Matrix objectToWorldTransform, unsigned segmentIndex=0 )
	: splineObject_( instance.get() ),
	splineInstance_( instance ),
	startTransform_( ( ~objectToWorldTransform * ~instance->GetMg()) * objectToWorldTransform ),
	segmentIndex_( segmentIndex )
	{
	}
	
	static bool FromC4DObject( BaseObject* obj, std::vector<C4DCurveWrapper>& result, const Matrix objectToWorldTransform );
	
	static bool FromDeformedSpline( BaseObject* obj, std::vector<C4DCurveWrapper>& result, const Matrix objectToWorldTransform );
	
	static bool FromNewSpline( SplineObject* splineObject, std::vector<C4DCurveWrapper>& result, const Matrix objectToWorldTransform );
	
	HostVector Evaluate( const float positionAlongCurve, const bool useNormalizedSpace = false ) const;
	
	SplineObject* splineObject_;
	std::shared_ptr<SplineObject> splineInstance_;
	Matrix startTransform_;
	unsigned segmentIndex_;
};
	
typedef HostVector HostVector2;
typedef HostVector HostVector3;
typedef HostVector HostTextureCoordinate;	
typedef Matrix64 HostXform3;
typedef Ornatrix::Box3 HostAxisAlignedBox3;
typedef PolygonObjectWrapper HostTriangleMesh;
typedef PolygonObjectWrapper HostPolygonMesh;
typedef C4DCurveWrapper HostCurves;
typedef CPolygon HostTriangleFace;
typedef CPolygon HostTriangleTextureFace;
typedef BaseShaderWrapper HostTextureMap;
typedef std::vector<bool> HostElementSelection;
typedef BaseBitmapWrapper HostImage;
typedef void* HostTextureAccessHandle;	// TODO
typedef TriangleMeshAccess* HostTriangleMeshAccessHandle;
typedef void* HostMeshIntersector; // Used in Maya only
typedef void* HostCurvesAccessHandle;	// TODO


inline void Invert( HostElementSelection& selection )
{
	selection.flip();
}

inline HostXform3 HostInverse( const HostXform3& xform )
{
	return ~xform;
}

inline HostVector3 HostTransform( const HostXform3& xform, const HostVector3& point )
{
	auto result = xform * point;
	return HostVector3( result );
}

Ornatrix::Xform3 GetCommonValue( const HostXform3& hostValue );
inline HostAxisAlignedBox3 HostTransform( const HostXform3& xform, const HostAxisAlignedBox3& box )
{
	const auto common = GetCommonValue( xform );
	return common * box;
}

inline HostVector3 HostBoundingBoxMin( const HostAxisAlignedBox3& box )
{
	auto result = box.pmin();
	return HostVector3( result );
}

inline HostVector3 HostBoundingBoxMax( const HostAxisAlignedBox3& box )
{
	auto result = box.pmax();
	return HostVector3( result );
}

inline void HostBoundingBoxExpand( HostAxisAlignedBox3& box, const HostAxisAlignedBox3& other )
{
	box += other;
}

inline HostVector3 CrossProd( const HostVector3& left, const HostVector3& right )
{
	auto result = Cross( left, right );
	return HostVector3( result );
}

inline float Length( const HostVector3& vector )
{
	return float( vector.GetLength() );
}


} // namespace Ephere::Plugins


namespace Ephere::Plugins::Maxon::C4D {
	
class C4DMapUser;

namespace Ornatrix {
	
	struct C4DTextureMap final : Ephere::Ornatrix::ITextureMap
	{
		explicit C4DTextureMap( C4DMapUser* mapUser );
		
		static std::shared_ptr<C4DTextureMap> Create( C4DMapUser* mapUser )
		{
			return std::make_shared<C4DTextureMap>( mapUser );
		}
		
		std::shared_ptr<Ephere::Ornatrix::IMapSampler> CreateSampler( std::vector<Ephere::Ornatrix::TextureCoordinate> textureCoordinatesAtStrandRoots, bool invertValues ) const override;
		
		C4DMapUser* mapUser_ = nullptr;
		BaseShaderWrapper* hostTextureMap_ = nullptr;
	};
	
	struct C4DMapSampler final : Ephere::Ornatrix::IMapSampler
	{
		C4DMapSampler( C4DTextureMap const& map, std::vector<Ephere::Ornatrix::TextureCoordinate> textureCoordinatesAtStrandRoots, bool invertValues );

		float GetValue( int strandIndex, float positionAlongStrand = 0.0f ) const override;

		Ephere::Ornatrix::Vector3 GetValue3( int strandIndex, float positionAlongStrand = 0.0f ) const override;

		const Ephere::Ornatrix::ITextureMap& GetMap() const override
		{
			return *map;
		}

		C4DTextureMap const* map;
		bool invertValues;
		std::vector<Ephere::Ornatrix::Vector3> perStrandValues;
//		std::map<unsigned, MImage> textureMapTiles;
	};
	
//	struct MayaTriangleMesh final : Ephere::Ornatrix::ITriangleMesh
//	{
//		explicit MayaTriangleMesh( TriangleMeshAccess& meshAccess )
//		: meshAccess( &meshAccess )
//		{
//		}
//
//		void PrepareVertexColorSet( int colorSetIndex ) override;
//
//		int GetVertexColorSetCount() const override;
//
//		Ephere::Ornatrix::Vector3 GetVertexColor( int faceIndex, Ephere::Ornatrix::Vector3 const& barycentricCoordinate, int colorSetIndex ) const override;
//
//
//		TriangleMeshAccess* meshAccess;
//	};
	
} } // namespace Ephere::Plugins::Maxon::C4D

#ifdef C4D_VERSION
#include "Ephere/NativeTools/Types.h"
EPHERE_DECLARE_TYPE_SIZE( Ephere::Plugins::HostVector3, 24 )
#endif

#elif defined(MAXON_TARGET_OSX) || defined(MAXON_TARGET_WINDOWS)
#error "Need to define MAXON_C4D"
#endif
