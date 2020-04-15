// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Geometry/Native/Box.h"
#include "Ephere/Geometry/Native/Matrix.h"
#include "Ephere/NativeTools/IInterfaceProvider.h"
#include "Ephere/NativeTools/MacroTools.h"
#include "Ephere/Ornatrix/GuideDependency.h"
#include "Ephere/Geometry/Native/MeshSurfacePosition.h"
#include "Ephere/Ornatrix/StrandChannelName.h"
#include "Ephere/Ornatrix/StrandTopology.h"
#include "Ephere/Ornatrix/Types.h"

// transform
#include <algorithm>
// ptrdiff_t
#include <cstddef>
// wcsncpy
#include <cwchar>
// uint64_t
#include <cstdint>
// inserter
#include <iterator>
// iota
#include <numeric>
// wstring
#include <string>
#include <unordered_set>
// wstringstream
#include <sstream>

namespace Ephere { namespace Geometry
{
class IPolygonMeshSA;
} }

namespace Ephere { namespace Ornatrix
{
using Geometry::IPolygonMeshSA;
	
const InterfaceId InterfaceId_Hair = InterfaceId_Company_Ephere + 0x100;

typedef unsigned StrandId;

const StrandId StrandIdInvalid = StrandId( -1 );

/** Ornatrix hair interface class. Contains data defining a hair structure consisting of vertices, strands, and methods for manipulating said data.

A typical implementation of this interface is structured in a similar way to a polygonal mesh. It contains a set of vertices defining the geometric information
about the hair. It also contains information about how these vertices are connected to form polylines, defining the topological information about hair. Each
polyline defines a hair strand.

The topology and geometry of hair can be represented in multiple ways. For example, all vertices can be specified in hair object's local coordinates or they can
be specified in local strand coordinates and a separate per-strand transformation can be defined to bring them into the hair object's coordinates. All strands
can have the same number of points defining their polylines or each strand can have a different number of points. In the latter case information is stored to specify
how many points each strand holds.
*/
class IHair
{
public:

	/** The type of data which can be attached to elements of this class */
	enum StrandDataType
	{
		/** Each data element is attached to each strand */
		PerStrand,

		/** Each data element is attached to each vertex */
		PerVertex,

		/** There is a single global value for all strands */
		StrandDataType_Global,

		/** There is no data specified */
		StrandDataType_None
	};

	/** Coordinate space in which vertices of this class are defined */
	enum CoordinateSpace
	{
		/** Vertices are in local strand space. Per-strand transformations need to be applied to vertices to move them into object space */
		Strand,

		/** Vertices are in object space. Inverse of per-strand transformations need to be applied to vertices to move them into strand space */
		Object,
	};

	static const InterfaceId IID = InterfaceId_Hair + 3;

	virtual void CopyFrom( const IHair& source, bool copyGeometry = true, bool copyTopology = true, bool copySelection = true, bool copyTextureCoordinates = true, bool copyChannelData = true ) = 0;

	// Strands:

	/** Gets the total number of strands in this hair object
		@return Number of strands
	*/
	EPHERE_NODISCARD virtual int GetStrandCount() const = 0;

	virtual void SetStrandCount( int value ) = 0;

	// Vertices:

	EPHERE_NODISCARD virtual int GetVertexCount() const = 0;

	virtual void SetVertexCount( int value ) = 0;

	virtual bool GetVertices( int firstIndex, int count, Vector3* result, CoordinateSpace space ) const = 0;

	virtual bool SetVertices( int firstIndex, int count, const Vector3* values, CoordinateSpace space ) = 0;

	/** Gets the indices of first strands that use specified vertices
	* @param firstVertexIndex Index of first vertex for which to get the result
	* @param count Total number of result entries to get
	* @param result Destination storage
	* @return true if successful
	*/
	virtual bool GetVertexStrandIndices( int firstVertexIndex, int count, int* result ) const = 0;

	/** This function should be called before one or more calls to GetVertexStrandIndices(...). It is not thread-safe.
	* @return true if successful. If false is returned using GetVertexStrandIndices(...) is not safe.
	*/
	virtual bool ValidateVertexStrandIndices() = 0;

	// Strand points:

	virtual bool GetStrandPointCounts( int firstStrandIndex, int count, int* result ) const = 0;

	virtual bool GetStrandFirstVertexIndices( int firstStrandIndex, int count, int* result ) const = 0;

	virtual bool GetStrandPoints( int strandIndex, int firstPointIndex, int pointCount, Vector3* result, CoordinateSpace resultSpace ) const = 0;

	virtual bool SetStrandPoints( int strandIndex, int firstPointIndex, int pointCount, const Vector3* values, CoordinateSpace sourceSpace ) = 0;

	// Strand Ids:

	/** Determines if this hair has unique per-strand ids
	 * @return true if unique ids are stored
	 */
	EPHERE_NODISCARD virtual bool HasStrandIds() const = 0;

