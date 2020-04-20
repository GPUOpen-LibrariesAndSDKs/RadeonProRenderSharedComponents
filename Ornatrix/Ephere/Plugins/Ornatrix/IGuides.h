// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

#include "Ephere/Plugins/CommonTypes.h"
#include "Ephere/Plugins/Ornatrix/IHair__deprecated.h"

namespace Ephere { namespace Plugins { namespace Ornatrix
{

// Per-vertex data
enum GuidesPerVertexDataIndex
{
	GuidesPerVertexDataIndex_Selection = 0,
	GuidesPerVertexDataIndex_User = 1u
};

// Per-root data
enum GuidesPerRootDataIndex
{
	GuidesPerRootDataIndex_Selection = 0,
	GuidesPerRootDataIndex_User = 1u
};

// forward declares:
class IHair__deprecated;

/** Ornatrix guides interface class. Used in conjunction with IHair__deprecated class to add selection, channel, and weight information to strands and their vertices.
 */
class IGuides
{
public:

	//! Gets the hair interface referenced by these guides
	virtual const IHair__deprecated& AsHair() const = 0;

	//! Gets the hair interface referenced by these guides
	IHair__deprecated& AsHair()
	{
		return const_cast<IHair__deprecated&>( static_cast<const IGuides&>( *this ).AsHair() );
	}

	/** Deletes strands specified in the set
		@param set A set of strands to delete
	*/
	virtual void DeleteStrands__deprecated( const HostElementSelection& set ) = 0;

	//! Gets the number of vertex data channels
	virtual unsigned GetVertexDataCount() const = 0;

	/** Sets number of vertex data channels. This will also resize the vertex data array.
		@param numChans New number of vertex channels
		@param keep If TRUE vertex channel data is not reset
	*/
	virtual void SetVertexDataCount( unsigned numChans, bool keep = true ) = 0;

	/** Delete a set of channels based on specified selection
		@param set Set which is used for deletion (true - delete channel, false - leave channel)
	*/
	virtual void DeleteVertexChannels( const HostElementSelection& set ) = 0;

	/** Retreives single vertex data for specified channel and vertex.
		@param channel Channel index
		@param vertexIndex Vertex index
		@return Associated vertex data
	*/
	virtual float GetVertexData( unsigned channel, unsigned vertexIndex ) const = 0;

	/** Retreives vertex data pointer for specified channel and root.
		@see GetVertexData()
		@param rootIndex Root index
		@return Associated vertex data pointer
	*/
	virtual const float* GetVertexData__deprecated( unsigned rootIndex ) const = 0;

	/** Retreives vertex data pointer for specified channel and root.
	@see GetVertexData()
	@param rootIndex Root index
	@return Associated vertex data pointer
	*/
	float* GetVertexData__deprecated( const unsigned rootIndex )
	{
		return const_cast<float*>( static_cast<const IGuides*>( this )->GetVertexData__deprecated( rootIndex ) );
	}

	/** Set the value of vertex data for specified channel and vertex.
		@see GetVertexData()
		@param channelIndex Channel index
		@param vertexIndex Vertex index
		@param value New value for data
	*/
	virtual void SetVertexData( unsigned channelIndex, unsigned vertexIndex, float value ) = 0;

	/** Retreives single vertex data for specified channel, root, and vertex.
		@param channelIndex Channel index
		@param r Root index
		@param v Vertex index relative to the root
		@return Associated vertex data
	*/
	virtual float GetVertexData( unsigned channelIndex, unsigned r, unsigned v ) const = 0;

	/** Set the value of vertex data for specified channel, root, and vertex.
		@param channelIndex Channel index
		@param strandIndex Root index
		@param pointIndex Vertex index relative to the root
		@param value New #pragma GCC diagnostic ignored  for data
	*/
	virtual void SetVertexData( unsigned channelIndex, unsigned strandIndex, unsigned pointIndex, float value ) = 0;

	/** Set the vertex channel name for specified channel. This name appears in the UI.
		@param channelIndex Channel index
		@param name Null-terminated name string, maximum of 16 characters
	*/
	virtual void SetVertexChanName( unsigned channelIndex, const wchar_t* name ) = 0;

