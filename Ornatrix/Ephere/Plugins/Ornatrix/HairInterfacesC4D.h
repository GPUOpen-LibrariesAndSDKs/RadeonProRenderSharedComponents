#pragma once

#include "c4d.h"

#if (API_VERSION < 20000)
#include "hashcode.h"
#endif

#ifdef MAXON_TARGET_WINDOWS
#pragma warning(disable : 4265)
#endif

#include "Ephere/Ornatrix/IHair.h"
#include "Ephere/Geometry/Native/IPolygonMesh.h"
#include "Ephere/NativeTools/IInterfaceProvider.h"

#define ID_OX_HAIR_OBJECT	1050340

namespace Ephere { namespace Plugins { namespace Ornatrix {

	using namespace Ephere::Ornatrix;
	
	inline bool MessageInterfaceObject( BaseObject* baseObject, const String key, GeData& result )
	{
		RetrievePrivateData privateData;
		privateData.data = &result;
#if (API_VERSION < 20000)
		auto cstr = key.GetCStringCopy();
		if( cstr == nullptr )
		{
			return false;
		}
		privateData.flags = Int32( maxon::CStringHash::GetHashCode( cstr ) );
		DeleteMem( cstr );
#else
		privateData.flags = Int32( key.GetHashCode() );
#endif
		return baseObject->Message( MSG_RETRIEVEPRIVATEDATA, &privateData ) == 0 ? false : true;
	}
	
	inline IInterfaceProvider* GetInterfaceProvider( BaseObject* baseObject )
	{
		GeData data;
		if ( !MessageInterfaceObject( baseObject, "IInterfaceProvider", data ) )
		{
			return nullptr;
		}
		
		auto result = static_cast<IInterfaceProvider*>( data.GetVoid() );

		return result;
	}

	inline std::pair<std::shared_ptr<IHair>, std::string> GetHairInterface( BaseObject* baseObject )
	{
		if ( baseObject == nullptr
			|| !baseObject->IsInstanceOf( ID_OX_HAIR_OBJECT ) )
		{
			return std::make_pair( nullptr, "Select an Ornatrix Hair Object" );
		}

		auto interfaceProvider = GetInterfaceProvider( baseObject );
		if ( interfaceProvider == nullptr )
		{
			return std::make_pair( nullptr, "Error retrieving Hair Interface" );
		}

		auto hair = interfaceProvider->GetInterface<IHair>( "outputHair" );
		if ( hair == nullptr )
		{
			return std::make_pair( nullptr, "No Hair found" );
		}

		return std::make_pair( hair, "" );
	}

	inline std::pair<std::shared_ptr<IPolygonMeshSA>, std::string> GetDistributionMeshInterface( BaseObject* baseObject )
	{
		if ( baseObject == nullptr
			|| !baseObject->IsInstanceOf( ID_OX_HAIR_OBJECT ) )
		{
			return std::make_pair( nullptr, "Select an Ornatrix Hair Object" );
		}
		
		auto interfaceProvider = GetInterfaceProvider( baseObject );
		if ( interfaceProvider == nullptr )
		{
			return std::make_pair( nullptr, "Error retrieving Distribution Mesh Interface" );
		}
		
		auto distributionMesh = interfaceProvider->GetInterface<IPolygonMeshSA>( "distributionMesh" );
		if ( distributionMesh == nullptr )
		{
			return std::make_pair( nullptr, "No Distribution Mesh found" );
		}
		
		return std::make_pair( distributionMesh, "" );
	}

} } } // Ephere::Plugins::Ornatrix