	/** Sets whether unique per-strand ids are stored in this hair
	 * @param value true to store unique ids
	 */
	virtual void SetUseStrandIds( bool value ) = 0;

	virtual bool GetStrandIds( int firstStrandIndex, int count, StrandId* result ) const = 0;

	virtual bool SetStrandIds( int firstStrandIndex, int count, const StrandId* source ) = 0;

	/** Gets indices into the strand arrays based on their unique ids */
	virtual bool GetStrandIndices( const unsigned* strandId, int count, int* result ) const = 0;

	// No need to call this one any longer
	virtual bool ValidateStrandIdsToIndices__obsolete() = 0;

	// Surface dependency:

	EPHERE_NODISCARD virtual bool HasSurfaceDependency() const = 0;

	virtual void SetUseSurfaceDependency( bool value ) = 0;

	virtual bool GetSurfaceDependencies( int firstIndex, int count, Geometry::MeshSurfacePosition* result ) const = 0;

	virtual bool SetSurfaceDependencies( int firstIndex, int count, const Geometry::MeshSurfacePosition* values ) = 0;

	//! This function will not validate the bounding box before getting it
	EPHERE_NODISCARD virtual Box3 GetBoundingBox() const = 0;

	// Guide dependency:

	EPHERE_NODISCARD virtual bool HasGuideDependency() const = 0;

	virtual void SetUsesGuideDependency( bool value ) = 0;

	virtual bool GetGuideDependencies( int firstStrandIndex, int count, GuideDependency* result ) const = 0;

	virtual bool SetGuideDependencies( int firstStrandIndex, int count, const GuideDependency* source ) = 0;

	// Strand topology:

	EPHERE_NODISCARD virtual bool HasStrandTopology() const = 0;

	virtual void SetUsesStrandTopology( bool value ) = 0;

	virtual bool GetStrandTopologies( int firstStrandIndex, int count, StrandTopology* result ) const = 0;

	virtual bool SetStrandTopologies( int firstStrandIndex, int count, const StrandTopology* source ) = 0;

	EPHERE_NODISCARD virtual int GetGlobalStrandPointCount() const = 0;

	virtual void SetGlobalStrandPointCount( int value ) = 0;

	// Strand transformations:

	EPHERE_NODISCARD virtual bool HasStrandToObjectTransforms() const = 0;

	virtual void SetUseStrandToObjectTransforms( bool value ) = 0;

	virtual bool GetStrandToObjectTransforms( int firstStrandIndex, int count, Xform3* result ) const = 0;

	virtual bool SetStrandToObjectTransforms( int firstStrandIndex, int count, const Xform3* values ) = 0;

	virtual void ValidateStrandToObjectTransforms__deprecated( bool forceStrandCoordinates = true ) = 0;

	bool ValidateStrandToObjectTransforms( const Geometry::IPolygonMeshSA* distributionMesh, bool forceStrandCoordinates = true )
	{
		return SetPropertyValues( int( CommandExtension::ValidateStrandToObjectTransforms ), forceStrandCoordinates ? 1 : 0, 0, distributionMesh );
	}

	// Strand rotations/twists:

	EPHERE_NODISCARD virtual bool HasStrandRotations() const = 0;

	virtual void SetUseStrandRotations( bool value ) = 0;

	// TODO: Deprecate in favor of function providing a data type
	virtual bool GetStrandRotations( int firstStrandIndex, int count, float* result ) const = 0;

	// TODO: Deprecate in favor of function providing a data type
	virtual bool SetStrandRotations( int firstStrandIndex, int count, const float* values ) = 0;

	/** Gets the current type of data storage for strand width values */
	EPHERE_NODISCARD StrandDataType GetRotationsDataType() const
	{
		int result;
		GetPropertyValues( int( CommandExtension::RotationsStrandDataType ), 0, 0, &result );
		return StrandDataType( result );
	}

	struct GetStrandRotationsResultStruct
	{
		float* result;
		StrandDataType dataType;
	};

	// TODO: Add to IHair
	bool GetStrandRotations( int firstElementIndex, int count, float* result, StrandDataType dataType ) const
	{
		GetStrandRotationsResultStruct resultStruct = { result, dataType };
		return GetPropertyValues( int( CommandExtension::Rotations ), firstElementIndex, count, &resultStruct );
	}

	struct SetStrandRotationsValuesStruct
	{
		const float* values;
		StrandDataType dataType;
	};

	// TODO: Add to IHair
	bool SetStrandRotations( int firstElementIndex, int count, const float* values, StrandDataType dataType )
	{
		const SetStrandRotationsValuesStruct valuesStruct = { values, dataType };
		return SetPropertyValues( int( CommandExtension::Rotations ), firstElementIndex, count, &valuesStruct );
	}

	// Vertex widths (strand diameters):

	EPHERE_NODISCARD virtual bool HasWidths() const = 0;

	virtual void SetUseWidths( bool value ) = 0;

	virtual bool GetWidths( int firstVertexIndex, int count, float* result ) const = 0;

	virtual bool SetWidths( int firstVertexIndex, int count, const float* values ) = 0;

