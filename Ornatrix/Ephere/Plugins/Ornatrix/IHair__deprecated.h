// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

// Needs to be first because it includes the host headers, and they cause problems if not included first
#include "Ephere/Plugins/CommonTypes.h"

#include "Ephere/Ornatrix/IHair.h"
#include "Ephere/Ornatrix/StrandChannelType.h"

#include "Ephere/Plugins/Ornatrix/SurfaceDependency.h"

namespace Ephere { namespace Plugins { namespace Ornatrix
{

using namespace Ephere::Ornatrix;

// forward declarations:
struct SurfaceDependency;
class IGuides;

// Old deprecated interfaces (use IHair instead):

/** Ornatrix hair interface class. Contains data defining a hair structure consisting of strands and methods for manipulating said data.

A typical implementation of this interface is structured in a similar way to a polygonal mesh. It contains a set of vertices defining the geometric information
about the hair. It also contains information about how these vertices are connected to form polylines, defining the topological information about hair. Each
polyline defines a hair strand.

The topology and geometry of hair can be represented in multiple ways. For example, all vertices can be specified in hair object's local coordinates or they can
be specified in local strand coordinates and a separate per-strand transformation can be defined to bring them into the hair object's coordinates. All strands
can have the same number of points defining their polylines or each strand can have a different number of points. In the latter case information is stored to specify
how many points each strand holds.
*/
class IHair__deprecated
{
public:

	static const InterfaceId IID = InterfaceId_Hair + 1;

	/** Copy all hair information from another instance
	@param source Another hair object from which to copy data into this one
	*/
	virtual void CopyFrom__deprecated( const IHair__deprecated& source, bool copyGeometry = true, bool copyTopology = true, bool copySelection = true, bool copyChannelData = true ) = 0;

	/** Creates a copy of this object
	@bool copyGeometry When true geometry information of this object is cloned
	@bool copyTopology When true topology information of this object is cloned
	@return Copy of this object
	*/
	virtual IHair__deprecated* Clone( bool copyGeometry = true, bool copyTopology = true, bool copySelection = true, bool copyChannelData = true ) const = 0;

	//! Deletes all data used by this class
	virtual void Clear() = 0;

	// Strand count:

	/** Gets the total number of strands in this hair object
	@return Number of strands
	*/
	virtual unsigned GetStrandCount__deprecated() const = 0;

	/** Sets the number of strands in this hair object and updates all associated data
	@param count Number of strands to set
	@param keep If true previous data is kept, otherwise it is deleted
	*/
	virtual void SetStrandCount__deprecated( unsigned count, bool keep = false ) = 0;

	// Strand topology:

	/** Gets whether strand topology information is used
	When true, information is stored per each strand specifying which vertex the strand's polyline starts from and how many points define the strand.
	When false, global per-strand point count is used in this hair object.
	@return true if each strands in this structure can have a strands to have varying vertex counts
	@see StrandTopology
	*/
	virtual bool UsesStrandTopology() const = 0;

	/** Sets whether to use same vertex count for all strands (if false) or have ability for strands to have varying vertex count (if true)
	@see UsesStrandTopology()
	@param on true to have ability for strands to have varying vertex count
	*/
	virtual void UsesStrandTopology( bool on ) = 0;

	/** Gets topological information for specified strand
	@see UsesStrandTopology()
	@param strandIndex Index of strand
	@return Strand topology structure
	*/
	virtual const StrandTopology& GetStrandTopology( unsigned strandIndex ) const = 0;

	/** Sets topological information for specified strand
	@see UsesStrandTopology()
	@param strandIndex Index of strand
	@param value Strand topology structure
	*/
	virtual void SetStrandTopology( unsigned strandIndex, const StrandTopology& value ) = 0;

	// Per-strand transformations:

	/** Gets whether each strand is located in strand space and has a per-strand transform matrix to put it into object space.
	@return true if strands use local strand space
	*/
	virtual bool UsesPerStrandTransformations() const = 0;

	/** Sets whether each strand is located in strand space and has a per-strand transform matrix to put it into object space.
	@see UsesPerStrandTransformations()
	@param on true if strands use local strand space
	*/
	virtual void UsesPerStrandTransformations( bool on ) = 0;

