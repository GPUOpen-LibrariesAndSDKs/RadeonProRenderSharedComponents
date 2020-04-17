// Must compile with VC 2012

#pragma once

#include "Matrix.h"
#include "Box.h"
#include "MeshSurfacePosition.h"
#include "Ephere/NativeTools/IInterfaceProvider.h"
#include "Ephere/NativeTools/MacroTools.h"
#include "Ephere/NativeTools/Span.h"

#include <array>
#include <vector>

namespace Ephere { namespace Geometry
{
class IPolygonMesh;

const InterfaceId InterfaceId_PolygonMesh = InterfaceId_Company_Ephere + 0x200;

class IPolygonMeshSA
{
protected:
	~IPolygonMeshSA() {}

public:
	IPolygonMeshSA() {}
	IPolygonMeshSA( IPolygonMeshSA const& ) {}

	typedef Geometry::Vector3f Vector3f;

	static const InterfaceId IID = InterfaceId_PolygonMesh;

	// Vertices:

	EPHERE_NODISCARD virtual int GetVertexCount() const = 0;

	virtual void SetVertexCount( int value ) = 0;

	virtual void GetVertices( int firstIndex, int count, Vector3f* result ) const = 0;

	virtual void SetVertices( int firstIndex, int count, const Vector3f* values ) = 0;

	EPHERE_NODISCARD virtual bool IsVertexSelected( int vertexIndex ) const = 0;

	virtual void SelectVertex( int vertexIndex, bool select ) = 0;

	// TODO: Make virtual, move to implementation
	void GetVerticesByIndex( const int* vertexIndices, int count, Vector3f* result ) const
	{
		for( auto index = 0; index < count; ++index, ++result, ++vertexIndices )
		{
			GetVertices( *vertexIndices, 1, result );
		}
	}

	// Triangles:

	// TODO: This call is used sometimes to initialize the mesh to use triangles. Need to find a better way to do that and uncomment below.
	/*EPHERE_NODISCARD */virtual int GetTriangleCount() const = 0;

	virtual void GetTriangleVerticesIndices( int firstTriangleIndex, int count, std::array<int, 3>* result ) const = 0;

	EPHERE_NODISCARD virtual std::array<int, 3> GetTriangleTextureCoordinateIndices( int channelIndex, int triangleIndex ) const = 0;

	/** Gets the indices of polygons on which specified triangles are located, including the indices of the triangles on said polygons
	 * If the mesh was not triangulated or the polygon is also a triangle then the index of the triangular polygon is returned */
	virtual void GetTrianglePolygonIndices( int firstTriangleIndex, int count, std::array<int, 2>* result ) const = 0;

	// Polygons:

	EPHERE_NODISCARD virtual int GetPolygonCount() const = 0;

	virtual void SetPolygonCount( int value, int verticesPerFaceCount ) = 0;

	EPHERE_NODISCARD virtual int GetPolygonVertexCount( int polygonIndex ) const = 0;

	virtual void GetPolygonVertexIndices( int polygonIndex, int firstIndex, int count, int* result ) const = 0;

	virtual void SetPolygonVertexIndices( int polygonIndex, int firstIndex, int count, const int* vertexIndices ) = 0;

	EPHERE_NODISCARD virtual int GetPolygonTriangleCount( int polygonIndex ) const = 0;

	virtual void GetPolygonTriangleIndices( int polygonIndex, int firstIndex, int count, int* result ) const = 0;

	// Texture coordinates:

	EPHERE_NODISCARD virtual int GetTextureChannelCount() const = 0;

	virtual void SetTextureChannelCount( int value ) = 0;

	// TODO: We use this function to pre-initialize some stuff in Maya. Uncomment below when this is fixed.
	/*EPHERE_NODISCARD*/ virtual bool HasTextureChannel( int index ) const = 0;

	EPHERE_NODISCARD virtual int GetTextureCoordinateCount( int textureChannel ) const = 0;

	virtual void SetTextureCoordinateCount( int textureChannel, int count ) = 0;

	virtual void GetPolygonTextureCoordinateIndices( int channelIndex, int polygonIndex, int firstPolygonVertexIndex, int count, int* result ) const = 0;

	virtual void SetPolygonTextureVertexIndices( int textureChannel, int polygonIndex, int firstPolygonVertexIndex, int count, const int* textureCoordinateIndices ) = 0;

	virtual void GetTextureCoordinates( int channelIndex, int firstIndex, int count, TextureCoordinate* result ) const = 0;

	virtual void SetTextureCoordinates( int channelIndex, int firstIndex, int count, const TextureCoordinate* values ) = 0;

	// Face selection:

	EPHERE_NODISCARD virtual int GetSelectedFaceCount() const = 0;

	EPHERE_NODISCARD virtual bool IsFaceSelected( int faceIndex ) const = 0;

	EPHERE_NODISCARD virtual int GetFaceMaterialId( int faceIndex ) const = 0;

	virtual void SetFaceMaterialId( int faceIndex, int value ) = 0;

