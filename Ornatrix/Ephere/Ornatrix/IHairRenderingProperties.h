// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "IFunction.h"

namespace Ephere { namespace Ornatrix
{
class ITextureMapSA;

//! Interface to properties that specify how to render an IHair__deprecated interface
class IHairRenderingProperties
{
public:

	// Properties:

	//! Gets whether this class needs to be used or ignored
	virtual bool IsUsed() const = 0;

	//! Sets whether this class needs to be used or ignored
	virtual void SetUsed( bool value ) = 0;

	//! Gets to which strand group these properties need to be applied. Different properties can be applied to different groups.
	virtual unsigned short GetStrandGroupIndex() const = 0;

	//! Sets to which strand group these properties need to be applied
	virtual void SetStrandGroupIndex( unsigned short value ) = 0;

	// Strand radius:

	//! Gets global radius to be used (or pre-multiplied) for all strands
	virtual float GetGlobalStrandRadius() const = 0;

	//! Sets global radius to be used (or pre-multiplied) for all strands
	virtual void SetGlobalStrandRadius( float value ) = 0;

	//! Gets a thickness map to be used to determine strand radius on per-strand basis, or 0 if none
	virtual const ITextureMapSA* GetRadiusMap() const = 0;

	//! Sets a thickness map to be used to determine strand radius on per-strand basis, or 0 if none
	virtual void SetRadiusMap( const ITextureMapSA* value ) = 0;

	//! Gets strand data channel to be used to determine strand thickness, or unsigned( -1 ) if none
	virtual unsigned GetRadiusStrandDataChannel() const = 0;

	//! Sets strand data channel to be used to determine strand thickness, or unsigned( -1 ) if none
	virtual void SetRadiusStrandDataChannel( unsigned value ) = 0;

	//! Gets a curve to determine global strand thickness on points along the strand, or 0 if none
	virtual const IFunction1* GetRadiusCurve() const = 0;

	//! Sets a curve to determine global strand thickness on points along the strand, or 0 if none
	virtual void SetRadiusCurve( const IFunction1* curve ) = 0;

	// Self shadowing:

	//! Gets whether self-shadowing is used
	virtual bool IsUsingSelfShadowing() const = 0;

	//! Sets whether self-shadowing is used
	virtual void SetUsingSelfShadowing( bool value ) = 0;

	//! Gets self-shadowing coefficient
	virtual float GetSelfShadowingCoefficient() const = 0;

	//! Sets self-shadowing coefficient
	virtual void SetSelfShadowingCoefficient( float value ) = 0;

	//! Gets the texture mapping channel index if radius map is used
	virtual unsigned GetRadiusMapChannel() const = 0;

	/** Sets the texture mapping channel index if radius map is used
		@param value Mapping channel index
	*/
	virtual void SetRadiusMapChannel( unsigned value ) = 0;

	//! Get the map value for a given UV coordinate
	virtual float GetRadiusMapValue( float u, float v ) const = 0;

protected:

	IHairRenderingProperties() {}
	IHairRenderingProperties( const IHairRenderingProperties& ) {}
	~IHairRenderingProperties() {}
	IHairRenderingProperties& operator=( const IHairRenderingProperties& ) { return *this; }
};

/** IHairRenderingProperties interface extensions.
	This interface is added to provide backwards compatibility with existing ornatrix installations.
	All new virtual functions should go here and API/SDK clients needing the new functionality should 
	dynamic_cast to check if this interface is supported.
*/
class IHairRenderingProperties2 : public IHairRenderingProperties
{
public:

	//! Gets to which strand groups these properties need to be applied
	virtual const char* GetStrandGroupPattern() const = 0;

	//! Sets to which strand groups these properties need to be applied
	virtual void SetStrandGroupPattern( const char* value ) = 0;

protected:

	IHairRenderingProperties2() {}
	IHairRenderingProperties2( const IHairRenderingProperties2& ) {}
	~IHairRenderingProperties2() {}
	IHairRenderingProperties2& operator=( const IHairRenderingProperties2& ) { return *this; }
};

} }