	/** Gets the transformation applied to specified strand's points to bring them into object space
	@see UsesPerStrandTransformations()
	@param strandIndex Strand index
	@return Transform associated with root
	*/
	virtual HostXform3 GetStrandTransform( unsigned strandIndex ) const = 0;

	/** Sets the transformation applied to specified strand's points to bring them into object space
	@see UsesPerStrandTransformations()
	@param strandIndex Index of strand
	@param value Matrix to set
	*/
	virtual void SetStrandTransform( unsigned strandIndex, const HostXform3& value ) = 0;

	/** Sets strand transformations from the specified array of transforms
	This method could be faster than setting transforms one by one by utilizing copying of an array of values
	@param source Array of transforms to set
	@param count Number of values in source
	@param destinationStartIndex Index of first element in this object's transform array to set
	*/
	virtual void SetStrandTransforms( const HostXform3* source, unsigned count, unsigned destinationStartIndex = 0 ) = 0;

	/** Updates strand transforms using surface dependency and distribution mesh normals
	@param startIndex Start root index
	@param count Number of roots to update
	*/
	virtual void UpdateStrandTransformationsFromDistributionMesh__deprecated( unsigned startIndex = 0, int count = -1 ) = 0;

	/** Gets whether per-strand rotation angles are used.
	Per-strand rotation angles allow determining the twist of individual strands along their Z axis relative to the distribution surface topology
	@return true if angles are used
	*/
	virtual bool IsUsingPerStrandRotationAngles() const = 0;

	/** Sets whether per-strand rotation angles are used.
	Per-strand rotation angles allow determining the twist of individual strands along their Z axis relative to the distribution surface topology
	@param value true if angles are used
	*/
	virtual void SetUsingPerStrandRotationAngles( bool value ) = 0;

	/** Gets per-strand rotation angle if these angles are used
	@param strandIndex Index of strand
	@return Rotation angle in radians
	*/
	virtual Real GetPerStrandRotationAngle( unsigned strandIndex ) const = 0;

	/** Sets per-strand rotation angle if these angles are used
	@param strandIndex Index of strand
	@param value Rotation angle in radians
	*/
	virtual void SetPerStrandRotationAngle( unsigned strandIndex, Real value ) = 0;

	// Surface dependency:

	/** Gets whether this hair class retains information about where each strand is placed on the surface from which it was generated.
	Having this information allows, at a later time, to update the strand's position if the distribution geometry changes and also to derive additional information
	from the distribution mesh such as texture coordinates and per-vertex or per-face data.
	@see GetDistributionMesh()
	@return true if surface dependency information is kept
	*/
	virtual bool KeepsSurfaceDependency() const = 0;

	/** Sets whether this hair class retains information about where each strand is placed on the surface from which it was generated.
	@see KeepsSurfaceDependency()
	@see GetDistributionMesh()
	@param on true if surface dependency information is to be kept
	*/
	virtual void KeepsSurfaceDependency( bool on ) = 0;

	/** Gets information about about where specified strand was placed onto a distribution surface
	@see KeepsSurfaceDependency()
	@param strandIndex Strand index
	@return Surface dependency for specified strand
	*/
	virtual const SurfaceDependency& GetSurfaceDependency__deprecated( unsigned strandIndex ) const = 0;

	/** Sets information about about where specified strand was placed onto a distribution surface
	@see KeepsSurfaceDependency()
	@param strandIndex Strand index
	@param value Surface dependency for specified strand
	*/
	virtual void SetSurfaceDependency__deprecated( unsigned strandIndex, const SurfaceDependency& value ) = 0;

	/** Sets surface dependencies from the specified array of values
	This method could be faster than setting surface dependencies one by one by utilizing copying of an array of values
	@param source Array of surface dependencies to set
	@param count Number of values in source
	@param destinationStartIndex Index of first element in this object's surface dependency array to set
	*/
	virtual void SetSurfaceDependencies( const SurfaceDependency* source, unsigned count, unsigned destinationStartIndex = 0 ) = 0;

	// Distribution surface:

	//! Gets distribution mesh to which this hair applies or 0 if none is specified
	virtual const HostTriangleMeshAccessHandle& GetDistributionMesh() const = 0;

	//! Sets distribution mesh to which this hair applies or 0 if none is specified
	virtual void SetDistributionMesh( const HostTriangleMeshAccessHandle& value ) = 0;

	// Guide dependency:

	/** Gets whether information is kept about which guides were used to generate each strand.
	This only applies to dense hair or guides which were generated from guides. The information contains indices into guides object of guide strands used to generate a strand in this object
	as well as the distances to each guide. This information can be used to derive information about this strand from guides such as texture coordinates, channel data, and positions of vertices
	through interpolation.
	@see GetGuides()
	@return true if guide dependency is kept
	*/
	virtual bool KeepsGuideDependency() const = 0;

	/** Sets whether information is kept about which guides were used to generate each strand.
	@see KeepsGuideDependency()
	@see GetGuides()
	@param on true if guide dependency is to be kept
	*/
	virtual void KeepsGuideDependency( bool on ) = 0;

	/** Gets information about the guide used to generate specified strand
	@see KeepsGuideDependency()
	@param strandIndex Index of the strand
	@return Guide dependency for specified root
	*/
	virtual const GuideDependency& GetGuideDependency__deprecated( unsigned strandIndex ) const = 0;

	/** Sets information about the guide used to generate specified strand
	@see KeepsGuideDependency()
	@param strandIndex Index of the strand
	@param value Guide dependency for specified root
	*/
	virtual void SetGuideDependency__deprecated( unsigned strandIndex, const GuideDependency& value ) = 0;

	// Guides from which this hair was generated:

	/** Gets guides which control this hair or nullptr if none are assigned
	@return Guide hair object
	*/
	virtual IHair__deprecated* GetGuidesDeprecated() const = 0;

	/** Sets guides which control this hair or nullptr if none are assigned
	@param value Guide hair object
	*/
	virtual void SetGuidesDeprecated( IHair__deprecated* value ) = 0;

	// Topology orientation:

	/** Gets whether the twist orientation of strands should be linked to the distribution surface's topology.
	If this option is on the "twist" orientation along the strand should be calculated in a way which would make it stay aligned with the distribution surface as it deforms.
	@return true if topology orientation is used
	*/
	virtual bool UsesTopologyOrientation() const = 0;

	/** Sets whether the twist orientation of strands should be linked to the distribution surface's topology.
	@see UsesTopologyOrientation()
	@param on true if topology orientation is to be used
	*/
	virtual void UsesTopologyOrientation( bool on ) = 0;

	// Global per-strand point count:

	/** Gets whether all strands in this hair will have the same number of points
	Having all strands have the same number of points allows us not to track starting vertex indices for each strand and allows for this object to be more compact.
	@see UsesGlobalPerStrandPointCount( bool )
	@see UsesStrandTopology()
	@see GetGlobalStrandPointCount()
	@see SetGlobalStrandPointCount( unsigned )
	@return true if all strands have the same point count
	*/
	virtual bool UsesGlobalPerStrandPointCount() const = 0;

	/** Sets whether all strands in this hair will have the same number of points
	@see UsesGlobalPerStrandPointCount()
	@see UsesStrandTopology()
	@see GetGlobalStrandPointCount()
	@see SetGlobalStrandPointCount( unsigned )
	@param on true if all strands are to have the same point count
	*/
	virtual void UsesGlobalPerStrandPointCount( bool on ) = 0;

	/** Sets the uniform point count for all strands in this object
	@param value Number of points per strand to set
	*/
	virtual void SetGlobalStrandPointCount__deprecated( unsigned value ) = 0;

	/** Gets the uniform point count for all strands in this object
	@return Global number of points per strand
	*/
	virtual unsigned GetGlobalStrandPointCount__deprecated() const = 0;

	/** Gets the number of points in the specified strand
	@param strandIndex Index of strand
	@return Number of points in specified strand
	*/
	virtual unsigned GetStrandPointCount__deprecated( unsigned strandIndex ) const = 0;

	// Vertices and geometry:

	/** Gets the total number of vertices in this hair object
	@param absolute Determines whether to return all vertices stored in the hair structure, or number of vertices stored just for one frame. Typically (without animation storage) this parameter will not affect the result.
	@return Total number of hair vertices
	*/
	virtual unsigned GetVertexCount__deprecated( bool absolute ) const = 0;

	/** Sets the number of vertices in this hair object and resizes all connected data (based on flags)
	@param count New number of vertices
	@param keep If TRUE previous vertex information will be kept.
	*/
	virtual void SetVertexCount__deprecated( unsigned count, bool keep = false ) = 0;

	/** Append a vertex array to the end of vertex list and update lookup tables (if used)
	@param vertices Pointer to the vertex array to append
	@param vertexCount Number of element in vertex array
	*/
	virtual void AppendVerts( const HostVector3* vertices, unsigned vertexCount ) = 0;

	/** Gets the specified vertex position
	@param vertexIndex Index of vertex
	@return Position of specified vertex
	*/
	virtual const HostVector3& GetVertex__deprecated( unsigned vertexIndex ) const = 0;

	/** Gets the specified vertex position
	@param vertexIndex Index of vertex
	@return Position of specified vertex
	*/
	HostVector3& GetVertex__deprecated( unsigned vertexIndex )
	{
		return const_cast<HostVector3&>( const_cast<const IHair__deprecated&>( *this ).GetVertex__deprecated( vertexIndex ) );
	}

	/** Gets the specified vertex position
	@param vertexIndex Vertex index
	@param frame Index of the frame for which to get the vertex
	@return Position of specified vertex
	*/
	virtual HostVector3 GetVertex__deprecated( unsigned vertexIndex, float frame ) const = 0;

	/** Gets a pointer to an array of points defining the specified strand
	@param strandIndex Index of strand
	@return Pointer to first vertex for specified strand
	*/
	virtual const HostVector3* GetStrandPointsDeprecated( unsigned strandIndex ) const = 0;

	/** Sets the position of specified vertex
	@param vertexIndex Index of vertex
	@param value Position to set
	*/
	virtual void SetVertex__deprecated( unsigned vertexIndex, const HostVector3& value ) = 0;

	/** Gets the position of specified point on a strand
	@param strandIndex Index of strand
	@param pointIndex Index of point on strand
	@return Point position
	*/
	virtual HostVector3 GetStrandPoint__deprecated( unsigned strandIndex, unsigned pointIndex ) const = 0;

	/** Sets the position of specified point on a strand
	@param strandIndex Index of strand
	@param pointIndex Index of point on strand
	@param value Position to set
	*/
	virtual void SetStrandPoint__deprecated( unsigned strandIndex, unsigned pointIndex, const HostVector3& value ) = 0;

	// TODO: GetStrandPointInObjectCoordinates can be implemented in terms of the other methds of the interface (as SetStrandPointInObjectCoordinates), no need to be virtual
	/** Gets a strand point in object coordinates
	If a strand uses per-strand transform it is automatically applied to the resulting point, so we can be assured the result will always be in object coordinates.
	@param strandIndex Index of strand
	@param pointIndex Index of point on strand
	@return Point position in object coordinates
	*/
	virtual HostVector3 GetStrandPointInObjectCoordinates( unsigned strandIndex, unsigned pointIndex ) const = 0;

	//! The opposite of GetStrandPointInObjectCoordinates.
	void SetStrandPointInObjectCoordinates( unsigned strandIndex, unsigned pointIndex, const HostVector3& value )
	{

		if( UsesPerStrandTransformations() )
		{
			SetStrandPoint__deprecated( strandIndex, pointIndex, HostTransform( HostInverse( GetStrandTransform( strandIndex ) ), value ) );
		}
		else
		{
			SetStrandPoint__deprecated( strandIndex, pointIndex, value );
		}
	}