	EPHERE_NODISCARD virtual int GetVertexColorChannelCount() const = 0;

	virtual void GetVertexColors( int faceIndex, int channelIndex, int firstFaceVertexIndex, int count, Vector3f* result ) const = 0;

	virtual void GetVertexNormals( int firstVertexIndex, int count, Vector3f* result ) const = 0;

	virtual void PrepareVertexColorChannel( int channelIndex ) const = 0;

	EPHERE_NODISCARD virtual std::shared_ptr<void> CreateVertexNormalContext() const = 0;

	// Normals:

	EPHERE_NODISCARD virtual Vector3f GetTriangleNormalDirection( int triangleIndex ) const = 0;

	virtual void SetVertexNormal( int vertexIndex, const Vector3f& normalVector ) = 0;

	//! Must be called before GetTriangleVertexNormals
	virtual void ValidateTriangleVertexNormals() const = 0;

	virtual void GetTriangleVertexNormals( int triangleIndex, int firstVertexIndex, int vertexCount, Vector3f* result ) const = 0;

	//! Must be called before GetPolygonVertexNormals
	virtual void ValidatePolygonVertexNormals() const = 0;

	struct GetPolygonVertexNormalsContext
	{
		std::vector<int> vertexIndicesStorage;
	};

	virtual void GetPolygonVertexNormals( int polygonIndex, int firstVertexIndex, int vertexCount, Vector3f* result, GetPolygonVertexNormalsContext& context ) const = 0;

	// Bounding box:

	EPHERE_NODISCARD virtual Box3f GetBoundingBox() const = 0;

	// Other:

	virtual bool ConvertToNativeMesh( IPolygonMesh& result, bool includeGeometry = true, bool includeTopology = true, bool includeTextureCoordinates = true,
									  bool includeVertexNormals = false, bool forceTriangularFaces = false ) const = 0;

	virtual void GetSurfaceDependenciesFromPositions( const std::vector<Vector3f>& positions, std::vector<MeshSurfacePosition>& result, void* meshIntersector = nullptr ) const = 0;

	virtual void CopyFacesFromMesh( const IPolygonMeshSA& sourceMesh, int destinationFaceStartingIndex ) = 0;

	virtual Xform3f GetSurfaceTransformation( const MeshSurfacePosition& surfacePosition, const float* normalDisplacement, bool usesTopologyOrientation ) const = 0;

	struct GetSurfaceTransformationContext
	{
		GetSurfaceTransformationContext() :
			currentPolygonIndex( -1 )
		{
		}

		int currentPolygonIndex;
		std::vector<int> polygonIndicesStorage;
		std::vector<Vector3f> polygonVertexStorage;
		GetPolygonVertexNormalsContext polygonVertexNormalsContext;
	};

	virtual Xform3f GetSurfaceTransformation( const SurfacePosition& surfacePosition, const float* normalDisplacement, bool usesTopologyOrientation, GetSurfaceTransformationContext& context ) const = 0;

	// Utility:

#ifndef EPHERE_NO_UTILITIES