	/** Get the vertex channel name for specified channel. This name appears in the UI.
		@param channelIndex Channel index
		@return Null-terminated name string, maximum of 16 characters
	*/
	virtual const wchar_t* GetVertexChanName( unsigned channelIndex ) const = 0;

	virtual void SetRootDataCount( unsigned count, bool keep = true ) = 0;

	//! Gets number of root data channels
	virtual unsigned GetRootDataCount() const = 0;

	/** Delete a set of channels based on specified selection
		@param set Set which is used for deletion (true - delete channel, false - leave channel)
	*/
	virtual void DeleteRootChannels( const HostElementSelection& set ) = 0;

	virtual void SetRootChannelName( unsigned channelIndex, const wchar_t* name ) = 0;

	/** Gets the name of specified root channel
		@param channelIndex Channel index
		@return Name of the channel
	*/
	virtual const wchar_t* GetRootChannelName( unsigned channelIndex ) const = 0;

	/** Gets data associated with specified root by its index
		@param channelIndex Data channe index
		@param rootIndex Strand index
		@return Root data
	*/
	virtual float GetRootData( unsigned channelIndex, unsigned rootIndex ) const = 0;

	/** Sets data associated with specified root by its index
		@param channelIndex Data channe index
		@param rootIndex Strand index
		@param value Root data
	*/
	virtual void SetRootData( unsigned channelIndex, unsigned rootIndex, float value ) = 0;

	/** Gets a pointer to to contiguous storage of all root data in these guides.
		@return Root data storage pointer
	*/
	virtual const float* GetRootData__deprecated() const = 0;

	/** Notifies this object that root data has been changed after one or more operation. This will update the class internally. */
	virtual void NotifyRootDataUpdated() = 0;

	/** Gets the binary selection states of individual strands.
		A strand is only considered selected if its binary selection is true, even if its scalar selection set via SetStrandSelectionWeight(...) method is non-zero.
		@return Strand selection
	*/
	virtual const HostElementSelection& GetStrandSelection() const = 0;

	/** Sets strand selection to specified set
		@param selection New selection to assign to strands. If this set has fewer elements than there are strands the remaining strands will be deselected.
	*/
	virtual void SetStrandSelection( const HostElementSelection& selection ) = 0;

	/** Gets the scalar selection value of a strand.
		Binary root selection set via GetStrandSelection() must be set for this value to have any effect.
		@param rootIndex Index of the strand
		@return Scalar selection value
	*/
	virtual float GetStrandSelectionWeight( unsigned rootIndex ) const = 0;

	/** Sets the scalar selection value of a strand.
		Binary root selection set via GetStrandSelection() must be set for this value to have any effect.
		@param rootIndex Index of the strand
		@param value The new selection value
	*/
	virtual void SetStrandSelectionWeight( unsigned rootIndex, float value ) = 0;

	/** Gets a set indicating which strands are to be displayed in the UI
		@return Strand visibility set
	*/
	virtual const HostElementSelection& GetStrandVisibility() const = 0;

	/** Sets a set indicating which strands are to be displayed in the UI
		@param set Strand visibility set
	*/
	virtual void SetStrandVisibility( const HostElementSelection& set ) = 0;

	/** Gets a set indicating which strands can be selected through the UI
		@return Strand selectability set
	*/
	virtual const HostElementSelection& GetStrandSelectability() const = 0;

	/** Sets a set indicating which strands can be selected through the UI
		@param set Strand selectability set
	*/
	virtual void SetStrandSelectability( const HostElementSelection& set ) = 0;

	/** Sets all vertex data values for specified channel to a uniform value
		@param channelIndex Vertex data channel index
		@param value Value to assign to all vertex data
	*/
	virtual void SetAllVertexData( unsigned channelIndex, float value ) = 0;

protected:

	// Use {} instead of =default to support VC 2010
	~IGuides() {}
	IGuides& operator=( const IGuides& ) { return *this; }
};

class IGuides2 : public IGuides
{
public:

	//! Gets unique hash for current per strand channel values. This function is mutable and not thread safe.
	virtual size_t GetPerStrandDataHash() const = 0;

protected:

	// Use {} instead of =default to support VC 2010
	~IGuides2() {}
	IGuides2& operator=( const IGuides2& ) { return *this; }
};

} } } // Ephere::Plugins::Ornatrix