	//! Return the index of the strand to which the given vertex index belongs, along with its index within the strand
	void GetVertexStrandAndPointIndices( unsigned vertexIndex, unsigned& strandIndexResult, unsigned& pointIndexResult ) const
	{
		for( auto strandIndex = 0u, strandCount = GetStrandCount__deprecated(); strandIndex < strandCount; ++strandIndex )
		{
			auto firstVertex = GetFirstVertexIndex( strandIndex );
			auto vertexCount = GetStrandPointCount__deprecated( strandIndex );
			auto pointIndex = vertexIndex - firstVertex;
			if( pointIndex < vertexCount )
			{
				strandIndexResult = strandIndex;
				pointIndexResult = pointIndex;
				return;
			}
		}

		//CHECK_ARGUMENT_RANGE( strandIndex < GetStrandCount() );
	}

	HostVector3 GetVertexInObjectCoordinates( unsigned vertexIndex ) const
	{
		HostVector3 result;
		if( UsesPerStrandTransformations() )
		{
			unsigned strandIndex, pointIndex;
			GetVertexStrandAndPointIndices( vertexIndex, strandIndex, pointIndex );
			result = GetStrandPointInObjectCoordinates( strandIndex, pointIndex );
		}
		else
		{
			result = GetVertex__deprecated( vertexIndex );
		}
		return result;
	}

	void SetVertexInObjectCoordinates( unsigned vertexIndex, const HostVector3& value )
	{
		if( UsesPerStrandTransformations() )
		{
			unsigned strandIndex, pointIndex;
			GetVertexStrandAndPointIndices( vertexIndex, strandIndex, pointIndex );
			SetStrandPointInObjectCoordinates( strandIndex, pointIndex, value );
		}
		else
		{
			SetVertex__deprecated( vertexIndex, value );
		}
	}

	//! If this hair represents guides an IGuides interface is returned, otherwise 0
	virtual const IGuides* AsGuides() const = 0;

	//! If this hair represents guides an IGuides interface is returned, otherwise 0
	IGuides* AsGuides()
	{
		return const_cast<IGuides*>( const_cast<const IHair__deprecated&>( *this ).AsGuides() );
	}

	//! Gets whether this hair allocates extra space to allow for quick association of a strand to each vertex
	virtual bool UsesVertexLookupTable() const = 0;

	//! Sets whether this hair allocates extra space to allow for quick association of a strand to each vertex
	virtual void UsesVertexLookupTable( bool on ) = 0;

	/** Updates vertex lookup tables based on available data
	*/
	virtual void UpdateVertexLookupTable() = 0;

	/** Gets the index of the first vertex associated with specified strand
	@param strandIndex Index of strand
	@return Index into vertex array for the first point in specified root
	*/
	virtual unsigned GetFirstVertexIndex( unsigned strandIndex ) const = 0;

	//! Updates bounding box for this hair class.
	virtual void UpdateBoundingBox() = 0;

	/**	Gets the box containing all vertices of this object
	@param box Box structure to fill with current bounding box
	@param isUsingSelection When true, only selected sub-components will be included in the resulting bounding volume
	*/
	virtual void GetBoundingBox__deprecated( HostAxisAlignedBox3& box, bool isUsingSelection = false ) const = 0;

	// Texture Coordinates:

	//! Gets the number of mapping channels present in this hair structure
	virtual unsigned GetMappingChannelCount() const = 0;

	/** Computes per-strand UV coordinates based on available data. This class is fast because it
	*	computes all UVW data at once, otherwise GetTextureCoordinate() function can be used.
	*	@param result Array where UVW coords should be stored. The size of the array must be the same as current strand count.
	*	@param mappingChannelIndex Index of mapping channel
	*	@return true if succesful
	*/
	virtual bool GetTextureCoordinates__deprecated2( HostTextureCoordinate* result, unsigned mappingChannelIndex = 0 ) const = 0;

	/** Computes per-strand UV coordinates for a single root
	*	@param strandIndex Root index
	*	@param vertexIndex Optional vertex index. If nonzero W coordinate is set to i/numPts
	*	@param mappingChannelIndex Index of mapping channel
	*	@return Computed UVW coordinate
	*/
	virtual HostTextureCoordinate GetTextureCoordinate( unsigned strandIndex, unsigned vertexIndex, unsigned mappingChannelIndex ) const = 0;

	/**
	*	@param vertexIndex Vertex index
	*	@param mappingChannelIndex Index of mapping channel
	*	@return Computed UVW coordinate
	*/
	virtual HostTextureCoordinate GetTextureCoordinate( unsigned vertexIndex, unsigned mappingChannelIndex ) const = 0;