	EPHERE_NODISCARD Vector3f GetVertex( int vertexIndex ) const
	{
		Vector3f result;
		GetVertices( vertexIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD std::vector<Vector3f> GetVertices() const
	{
		std::vector<Vector3f> result( GetVertexCount() );
		GetVertices( 0, int( result.size() ), result.data() );
		return result;
	}

	void SetVertex( int vertexIndex, const Vector3f& value )
	{
		SetVertices( vertexIndex, 1, &value );
	}

	EPHERE_NODISCARD std::vector<Vector3f> GetVerticesVector() const
	{
		std::vector<Vector3f> result( GetVertexCount() );
		GetVertices( 0, int( result.size() ), result.data() );
		return result;
	}

	void SetVerticesVector( const std::vector<Vector3f>& values )
	{
		SetVertices( 0, int( values.size() ), values.data() );
	}

	EPHERE_NODISCARD Vector3f GetTextureCoordinate( int channelIndex, int vertexIndex ) const
	{
		Vector3f result;
		GetTextureCoordinates( channelIndex, vertexIndex, 1, &result );
		return result;
	}

	void SetTextureCoordinate( int channelIndex, int index, const TextureCoordinate& value )
	{
		SetTextureCoordinates( channelIndex, index, 1, &value );
	}

	EPHERE_NODISCARD std::array<int, 3> GetTriangleVertexIndices( int triangleIndex ) const
	{
		std::array<int, 3> result;
		GetTriangleVerticesIndices( triangleIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD std::array<Vector3f, 3> GetTriangleVertices( int triangleIndex ) const
	{
		std::array<Vector3f, 3> result;
		const auto triangleVertexIndices = GetTriangleVertexIndices( triangleIndex );
		std::transform( triangleVertexIndices.begin(), triangleVertexIndices.end(), result.begin(), [this]( int value )
		{
			Vector3f resultVector;
			GetVertices( value, 1, &resultVector );
			return resultVector;
		} );

		return result;
	}

	EPHERE_NODISCARD std::array<TextureCoordinate, 3> GetTriangleTextureCoordinates( int channelIndex, int triangleIndex ) const
	{
		const auto textureVertexIndices = GetTriangleTextureCoordinateIndices( channelIndex, triangleIndex );

		std::array<TextureCoordinate, 3> result;
		std::transform( textureVertexIndices.begin(), textureVertexIndices.end(), result.begin(), [this, channelIndex]( int value )
		{
			TextureCoordinate textureCoordinate;
			GetTextureCoordinates( channelIndex, value, 1, &textureCoordinate );
			return textureCoordinate;
		} );

		return result;
	}

	EPHERE_NODISCARD std::vector<TextureCoordinate> GetPolygonTextureCoordinates( int uvChannel, int polygonIndex ) const
	{
		const auto faceVertexCount = GetPolygonVertexCount( polygonIndex );
		std::vector<int> polygonTextureCoordinateIndices( faceVertexCount );
		GetPolygonTextureCoordinateIndices( uvChannel, polygonIndex, 0, int( polygonTextureCoordinateIndices.size() ), polygonTextureCoordinateIndices.data() );

		std::vector<TextureCoordinate> polygonTextureCoordinates( faceVertexCount );
		std::transform( polygonTextureCoordinateIndices.begin(), polygonTextureCoordinateIndices.end(), polygonTextureCoordinates.begin(), [this, uvChannel]( int value )
		{
			TextureCoordinate result;
			GetTextureCoordinates( uvChannel, value, 1, &result );
			return result;
		} );

		return polygonTextureCoordinates;
	}

	EPHERE_NODISCARD Vector3f GetTriangleVertexColor( int triangleIndex, const Vector3f& barycentricCoordinate, int channelIndex ) const
	{
		std::array<Vector3f, 3> colors;
		GetVertexColors( triangleIndex, channelIndex, 0, 3, colors.data() );
		return colors[0] * barycentricCoordinate.x() + colors[1] * barycentricCoordinate.y() + colors[2] * barycentricCoordinate.z();
	}

	EPHERE_NODISCARD Vector3f GetTriangleVertex( int triangleIndex, const Vector3f& barycentricCoordinate ) const
	{
		const auto vertices = GetTriangleVertices( triangleIndex );
		return vertices[0] * barycentricCoordinate.x() + vertices[1] * barycentricCoordinate.y() + vertices[2] * barycentricCoordinate.z();
	}

	EPHERE_NODISCARD TextureCoordinate GetTriangleTextureCoordinate( int channelIndex, int triangleIndex, const Vector3f& barycentricCoordinate ) const
	{
		const auto textureCoordinate = GetTriangleTextureCoordinates( channelIndex, triangleIndex );
		return textureCoordinate[0] * barycentricCoordinate.x() + textureCoordinate[1] * barycentricCoordinate.y() + textureCoordinate[2] * barycentricCoordinate.z();
	}

	EPHERE_NODISCARD Vector3f GetVertexNormal( int triangleIndex, const Vector3f& barycentricCoordinate, bool forceNormalized = false ) const
	{
		auto result = Vector3f::Zero();
		const auto vertexIndices = GetTriangleVertexIndices( triangleIndex );
		for( auto index = 0; index < 3; ++index )
		{
			Vector3f vertexNormal;
			GetVertexNormals( vertexIndices[index], 1, &vertexNormal );
			if( forceNormalized )
			{
				vertexNormal.normalize();
			}

			result += vertexNormal * barycentricCoordinate[index];
		}

		return result;
	}

	EPHERE_NODISCARD Vector3f GetTrianglePoint( int triangleIndex, const Vector3f& barycentricCoordinate ) const
	{
		const auto vertices = GetTriangleVertices( triangleIndex );
		return vertices[0] * barycentricCoordinate.x() + vertices[1] * barycentricCoordinate.y() + vertices[2] * barycentricCoordinate.z();
	}

	EPHERE_NODISCARD Vector3f GetVertexColor( int channelIndex, int faceIndex, int faceVertexIndex ) const
	{
		Vector3f result;
		GetVertexColors( faceIndex, channelIndex, faceVertexIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD Vector3f GetVertexColor( int channelIndex, int faceIndex, const Vector3f& barycentricCoordinate ) const
	{
		std::array<Vector3f, 3> vertexColors;
		GetVertexColors( faceIndex, channelIndex, 0, 3, vertexColors.data() );
		return GetPointFromBarycentric( vertexColors.data(), barycentricCoordinate );
	}

	EPHERE_NODISCARD Vector3f GetTriangleVertexNormal( int triangleIndex, int vertexIndex ) const
	{
		Vector3f result;
		GetTriangleVertexNormals( triangleIndex, vertexIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD std::array<Vector3f, 3> GetTriangleVertexNormals( int triangleIndex ) const
	{
		std::array<Vector3f, 3> result;
		GetTriangleVertexNormals( triangleIndex, 0, 3, result.data() );
		return result;
	}

	EPHERE_NODISCARD virtual uint64_t GetTopologyHash() const = 0;

#endif

};

} }
