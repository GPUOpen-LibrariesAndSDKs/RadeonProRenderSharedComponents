// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/NativeTools/IInterfaceProvider.h"

namespace Ephere { namespace Ornatrix
{
// forward declares:
class IHairRenderingProperties;

//! Interface to a class containing IHairRenderingProperties
class IHairRenderingPropertiesContainer
{
public:

	static const InterfaceId IID = 0x100201;

	/** Gets hair rendering properties for specified strand group index
		@param strandGroupIndex Index of the strand group for which to get the rendering properties
		@return Rendering properties or null if no properties exist for the specified group
	*/
	virtual const IHairRenderingProperties* GetHairRenderingProperties( unsigned short strandGroupIndex ) const = 0;

	/** Gets hair rendering properties for specified strand group index
		@param strandGroupIndex Index of the strand group for which to get the rendering properties
		@return Rendering properties or null if no properties exist for the specified group
	*/
	inline IHairRenderingProperties* GetHairRenderingProperties( unsigned short strandGroupIndex )
	{
		return const_cast<IHairRenderingProperties*>( static_cast<const IHairRenderingPropertiesContainer&>( *this ).GetHairRenderingProperties( strandGroupIndex ) );
	}

	/** Gets the total number of hair rendering properties by group in this container
		@return Hair rendering properties count
	*/
	virtual unsigned GetHairRenderingPropertiesCount() const = 0;

	/** Gets hair rendering properties by their index within this container
		@param index Index of the properties to get
		@return Hair rendering properties instance
	*/
	virtual const IHairRenderingProperties& GetHairRenderingPropertiesByIndex( unsigned index ) const = 0;

protected:

	~IHairRenderingPropertiesContainer() {}
	IHairRenderingPropertiesContainer& operator=( const IHairRenderingPropertiesContainer& ) { return *this; }
};

class IHairRenderingPropertiesContainer2 : public IHairRenderingPropertiesContainer
{
public:

	static const InterfaceId IID = 0x100202;

	virtual const IHairRenderingProperties& GetDefaultRenderingProperties() const = 0;

protected:

	~IHairRenderingPropertiesContainer2() {}
	IHairRenderingPropertiesContainer2() {}
	IHairRenderingPropertiesContainer2( const IHairRenderingPropertiesContainer2& ) {}
	IHairRenderingPropertiesContainer2& operator=( const IHairRenderingPropertiesContainer2& ) { return *this; }
};

} }