	// User data:

	virtual float GetPointData__deprecated( StrandChannelType type, unsigned channelIndex, unsigned strandIndex, unsigned pointIndex ) const = 0;

	/** Copies all flags from specified source hair class into this one
	@param source Hair structure from which to copy flags
	*/
	void CopyFlags( const IHair__deprecated& source )
	{
		UsesStrandTopology( source.UsesStrandTopology() );
		UsesPerStrandTransformations( source.UsesPerStrandTransformations() );
		KeepsSurfaceDependency( source.KeepsSurfaceDependency() );
		KeepsGuideDependency( source.KeepsGuideDependency() );
		UsesTopologyOrientation( source.UsesTopologyOrientation() );
		UsesGlobalPerStrandPointCount( source.UsesGlobalPerStrandPointCount() );
		UsesVertexLookupTable( source.UsesVertexLookupTable() );
	}

	/** Sets all flags in this hair structure to false
	*/
	void ClearFlags()
	{
		UsesStrandTopology( false );
		UsesPerStrandTransformations( false );
		KeepsSurfaceDependency( false );
		KeepsGuideDependency( false );
		UsesTopologyOrientation( false );
		UsesGlobalPerStrandPointCount( false );
		SetUsingPerStrandRotationAngles( false );
		UsesVertexLookupTable( false );
	}

	/** Gets the version of the code for current host which is implementing this interface.
	For example, in 3dsmax this will be the integer version of 3dsmax Ornatrix plugin. In Maya, it will be version of Maya plugin.
	@return Integer version
	*/
	virtual int GetImplementationVersion() const = 0;

	virtual const Xform3& GetVertexToObjectTransform( unsigned vertexIndex ) const = 0;

	virtual void ValidateVertexToObjectTransforms() = 0;

	virtual void InvalidateVertexToObjectTransforms() = 0;

	/** Determines whether hair in this object uses a custom set of texture coordinates not derived from distribution mesh.
	@return true if using custom texture coordinates
	*/
	virtual bool IsUsingCustomTextureCoordinates() const = 0;

	/** Sets whether this object uses a custom set of texture coordinates.
	@param value true if using custom texture coordinates
	*/
	virtual void SetUsingCustomTextureCoordinates( bool value ) = 0;

	/** When IsUsingCustomTextureCoordinates() is true this method can be called to set the custom texture coordinate
	@param strandIndex Strand index
	@param pointIndex Point Index
	@param coordinate Coordinate to set
	*/
	virtual void SetTextureCoordinate( unsigned strandIndex, unsigned pointIndex, const HostTextureCoordinate& coordinate ) = 0;

	///** Gets the index of the dense render strand of a strand which appears inside the viewport
	//	@param viewStransIndex Index of preview strand
	//	@return Index of render strand
	//*/
	//virtual unsigned GetRenderStrandIndex( unsigned viewStransIndex ) const = 0;

	/* Call this after changes were made to vertices in this hair */
	virtual void InvalidateGeometryCache__deprecated() = 0;

	/* Call this after distribution surface of this hair was modified */
	virtual void InvalidateDistributionSurfaceCache() = 0;

	/* Call this before using GetStrandTransform function to make sure correct transforms are computed */
	virtual void ValidateStrandTransforms__deprecated() = 0;

	//! This function is added to support Maya OxGetStrandIds command
	std::vector<StrandId> GetStrandIdsVector() const
	{
		const auto hair3 = dynamic_cast<const IHair*>( this );
		std::vector<StrandId> result( hair3->GetStrandCount() );
		hair3->GetStrandIds( 0, int( result.size() ), result.data() );

		return result;
	}

	//! This function is added to support Maya OxGetStrandGroups command
	std::vector<int> GetStrandGroupsVector() const
	{
		const auto hair3 = dynamic_cast<const IHair*>( this );
		std::vector<int> result( hair3->GetStrandCount() );
		hair3->GetStrandGroups( 0, int( result.size() ), result.data() );

		return result;
	}

	// "Extension" methods (in C# terms):