	// Coordinate space:

	EPHERE_NODISCARD virtual CoordinateSpace GetCoordinateSpace() const = 0;

	virtual void SetCoordinateSpace( CoordinateSpace value ) const = 0;

	// Texture coordinates:

	EPHERE_NODISCARD virtual int GetTextureCoordinateChannelCount() const = 0;

	virtual void SetTextureCoordinateChannelCount( int value ) = 0;

	EPHERE_NODISCARD virtual StrandDataType GetTextureCoordinateDataType( int channelIndex ) const = 0;

	virtual bool GetTextureCoordinates( int channelIndex, int firstIndex, int count, TextureCoordinate* result, StrandDataType dataType ) const = 0;

	virtual bool SetTextureCoordinates( int channelIndex, int firstIndex, int count, const TextureCoordinate* values, StrandDataType dataType ) = 0;

	// Strand channels:

	EPHERE_NODISCARD virtual int GetStrandChannelCount( StrandDataType type ) const = 0;

	virtual bool GetStrandChannelNames( StrandDataType type, int channelIndex, int count, StrandChannelName *result ) const = 0;

	virtual bool GetStrandChannelData( StrandDataType type, int channelIndex, int firstElementIndex, int count, float* result ) const = 0;

	virtual void SetStrandChannelCount( StrandDataType type, int count ) = 0;

	virtual bool SetStrandChannelData( StrandDataType type, int channelIndex, int firstElementIndex, int count, const float* source ) = 0;

	virtual bool SetStrandChannelNames( StrandDataType type, int channelIndex, int count, const StrandChannelName* source ) = 0;

	virtual bool DeleteStrandChannels( StrandDataType type, const int* indices, int indexCount ) = 0;

	// Selection, hiding, and freezing:

	EPHERE_NODISCARD virtual int GetSelectedStrandCount() const = 0;

	virtual bool GetSelectedStrandIds( int startIndex, int count, StrandId* result ) const = 0;

	virtual bool SetSelectedStrandIds( const StrandId* values, int count ) = 0;

	EPHERE_NODISCARD virtual int GetHiddenStrandCount() const = 0;

	virtual bool GetHiddenStrandIds( int startIndex, int count, StrandId* result ) const = 0;

	virtual bool SetHiddenStrandIds( const StrandId* values, int count ) = 0;

	EPHERE_NODISCARD virtual int GetFrozenStrandCount() const = 0;

	virtual bool GetFrozenStrandIds( int startIndex, int count, StrandId* result ) const = 0;

	virtual bool SetFrozenStrandIds( const StrandId* values, int count ) = 0;

	// Caching:

	virtual void InvalidateGeometryCache() = 0;

	EPHERE_NODISCARD virtual std::uint64_t GetTopologyHash() const = 0;

	// Future expansion (these will be used to add new functionality without modifying the virtual signature of this interface):

	virtual bool Execute( int commandIndex ) = 0;

	EPHERE_NODISCARD virtual bool HasProperty( int propertyIndex ) const = 0;

	virtual void SetUsesProperty( int propertyIndex ) = 0;

	virtual bool GetPropertyValues( int propertyIndex, int firstElementIndex, int count, void* values ) const = 0;

	virtual bool SetPropertyValues( int propertyIndex, int firstElementIndex, int count, const void* values ) = 0;

	// Groups:

	EPHERE_NODISCARD virtual bool HasStrandGroups() const = 0;

	virtual void SetUsesStrandGroups( bool value ) = 0;

	virtual bool GetStrandGroups( int firstStrandIndex, int count, int* result ) const = 0;

	virtual bool SetStrandGroups( int firstStrandIndex, int count, const int* values ) = 0;

	// Deletion:

	virtual bool DeleteStrands( const StrandId* strandIds, int count ) = 0;

	// Utility functions:
	// The following functions are inline and defined for providing common useful operations on this class.

	EPHERE_NODISCARD bool IsEmpty() const
	{
		return GetStrandCount() == 0;
	}

	/** Removes all data from this object, returning it to initial state */
	void Clear()
	{
		SetStrandCount( 0 );
		SetVertexCount( 0 );
		SetStrandChannelCount( StrandDataType::PerStrand, 0 );
		SetStrandChannelCount( StrandDataType::PerVertex, 0 );
		SetTextureCoordinateChannelCount( 0 );
	}

	EPHERE_NODISCARD StrandId GetStrandId( const int strandIndex ) const
	{
		if( HasStrandIds() )
		{
			StrandId result;
			if( GetStrandIds( strandIndex, 1, &result ) )
			{
				return result;
			}
		}

		return StrandId( strandIndex );
	}

	EPHERE_NODISCARD Xform3 GetStrandToObjectTransform( const int strandIndex ) const
	{
		Xform3 result;
		if( GetStrandToObjectTransforms( strandIndex, 1, &result ) )
		{
			return result;
		}

		return Xform3::Identity();
	}

