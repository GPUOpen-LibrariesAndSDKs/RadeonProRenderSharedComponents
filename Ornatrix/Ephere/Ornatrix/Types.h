// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

#include "Ephere/Geometry/Native/Matrix.h"
#include "Ephere/Geometry/Native/Box.h"

#include <memory>
#include <vector>

namespace Ephere { namespace Ornatrix
{
	typedef float Real;

	typedef Geometry::Matrix<2, 1, Real> Vector2;
	typedef Geometry::Matrix<3, 1, Real> Vector3;
	typedef Geometry::Matrix<4, 1, Real> Vector4;
	typedef Geometry::Matrix<3, 4, Real> Xform3;
	typedef Geometry::Matrix<3, 3, Real> Matrix3;
	typedef Geometry::Matrix<4, 4, Real> Matrix4;

	using Geometry::TextureCoordinate;

	typedef Geometry::Box<2, Real> Box2;
	typedef Geometry::Box<3, Real> Box3;

	struct IMapSampler;

	struct ITextureMap
	{
		virtual std::shared_ptr<IMapSampler> CreateSampler( std::vector<TextureCoordinate> textureCoordinatesAtStrandRoots, bool invertValues = false ) const = 0;

	protected:
		virtual ~ITextureMap()
		{
		}
	};

	struct IMapSampler
	{
		virtual float GetValue( int strandIndex, float positionAlongStrand = 0.0f ) const = 0;

		virtual Vector3 GetValue3( int strandIndex, float positionAlongStrand = 0.0f ) const = 0;

		virtual const ITextureMap& GetMap() const = 0;

	protected:
		virtual ~IMapSampler()
		{
		}
	};

	//struct TextureMapParameter
	//{
	//	TextureMapParameter()
	//		: textureChannel( 0 ),
	//		invertValues( false )
	//	{
	//	}

	//	TextureMapParameter( std::shared_ptr<ITextureMap const> map, int textureChannel = 0, bool invertValues = false )
	//		: map( map ),
	//		textureChannel( textureChannel ),
	//		invertValues( invertValues )
	//	{
	//	}

	//	std::shared_ptr<ITextureMap const> map;
	//	int textureChannel;
	//	bool invertValues;
	//};

	struct ITriangleMesh
	{
		virtual void PrepareVertexColorSet( int colorSetIndex ) = 0;

		virtual int GetVertexColorSetCount() const = 0;

		virtual Vector3 GetVertexColor( int faceIndex, Vector3 const& barycentricCoordinate, int colorSetIndex ) const = 0;

	protected:
		virtual ~ITriangleMesh()
		{
		}
	};

} } // Ephere::Ornatrix

// For compatibility with the old code. TODO: Remove
namespace Ephere { namespace Plugins { namespace Ornatrix
{

using namespace Ephere::Ornatrix;

} } }
