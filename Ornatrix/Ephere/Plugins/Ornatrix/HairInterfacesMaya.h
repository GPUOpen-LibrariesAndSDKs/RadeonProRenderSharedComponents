// THIS FILE MUST COMPILE WITH Visual Studio 2012 / GCC 4.8

#pragma once

#include "Ephere/Geometry/Native/IPolygonMesh.h"
#include "IHair__deprecated.h"
#include "IHairContainer.h"
#include "DefaultHairRenderingProperties.h"

#include <maya/MDataHandle.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MPxGeometryData.h>

// shared_ptr
#include <memory>

namespace Ephere { namespace Plugins { namespace Ornatrix
{

/** Gets an IInterfaceProvider pointer from a dependency node, used to access various interfaces of its output data
	@return An IInterfaceProvider pointer if the node implements one, nullptr otherwise
*/
inline IInterfaceProvider* GetInterfaceProvider( const MObject& node )
{
	MFnDependencyNode depNode( node );

	MStatus status;

	auto interfacePlug = depNode.findPlug( "interfaceProvider", false, &status );
	if( !status )
	{
		return nullptr;
	}

	auto handle = interfacePlug.asMDataHandle(
#if MAYA_API_VERSION < 20180000
		MDGContext::fsNormal,
#endif
		&status );
	if( !status )
	{
		return nullptr;
	}

	auto result = static_cast<IInterfaceProvider*>( handle.asAddr() );
	interfacePlug.destructHandle( handle );
	return result;
}

/** Gets Ornatrix hair interface from a HairShape node
	@return shared_ptr to hair interface if object was Ornatrix HairShape and IHair was provided
*/
inline std::shared_ptr<IHair> GetHairInterface( const MObject& hairShapeNode )
{
	auto interfaceProvider = GetInterfaceProvider( hairShapeNode );
	return interfaceProvider != nullptr ? interfaceProvider->GetInterface<IHair>( "outputHair" ) : std::shared_ptr<IHair>();
}

inline std::shared_ptr<IPolygonMeshSA> GetDistributionMeshInterface( const MObject& hairShapeNode )
{
	auto interfaceProvider = GetInterfaceProvider( hairShapeNode );
	return interfaceProvider != nullptr ? interfaceProvider->GetInterface<IPolygonMeshSA>( "distributionMesh" ) : std::shared_ptr<IPolygonMeshSA>();
}

/** Gets old (deprecated) Ornatrix hair and rendering properties interfaces from a HairShape node
@return true if object was Ornatrix HairShape and both IHair__deprecated and IHairRenderingPropertiesContainer were provided, false otherwise
*/
inline bool GetHairInterfaces( const MObject& hairShapeNode, std::shared_ptr<IHair__deprecated>& hair, std::shared_ptr<IHairRenderingPropertiesContainer>& renderSettings )
{
	auto interfaceProvider = GetInterfaceProvider( hairShapeNode );
	if( interfaceProvider == nullptr )
	{
		return false;
	}

	hair = interfaceProvider->GetInterface<IHair__deprecated>( "outputHair" );
	if( hair == nullptr )
	{
		return false;
	}

	renderSettings = interfaceProvider->GetInterface<IHairRenderingPropertiesContainer>( "outputRenderSettings" );
	if( renderSettings == nullptr )
	{
		renderSettings.reset( new DefaultHairRenderingPropertiesContainer );
	}

	return true;
}


const unsigned EphereIdentifierBase = 0x00124400;

// Matches the virtual table layout of Ephere::Plugins::Autodesk::Maya::HairData (which is not exported), to give clients access to the IHair stored in it
class HairData : public MPxGeometryData, public IHairContainer, public IInterfaceProvider
{
public:
	// Stable version 2.3.6
	static const int IHairSupportRevision = 19497;

	static MTypeId GetTypeId()
	{
		return MTypeId( EphereIdentifierBase, 1 );
	}
};

// This class is used just to distinguish data containing "hair" (ready for rendering) from "guides" (with some extra info needed for editing).
// Some operators require or produce "guides", while the operators which don't care about guides accept both
class GuidesData : public HairData
{
public:
	static MTypeId GetTypeId()
	{
		return MTypeId( EphereIdentifierBase, 3 );
	}
};

} } } // Ephere::Plugins::Ornatrix