	void SetStrandToObjectTransform( const int strandIndex, const Xform3& value )
	{
		SetStrandToObjectTransforms( strandIndex, 1, &value );
	}

	int GetChannelIndex( const StrandDataType type, const wchar_t* channelName ) const
	{
		const auto channelCount = GetStrandChannelCount( type );
		if( channelCount == 0 )
		{
			return -1;
		}

		std::vector<StrandChannelName> channelNames( channelCount );
		GetStrandChannelNames( type, 0, channelCount, channelNames.data() );

		const std::wstring channelNameString( channelName );
		for( auto channelIndex = 0; channelIndex < channelCount; ++channelIndex )
		{
			if( channelNameString == channelNames[channelIndex].name )
			{
				return channelIndex;
			}
		}

		return -1;
	}

	EPHERE_NODISCARD float GetStrandRotation( const int strandIndex ) const
	{
		float result;
		if( GetStrandRotations( strandIndex, 1, &result, StrandDataType::PerStrand ) )
		{
			return result;
		}

		return 0.0f;
	}

	bool SetStrandRotation( const int strandIndex, const float value )
	{
		return SetStrandRotations( strandIndex, 1, &value, StrandDataType::PerStrand );
	}

	EPHERE_NODISCARD Geometry::MeshSurfacePosition GetSurfaceDependency( const int strandIndex ) const
	{
		auto result = Geometry::MeshSurfacePosition();
		GetSurfaceDependencies( strandIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD Geometry::SurfacePosition GetSurfaceDependency2( const int strandIndex ) const
	{
		auto result = Geometry::SurfacePosition();
		GetSurfaceDependencies2( strandIndex, 1, &result );
		return result;
	}

	void SetSurfaceDependency( const int strandIndex, const Geometry::MeshSurfacePosition& value )
	{
		SetSurfaceDependencies( strandIndex, 1, &value );
	}

	EPHERE_NODISCARD GuideDependency GetGuideDependency( const int strandIndex ) const
	{
		auto result = GuideDependency();
		GetGuideDependencies( strandIndex, 1, &result );
		return result;
	}

	void SetGuideDependency( const int strandIndex, const GuideDependency& value )
	{
		SetGuideDependencies( strandIndex, 1, &value );
	}

	EPHERE_NODISCARD float GetStrandChannelData( const int channelIndex, const int strandIndex ) const
	{
		float result;
		if( GetStrandChannelData( PerStrand, channelIndex, strandIndex, 1, &result ) )
		{
			return result;
		}

		return 0.0f;
	}

	EPHERE_NODISCARD float GetVertexChannelData( const int channelIndex, const int vertexIndex ) const
	{
		float result;
		if( GetStrandChannelData( PerVertex, channelIndex, vertexIndex, 1, &result ) )
		{
			return result;
		}

		return 0.0f;
	}

	void SetStrandChannelData( const int channelIndex, const int strandIndex, const float value )
	{
		SetStrandChannelData( PerStrand, channelIndex, strandIndex, 1, &value );
	}

	void SetVertexChannelData( const int channelIndex, const int vertexIndex, const float value )
	{
		SetStrandChannelData( PerVertex, channelIndex, vertexIndex, 1, &value );
	}

	EPHERE_NODISCARD int GetVertexStrandIndex( int vertexIndex ) const
	{
		int result;
		GetVertexStrandIndices( vertexIndex, 1, &result );
		return result;
	}

	EPHERE_NODISCARD int GetStrandPointCount( const int strandIndex ) const
	{
		int result;
		if( GetStrandPointCounts( strandIndex, 1, &result ) )
		{
			return result;
		}

		return -1;
	}

	EPHERE_NODISCARD int GetStrandFirstVertexIndex( const int strandIndex ) const
	{
		int result;
		if( GetStrandFirstVertexIndices( strandIndex, 1, &result ) )
		{
			return result;
		}

		return -1;
	}

	void SetStrandChannelData( const int channelIndex, const int strandIndex, const int pointIndex, const float value )
	{
		const auto firstVertexIndex = GetStrandFirstVertexIndex( strandIndex );
		SetStrandChannelData( PerVertex, channelIndex, firstVertexIndex + pointIndex, 1, &value );
	}

	EPHERE_NODISCARD float GetStrandChannelData( const int channelIndex, const int strandIndex, const int pointIndex ) const
	{
		const auto firstVertexIndex = GetStrandFirstVertexIndex( strandIndex );
		float result;
		if( GetStrandChannelData( PerVertex, channelIndex, firstVertexIndex + pointIndex, 1, &result ) )
		{
			return result;
		}

		return 0.0f;
	}

	void SetStrandChannelAllData( const StrandDataType dataType, const int firstChannelIndex, int count, const float value )
	{
		const auto elementCount = dataType == PerStrand ? GetStrandCount() : GetVertexCount();

		if( count == -1 )
		{
			count = GetStrandChannelCount( dataType ) - firstChannelIndex;
		}

		std::vector<float> channelData( elementCount, value );

		const auto lastChannelIndex = firstChannelIndex + count;
		for( auto channelIndex = firstChannelIndex; channelIndex < lastChannelIndex; ++channelIndex )
		{
			SetStrandChannelData( dataType, channelIndex, 0, int( channelData.size() ), channelData.data() );
		}
	}

	/** Get strand indices if they are present, or otherwise copy the strand ids as indices */
	bool GetStrandIndicesOrIds( const StrandId* strandIds, const int count, int* result ) const
	{
		if( HasStrandIds() )
		{
			return GetStrandIndices( strandIds, count, result );
		}

		std::transform( strandIds, strandIds + count, result, []( StrandId strandId ) { return int( strandId ); } );
		return true;
	}

	EPHERE_NODISCARD int GetStrandIndexOrId( StrandId strandId ) const
	{
		int result;
		return GetStrandIndicesOrIds( &strandId, 1, &result ) ? result : -1;
	}

	bool GetStrandIdsOrIndices( const int firstStrandIndex, const int count, StrandId* result ) const
	{
		if( HasStrandIds() )
		{
			return GetStrandIds( firstStrandIndex, count, result );
		}

		// Fill with incremented integers
		std::iota( result, result + count, firstStrandIndex );
		return true;
	}

	bool GetStrandIdsOrIndices( std::vector<StrandId>& result ) const
	{
		result.resize( GetStrandCount() );
		if( HasStrandIds() )
		{
			return GetStrandIds( 0, int( result.size() ), result.data() );
		}

		// Fill with incremented integers
		std::iota( result.begin(), result.end(), 0 );
		return true;
	}

	EPHERE_NODISCARD Vector3 GetStrandPoint( const int strandIndex, const int pointIndex, const CoordinateSpace space ) const
	{
		auto result = Vector3();
		GetStrandPoints( strandIndex, pointIndex, 1, &result, space );
		return result;
	}

	EPHERE_NODISCARD std::vector<Vector3> GetStrandPoints( const int strandIndex, const CoordinateSpace space ) const
	{
		std::vector<Vector3> result( GetStrandPointCount( strandIndex ) );
		GetStrandPoints( strandIndex, 0, int( result.size() ), result.data(), space );
		return result;
	}

	bool SetStrandPoints( const int strandIndex, const std::vector<Vector3>& values, const CoordinateSpace space )
	{
		return SetStrandPoints( strandIndex, 0, int( values.size() ), values.data(), space );
	}

	void SetStrandPoint( const int strandIndex, const int pointIndex, const Vector3& value, const CoordinateSpace space )
	{
		SetStrandPoints( strandIndex, pointIndex, 1, &value, space );
	}

	EPHERE_NODISCARD Vector3 GetVertex( const int vertexIndex, const CoordinateSpace space ) const
	{
		auto result = Vector3();
		GetVertices( vertexIndex, 1, &result, space );
		return result;
	}

	void SetVertex( const int vertexIndex, const Vector3& value, const CoordinateSpace space )
	{
		SetVertices( vertexIndex, 1, &value, space );
	}

	EPHERE_NODISCARD std::vector<Vector3> GetVertices( CoordinateSpace space ) const
	{
		return GetVerticesVector( 0, int( GetVertexCount() ), space );
	}

	EPHERE_NODISCARD std::vector<Vector3> GetVerticesVector( int firstIndex, int count, CoordinateSpace space ) const
	{
		std::vector<Vector3> result( count == -1 ? GetVertexCount() : count );
		GetVertices( firstIndex, int( result.size() ), result.data(), space );
		return result;
	}

	// TODO: If this gets enough usage we should consider moving it into implementation
	void TransformVertices( int firstIndex, int count, CoordinateSpace space, const Xform3& transform )
	{
		if( count == 0 )
		{
			return;
		}

		std::vector<Vector3> vertices( count );
		GetVertices( firstIndex, int( vertices.size() ), vertices.data(), space );
		std::transform( begin( vertices ), end( vertices ), begin( vertices ), [&transform]( const Vector3& value )
		{
			return transform * value;
		} );

		SetVertices( firstIndex, int( vertices.size() ), vertices.data(), space );
	}

	EPHERE_NODISCARD std::vector<StrandId> GetStrandIds() const
	{
		std::vector<StrandId> result( GetStrandCount() );
		GetStrandIds( 0, int( result.size() ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::vector<int> GetStrandGroups() const
	{
		const auto strandCount = GetStrandCount();

		std::vector<int> result( strandCount );

		if( !GetStrandGroups( 0, strandCount, &result[0] ) )
		{
			result.clear();
		}

		return result;
	}

	void GetNonSelectedStrandIdSet( std::vector<StrandId>& result ) const
	{
		std::vector<StrandId> selectedStrandIds( GetSelectedStrandCount() ), allStrandIds( GetStrandCount() );
		GetSelectedStrandIds( 0, int( selectedStrandIds.size() ), selectedStrandIds.data() );
		GetStrandIdsOrIndices( 0, int( allStrandIds.size() ), allStrandIds.data() );

		std::sort( selectedStrandIds.begin(), selectedStrandIds.end() );
		std::sort( allStrandIds.begin(), allStrandIds.end() );

		std::set_difference( allStrandIds.begin(), allStrandIds.end(), selectedStrandIds.begin(), selectedStrandIds.end(), std::inserter( result, result.begin() ) );
	}

	void GetSelectedStrandIdSet( std::vector<StrandId>& result ) const
	{
		result.resize( GetSelectedStrandCount() );
		GetSelectedStrandIds( 0, int( result.size() ), result.data() );
	}

	void GetHiddenStrandIdSet( std::vector<StrandId>& result ) const
	{
		result.resize( GetHiddenStrandCount() );
		GetHiddenStrandIds( 0, int( result.size() ), result.data() );
	}

	void GetFrozenStrandIdSet( std::vector<StrandId>& result ) const
	{
		result.resize( GetFrozenStrandCount() );
		GetFrozenStrandIds( 0, int( result.size() ), result.data() );
	}

	template <class TContainer>
	void GetSelectedStrandIdSet( TContainer& result ) const
	{
		std::vector<StrandId> strandIds;
		GetSelectedStrandIdSet( strandIds );

		result.clear();
		result.insert( strandIds.begin(), strandIds.end() );
	}

	template <class TContainer>
	void SetSelectedStrandIdSet( const TContainer& values )
	{
		std::vector<StrandId> strandIdArray( values.begin(), values.end() );
		SetSelectedStrandIds( strandIdArray.data(), int( strandIdArray.size() ) );
	}

	template <class TContainer>
	void GetHiddenStrandIdSet( TContainer& result, bool overwriteResult = true ) const
	{
		std::vector<StrandId> strandIds;
		GetHiddenStrandIdSet( strandIds );

		if( overwriteResult )
		{
			result.clear();
		}
		
		result.insert( strandIds.begin(), strandIds.end() );
	}

	template <class TContainer>
	void SetHiddenStrandIdSet( const TContainer& values )
	{
		std::vector<StrandId> strandIdArray( values.begin(), values.end() );
		SetHiddenStrandIds( strandIdArray.data(), int( strandIdArray.size() ) );
	}

	template <class TContainer>
	void GetFrozenStrandIdSet( TContainer& result, bool overwriteResult = true ) const
	{
		std::vector<StrandId> strandIds;
		GetFrozenStrandIdSet( strandIds );

		if( overwriteResult )
		{
			result.clear();
		}
		
		result.insert( strandIds.begin(), strandIds.end() );
	}

	template <class TContainer>
	void SetFrozenStrandIdSet( const TContainer& values )
	{
		std::vector<StrandId> strandIdArray( values.begin(), values.end() );
		SetFrozenStrandIds( strandIdArray.data(), int( strandIdArray.size() ) );
	}

	//! Utility method for legacy scripted commands. These are used mostly for unit testing so performance isn't very important.
	EPHERE_NODISCARD std::vector<bool> GetStrandSelection() const
	{
		std::vector<bool> result;
		std::unordered_set<StrandId> selectedStrandIds;
		GetSelectedStrandIdSet( selectedStrandIds );
		std::vector<StrandId> strandIds( GetStrandCount() );
		GetStrandIds( 0, int( strandIds.size() ), strandIds.data() );

		result.reserve( GetStrandCount() );
		for( auto& strandId : strandIds )
		{
			result.push_back( selectedStrandIds.find( strandId ) != selectedStrandIds.end() );
		}

		return result;
	}

	//! Utility method for legacy scripted commands. These are used mostly for unit testing so performance isn't very important.
	EPHERE_NODISCARD std::vector<bool> GetStrandVisibility() const
	{
		std::vector<bool> result;
		std::unordered_set<StrandId> selectedStrandIds;
		GetHiddenStrandIdSet( selectedStrandIds );
		std::vector<StrandId> strandIds( GetStrandCount() );
		GetStrandIds( 0, int( strandIds.size() ), strandIds.data() );

		result.reserve( GetStrandCount() );
		for( auto strandIdIter = strandIds.begin(); strandIdIter != strandIds.end(); ++strandIdIter )
		{
			result.push_back( selectedStrandIds.find( *strandIdIter ) == selectedStrandIds.end() );
		}

		return result;
	}

	EPHERE_NODISCARD std::vector<TextureCoordinate> GetStrandTextureCoordinates( int channelIndex ) const
	{
		std::vector<TextureCoordinate> result( GetStrandCount() );
		if( !GetTextureCoordinates( channelIndex, 0, int( result.size() ), result.data(), PerStrand ) )
		{
			fill( begin( result ), end( result ), TextureCoordinate::Zero() );
		}

		return result;
	}

	EPHERE_NODISCARD TextureCoordinate GetStrandTextureCoordinate( int channelIndex, int strandIndex ) const
	{
		TextureCoordinate result;
		if( GetTextureCoordinates( channelIndex, strandIndex, 1, &result, PerStrand ) )
		{
			return result;
		}

		return TextureCoordinate::Zero();
	}

	void SetUniqueStrandChannelName( StrandDataType type, int channelIndex, const wchar_t* baseName )
	{
		const auto uniqueChannelNameString = GetUniqueStrandChannelName( type, baseName );
		StrandChannelName channelName;
		std::wcsncpy( channelName.name, uniqueChannelNameString.c_str(), StrandChannelName::MaximumNameLength );
		// TODO: what if uniqueChannelNameString is longer than MaximumNameLength? The name won't be unique
		channelName.name[StrandChannelName::MaximumNameLength - 1] = 0;

		SetStrandChannelNames( type, channelIndex, 1, &channelName );
	}

	/** Given a root name, generates a name for a strand channel which is not currently present. Integers are added at the end of the name
	to make it unique */
	EPHERE_NODISCARD std::wstring GetUniqueStrandChannelName( StrandDataType type, const wchar_t* nameRoot ) const
	{
		const auto channelCount = GetStrandChannelCount( type );
		if( channelCount == 0 )
		{
			return nameRoot;
		}

		std::vector<StrandChannelName> channelNames( channelCount );
		GetStrandChannelNames( type, 0, channelCount, channelNames.data() );

		std::unordered_set<std::wstring> channelNamesSet;
		for( const auto& channelName: channelNames )
		{
			channelNamesSet.insert( channelName.name );
		}

		std::wstringstream stringStream;
		stringStream << nameRoot;

		auto index = 1;
		while( channelNamesSet.find( stringStream.str() ) != channelNamesSet.end() )
		{
			stringStream.str( std::wstring() );
			stringStream << nameRoot;
			stringStream << index++;
		}

		return stringStream.str();
	}

	EPHERE_NODISCARD std::vector<StrandId> GetSelectedStrandIds() const
	{
		std::vector<StrandId> result( GetSelectedStrandCount() );
		GetSelectedStrandIds( 0, int( result.size() ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::vector<int> GetSelectedStrandIndices() const
	{
		std::vector<StrandId> result( GetSelectedStrandCount() );
		GetSelectedStrandIds( 0, int( result.size() ), result.data() );

		std::vector<int> resultIndices( result.size() );
		GetStrandIndicesOrIds( result.data(), int( result.size() ), resultIndices.data() );
		const auto end = std::remove_if( resultIndices.begin(), resultIndices.end(), []( int index )
		{
			return index <= 0;
		} );
		resultIndices.erase( end, resultIndices.end() );
		return resultIndices;
	}

	EPHERE_NODISCARD std::vector<float> GetWidthsVector() const
	{
		std::vector<float> result( GetVertexCount() );
		GetWidths( 0, int( result.size() ), result.data() );
		return result;
	}

	EPHERE_NODISCARD std::vector<float> GetStrandWidths( int strandIndex ) const
	{
		std::vector<float> result( GetStrandPointCount( strandIndex ) );
		GetWidths( GetStrandFirstVertexIndex( strandIndex ), int( result.size() ), result.data() );
		return result;
	}

	/** Get the names of root channels
	*  @return List of root channel names
	*/
	EPHERE_NODISCARD std::vector<std::wstring> GetRootChannels() const
	{
		std::vector<StrandChannelName> channelNames( GetStrandChannelCount( IHair::PerStrand ) );
		GetStrandChannelNames( IHair::PerStrand, 0, int( channelNames.size() ), channelNames.data() );

		std::vector<std::wstring> result( channelNames.size() );
		std::transform( channelNames.begin(), channelNames.end(), result.begin(), []( const StrandChannelName& value )
		{
			return std::wstring( value.name );
		} );

		return result;
	}

	/** Get the names of vertex channels
	*  @return List of vertex channel names
	*/
	EPHERE_NODISCARD std::vector<std::wstring> GetVertexChannels() const
	{
		std::vector<StrandChannelName> channelNames( GetStrandChannelCount( IHair::PerVertex ) );
		GetStrandChannelNames( IHair::PerVertex, 0, int( channelNames.size() ), channelNames.data() );

		std::vector<std::wstring> result( channelNames.size() );
		std::transform( channelNames.begin(), channelNames.end(), result.begin(), []( const StrandChannelName& value )
		{
			return std::wstring( value.name );
		} );

		return result;
	}

	// Extension functions, move to IHair2 later:

	enum class CommandExtension
	{
		UseGlobalSegmentTransformOrientation,
		GetGuideDependencies2TotalCount,
		GetGuideDependencies2,
		SetGuideDependencies2,
		GetRootPositionsInObjectSpace,
		RotationsStrandDataType,
		Rotations,
		ValidateStrandToObjectTransforms,
		DeleteStrandsByIndices,
		SurfaceDependency2,
		SurfaceDependency2Off
	};

	EPHERE_NODISCARD bool UseGlobalSegmentTransformOrientation() const
	{
		return HasProperty( int( CommandExtension::UseGlobalSegmentTransformOrientation ) );
	}

	// Guide dependency:

	/* New guide dependency functions with variable guide count per strand.

	These 3 new functions should be used to access guide dependencies in new applications instead
	of 4 old functions:
	* GetGuideDependencies and SetGuideDependencies
	* GetGuideDependency and SetGuideDependency

	Concurrency notes:

	It's not thread-safe to set strand guide dependencies because each strand can have arbitrary
	count of guide dependencies and change of this count may affect internal representation
	of other guide dependencies. See SetGuideDependencies2 function description for more details.

	Old SetGuideDependencies and SetGuideDependency functions can still be used concurrently with
	other old functions called for non-overlapping strand ranges but now this behavior is result
	of usage of internally created critical section.
	*/

	/** Gets total count of guide dependencies for specified strand range.

	Each strand can have arbitrary count of guide dependencies. */
	bool GetGuideDependencies2TotalCount( int firstStrandIndex, int count, unsigned& result ) const
	{
		return GetPropertyValues( int( CommandExtension::GetGuideDependencies2TotalCount ), firstStrandIndex, count, &result );
	}

	/** Gets guide dependencies with variable guide count per strand for specified strand range. */
	bool GetGuideDependencies2( int firstStrandIndex, int count, unsigned* indicesResult, int maxTotalGuideCount, GuideDependency2* guidesResult ) const
	{
		void* values[] = { indicesResult, reinterpret_cast<void*>( std::ptrdiff_t( maxTotalGuideCount ) ), guidesResult };
		return GetPropertyValues( int( CommandExtension::GetGuideDependencies2 ), firstStrandIndex, count, values );
	}

	/** Sets guide dependencies with variable guide count per strand for specified strand range.

	Concurrency notes:

	In general this function is not thread-safe and can not be used concurrently with any guide dependency functions.
	The only specific case when it can be used concurrently is when total guide count for specified strands is not changed.
	In this case only guide dependencies for specified strand range are accessed and guide dependency functions can be
	concurrently called for non-overlapping strand ranges.
	*/
	bool SetGuideDependencies2( int firstStrandIndex, int count, const unsigned* indicesSource, int totalGuideCount, const GuideDependency2* guidesSource )
	{
		const void* values[] = { indicesSource, reinterpret_cast<void*>( std::ptrdiff_t( totalGuideCount ) ), guidesSource };
		return SetPropertyValues( int( CommandExtension::SetGuideDependencies2 ), firstStrandIndex, count, values );
	}

	/** Retrieves the positions of first points of each strand in object space
	*/
	bool GetRootPositions( int firstStrandIndex, int count, Vector3* result, CoordinateSpace /*resultSpace*/ ) const
	{
		return GetPropertyValues( int( CommandExtension::GetRootPositionsInObjectSpace ), firstStrandIndex, count, result );
	}

	/** Deletes strands specified by their indices
	 * @param strandIndices Indices of strands to delete, sorted from smallest to largest
	 * @param count Number of strands to delete
	 * @return true if strands were deleted
	 */
	bool DeleteStrandsByIndices( const int* strandIndices, int count )
	{
		return SetPropertyValues( int( CommandExtension::DeleteStrandsByIndices ), 0, count, strandIndices );
	}

	EPHERE_NODISCARD bool HasSurfaceDependency2() const
	{
		return HasProperty( int( CommandExtension::SurfaceDependency2 ) );
	}

	void SetUseSurfaceDependency2( bool value )
	{
		SetUsesProperty( int( value ? CommandExtension::SurfaceDependency2 : CommandExtension::SurfaceDependency2Off ) );
	}

	bool GetSurfaceDependencies2( int firstStrandIndex, int count, Geometry::SurfacePosition* result ) const
	{
		return GetPropertyValues( int( CommandExtension::SurfaceDependency2 ), firstStrandIndex, count, result );
	}

	bool SetSurfaceDependencies2( int firstStrandIndex, int count, const Geometry::SurfacePosition* values )
	{
		return SetPropertyValues( int( CommandExtension::SurfaceDependency2 ), firstStrandIndex, count, values );
	}

	IHair& operator=( const IHair& other )
	{
		CopyFrom( other, true, true, true, true, true );
		return *this;
	}

	static bool CheckPointNaN( const Vector3& value )
	{
		return value.x() == value.x() && value.y() == value.y() && value.z() == value.z();
	}

	static bool CheckPoints( const Vector3* values, const int count )
	{
		for( auto index = 0; index < count; ++index )
		{
			if( !CheckPointNaN( values[index] ) )
			{
				return false;
			}
		}

		return true;
	}

protected:

	/** Default constructor */
	IHair()
	{}

	/** Default copy constructor */
	IHair( const IHair& )
	{}

	/** Default move constructor */
	IHair( IHair&& )
	{}

	/** Default destructor */
	~IHair()
	{}

	/** Default move assignment operator */
	IHair& operator=( IHair&& )
	{
		return *this;
	}
};

} } // Ephere::Ornatrix
