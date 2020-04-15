// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

#include "Ephere/Ornatrix/IHairRenderingProperties.h"
#include "Ephere/Ornatrix/IHairRenderingPropertiesContainer.h"

namespace Ephere { namespace Plugins { namespace Ornatrix
{

class DefaultHairRenderingProperties : public IHairRenderingProperties2
{
public:

	virtual ~DefaultHairRenderingProperties() {}
	DefaultHairRenderingProperties() {}
	DefaultHairRenderingProperties( const DefaultHairRenderingProperties& ) {}
	DefaultHairRenderingProperties& operator=( const DefaultHairRenderingProperties& ) { return *this; }

	// From IHairRenderingProperties:

	bool IsUsed() const override
	{
		return true;
	}

	void SetUsed( bool ) override
	{
	}

	unsigned short GetStrandGroupIndex() const override
	{
		return 0;
	}

	void SetStrandGroupIndex( unsigned short ) override
	{
	}

	float GetGlobalStrandRadius() const override
	{
		return 0.5f;
	}

	void SetGlobalStrandRadius( float ) override
	{
	}

	const ITextureMapSA* GetRadiusMap() const override
	{
		return nullptr;
	}

	void SetRadiusMap( const ITextureMapSA* ) override
	{
	}

	float GetRadiusMapValue( float, float ) const override
	{
		return 1.0f;
	}

	unsigned GetRadiusStrandDataChannel() const override
	{
		return 0;
	}

	void SetRadiusStrandDataChannel( unsigned ) override
	{
	}

	const IFunction1* GetRadiusCurve() const override
	{
		return nullptr;
	}

	void SetRadiusCurve( const IFunction1* ) override
	{
	}

	bool IsUsingSelfShadowing() const override
	{
		return true;
	}

	void SetUsingSelfShadowing( bool ) override
	{
	}

	float GetSelfShadowingCoefficient() const override
	{
		return 1.0f;
	}

	void SetSelfShadowingCoefficient( float ) override
	{
	}

	unsigned GetRadiusMapChannel() const override
	{
		return 0;
	}

	void SetRadiusMapChannel( unsigned ) override
	{
	}

	// IHairRenderingProperties2

	const char* GetStrandGroupPattern() const override
	{
		return "";
	}

	void SetStrandGroupPattern( const char* ) override
	{
	}
};

class DefaultHairRenderingPropertiesContainer final : public IHairRenderingPropertiesContainer2
{
public:

	// From IHairRenderingPropertiesContainer:

	const IHairRenderingProperties* GetHairRenderingProperties( unsigned short /*strandGroupIndex*/ = 0u ) const override
	{
		return &defaultProperties_;
	}

	unsigned GetHairRenderingPropertiesCount() const override
	{
		return 1;
	}

	const IHairRenderingProperties& GetHairRenderingPropertiesByIndex( unsigned /*index*/ ) const override
	{
		return defaultProperties_;
	}

	const IHairRenderingProperties& GetDefaultRenderingProperties() const override
	{
		return defaultProperties_;
	}

private:

	DefaultHairRenderingProperties defaultProperties_;
};

} } } // namespace Ephere::Plugins::Ornatrix