	/** Get data values for a specified root channel
	*  @param channelIndex Channel index
	*  @return List of float data values
	*/
	std::vector<float> GetRootValuesForChannel( const unsigned channelIndex ) const
	{
		const auto hair = dynamic_cast<const IHair*>( this );
		std::vector<float> result( hair->GetStrandCount() );
		hair->GetStrandChannelData( IHair::PerStrand, channelIndex, 0, int( result.size() ), result.data() );
		return result;
	}

	/** Get data values for a specified vertex channel
	*  @param channelIndex Channel index
	*  @return List of float data values
	*/
	std::vector<float> GetVertexValuesForChannel( const unsigned channelIndex ) const
	{
		const auto hair = dynamic_cast<const IHair*>( this );
		std::vector<float> result( hair->GetVertexCount() );
		hair->GetStrandChannelData( IHair::PerVertex, channelIndex, 0, int( result.size() ), result.data() );
		return result;
	}

protected:

	~IHair__deprecated()
	{
	}

	IHair__deprecated& operator=( const IHair__deprecated& )
	{
		return *this;
	}
};

/** IHair__deprecated interface extensions.
This interface is added to provide backwards compatibility with existing ornatrix installations.
All new virtual functions should go here and API/SDK clients needing the new functionality should
dynamic_cast to check if this interface is supported.
*/
class IHair2__deprecated : public IHair__deprecated
{
public:

	static const InterfaceId IID = InterfaceId_Hair + 2;

	/** Faster versions of IHair__deprecated's (using lookup table when available).
	*/
	HostVector3 GetVertexInObjectCoordinates2( unsigned vertexIndex ) const
	{
		HostVector3 result;
		if( UsesPerStrandTransformations() )
		{
			unsigned strandIndex, pointIndex;
			GetVertexStrandAndPointIndices2( vertexIndex, strandIndex, pointIndex );
			result = GetStrandPointInObjectCoordinates( strandIndex, pointIndex );
		}
		else
		{
			result = GetVertex__deprecated( vertexIndex );
		}
		return result;
	}

	void SetVertexInObjectCoordinates2( unsigned vertexIndex, const HostVector3& value )
	{
		if( UsesPerStrandTransformations() )
		{
			unsigned strandIndex, pointIndex;
			GetVertexStrandAndPointIndices2( vertexIndex, strandIndex, pointIndex );
			SetStrandPointInObjectCoordinates( strandIndex, pointIndex, value );
		}
		else
		{
			SetVertex__deprecated( vertexIndex, value );
		}
	}

	void GetVertexStrandAndPointIndices2( unsigned vertexIndex, unsigned& strandIndex, unsigned& pointIndex ) const
	{
		if( UsesVertexLookupTable() && LookupStrandIndex( vertexIndex, strandIndex ) )
		{
			pointIndex = vertexIndex - GetFirstVertexIndex( strandIndex );
		}
		else
		{
			// Fallback to slow version
			GetVertexStrandAndPointIndices( vertexIndex, strandIndex, pointIndex );
		}
	}

	/** Init vertex index -> strand index lookup table. Return true if init was successful, false otherwise.
	*/
	virtual bool InitVertexLookupTable() {
		return false;
	}

	/** If derrived classes use a lookup table to map vertex index to strand index, this is method should do the lookup and
	return true.
	Called by GetVertexInObjectCoordinates2, which has a default (slow) implementation.
	*/
	virtual bool LookupStrandIndex( unsigned /*vertexIndex*/, unsigned& /*strandIndex*/ ) const {
		return false;
	}

	virtual bool RetainDistributionMesh() {
		return false;
	}
	virtual bool ReleaseDistributionMesh() {
		return false;
	}

	virtual std::shared_ptr<IHair__deprecated> GetGuides() const = 0;

	virtual void SetGuides( const std::shared_ptr<IHair__deprecated>& value ) = 0;

	virtual int GetVertexStrandIndex( int vertexIndex ) const = 0;

protected:

	~IHair2__deprecated()
	{
	}

	IHair2__deprecated& operator=( const IHair2__deprecated& )
	{
		return *this;
	}
};

} } } // Ephere::Plugins::Ornatrix
