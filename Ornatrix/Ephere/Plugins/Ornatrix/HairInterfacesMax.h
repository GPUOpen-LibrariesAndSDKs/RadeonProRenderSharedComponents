// Must compile with VC 2012

#pragma once

#include "Ephere/Plugins/Ornatrix/DefaultHairRenderingProperties.h"
#include "Ephere/Plugins/Ornatrix/IHair__deprecated.h"

#include <max.h>

// Base classes
#define ORNAGUIDEHAIR_CLASS_ID	Class_ID( 0x2737ea1d, 0x3fe4aea7 )
#define ORNADENSEHAIR_CLASS_ID	Class_ID( 0x65c071fc, 0x27dc3eb0 )
#define ORNASTRANDS_CLASS_ID	Class_ID( 0xbfd4d76, 0x5dc22b67 )
#define OrnaGuideHairClassID	ORNAGUIDEHAIR_CLASS_ID
#define OrnaDenseHairClassID	ORNADENSEHAIR_CLASS_ID
#define OrnaStrandsClassID		ORNASTRANDS_CLASS_ID

// Interfaces
#define ORNA_HAIR_INTERFACE		Interface_ID( 0x57106329, 0x6fc35473 )
#define ORNA_GUIDES_INTERFACE	Interface_ID( 0x1f7225b3, 0x346703d5 )
#define IORNA_HAIR_CLASS_ID		Class_ID( 0x3d7e02f5, 0x42ae0215 )
#define IORNA_GUIDES_CLASS_ID	Class_ID( 0x12ed080b, 0x114132de )

#define ORNAGHAIRGEN_INTERFACE	Interface_ID( 0x261938bd, 0x74071179 )
#define ORNAHAIRPRESET_INTERFACE Interface_ID( 0x2c581ac5, 0x53337975 )
#define ORNABAKEDHAIR_INTERFACE Interface_ID( 0x6e631408, 0x341a500e )
#define ORNABAKEDGUIDES_INTERFACE Interface_ID( 0x4498190b, 0x25dd715a )
#define ORNAEDITGUIDES_INTERFACE Interface_ID( 0x59b16a5e, 0x123614c3 )
#define ORNASURFCOMB_INTERFACE	Interface_ID( 0x61e00c1e, 0x774e1ab4 )
#define ORNA_MESH_FROM_STRANDS_INTERFACE Interface_ID( 0x61e00c1e, 0x774e1ab5 )
#define ORNAHAIRSHELLS_INTERFACE Interface_ID( 0x64e15816, 0x6cd7b12 )
#define ORNAGROUNDSTRANDS_INTERFACE Interface_ID( 0x64e15816, 0x6cd7b13 )
#define ORNAHAIRFROMMESHSTRIPSOBJECT_INTERFACE Interface_ID( 0x64e15816, 0x6cd7b14 )
#define ORNAHAIRFROMPARTICLESOBJECT_INTERFACE Interface_ID( 0x64e15816, 0x6cd7b15 )
#define ORNAHAIRCLUSTERING_INTERFACE Interface_ID( 0xc1ff799e, 0xbf5861f3 )
#define WEAVEROBJECT_INTERFACE Interface_ID( 0x1aa8c620, 0x3295ecb )
#define STRAND_ANIMATION_MODIFIER_INTERFACE Interface_ID( 0x54691309, 0x36e767bf )
#define CLUMP_MODIFIER_INTERFACE Interface_ID( 0x8ee6671, 0xf8b72ae7 )

// Additional classes
#define ORNAMGUIDEGEN_CLASS_ID	Class_ID( 0x3f0d7d07, 0x55b2fefd )
#define ORNAGMGUIDEGEN_CLASS_ID	Class_ID( 0x744f06b7, 0x5f22a61 )
#define ORNASGUIDEGEN_CLASS_ID	Class_ID( 0x5f41e553, 0x5469622b )
#define ORNASGUIDEFROMHAIRGEN_CLASS_ID	Class_ID( 0x5f41e553, 0x5469622c )
#define ORNASGUIDEFROMPARTICLESGEN_CLASS_ID	Class_ID( 0x5f41e553, 0x5469622d )
#define ORNACGUIDEGEN_CLASS_ID Class_ID( 0x1a0d2543, 0x2fafc8b6 )
#define ORNAGHAIRGEN_CLASS_ID Class_ID( 0x5191aade, 0x4ae372e8 )
#define ORNAPHAIRGEN_CLASS_ID Class_ID( 0x6dfa912c, 0x437a815a )
#define ORNASMESHGEN_CLASS_ID Class_ID( 0x66b5f5bc, 0xa6e9575 )
#define ORNAEDITGUIDES_CLASS_ID Class_ID( 0x67530274, 0x622aa5b5 )
#define ORNASURFCOMB_CLASS_ID Class_ID( 0xcf69e6e, 0x1aa4a36b )
#define ORNADYNAMICS_CLASS_ID Class_ID( 0x58833ca2, 0x1f0d53e7 )
#define ORNAODEDYNAMICS_CLASS_ID Class_ID( 0x408fdf19, 0x4ec9bbb4 )
#define ORNAHAIRLENGTH_CLASS_ID Class_ID( 0x5e543a67, 0x33c4c202 )
#define ORNAHAIRCLUST_CLASS_ID Class_ID( 0x5eafbbd4, 0x1bed83bf )
#define ORNAHAIRCURL_CLASS_ID Class_ID( 0x5ef76aaa, 0x1255651d )
#define ORNASTRANDDETAIL_CLASS_ID Class_ID( 0x14757498, 0x3faae6bf )
#define ORNASTRANDGRAVITY_CLASS_ID Class_ID( 0x4b584a86, 0x358dafd2 )
#define ORNASTRANDSYM_CLASS_ID Class_ID( 0x18877f34, 0x7505d150 )
#define ORNARENDERSETTINGS_CLASS_ID Class_ID( 0x192dfd3b, 0x7be1047b )
#define ORNASTRANDFRIZZ_CLASS_ID Class_ID( 0x5b92cd54, 0x185520a4 )
#define ORNAMRHAIR_CLASS_ID Class_ID( 0x1b24c512, 0x38bdae0d )
#define ORNAHAIRSHELLS_CLASS_ID Class_ID( 0x16d6bea3, 0x48bca7e3 )
#define ORNASGROUND_CLASS_ID Class_ID( 0x66b57e5a, 0x7ea5e6e2 )
#define ORNAVRAYHAIR_CLASS_ID Class_ID( 0x5cfdceff, 0x584749d3 )
#define ORNASTRANDCLUST_CLASS_ID Class_ID( 0x261c2745, 0x5662059e )
#define ORNASTRANDANIM_CLASS_ID Class_ID( 0x3063b8ce, 0x6a37e641 )
#define ORNASTRANDPROP_CLASS_ID Class_ID( 0x46c0cfe5, 0x6577be83 )
#define OX_HAIRFROMMESHSTRIPS_CLASS_ID Class_ID( 0x3063b8ce, 0x6a37e642 )
#define OX_HAIRFROMPARTICLES_CLASS_ID Class_ID( 0x3063b8ce, 0x6a37e643 )
#define OX_ROTATESTRANDS_CLASS_ID Class_ID( 0x3113ae8b, 0xd03d2e3 )
#define ORNAHAIRIMPORT_CLASS_ID Class_ID( 0x1e7b455c, 0x1a623846 )
#define ORNAHAIREXPORT_CLASS_ID Class_ID( 0x70bbda95, 0x370bc6f1 )
#define ORNAALEMBICEXPORT_CLASS_ID Class_ID( 0x70bbda95, 0x370bc6f2 )
#define ORNASTRANDMULTIPLIER_CLASS_ID Class_ID( 0x04ba813c, 0x81db91ac )
#define PUSHAWAYFROMSURFACE_CLASS_ID Class_ID( 0x81bdb58, 0x29445da9 )
#define CACHEMODIFIER_CLASS_ID Class_ID( 0x7cf7496b, 0x4a0de0b4 )
#define GENERATE_GUIDE_DATA_MODIFIER_CLASS_ID Class_ID( 0xc5a720c1, 0x24093253 )
#define OX_WEAVEROBJECT_CLASS_ID Class_ID( 0x39df1f36, 0xaa11c73e )
#define OX_WEAVEPATTERNOBJECT_CLASS_ID Class_ID( 0xf4da75d2, 0xf4da75d2 )
#define OX_CHANGEWIDTH_CLASS_ID Class_ID( 0xad657a9, 0x556b0133 )
#define OX_OSCILLATOR_CLASS_ID Class_ID( 0xc2ea9326, 0x8c67e993 )
#define OX_RESOLVE_COLLISIONS_CLASS_ID Class_ID( 0x30920001, 0xc0b58719 )
#define OX_NORMALIZE_STRANDS_CLASS_ID Class_ID( 0x5ba3c3d7, 0x941148d2 )
#define OX_ADOPT_EXTERNAL_GUIDES_CLASS_ID Class_ID( 0xbe283415, 0xcc11b179 )
#define OX_CLUMP_MODIFIER_CLASS_ID Class_ID( 0x155e5fc2, 0x660def52 )
#define OX_BRAID_GUIDES_OBJECT_CLASS_ID Class_ID( 0xe75db463, 0x2df43b9e )
#define OX_MOOV_PHYSICS_MODIFIER_CLASS_ID Class_ID( 0xee23de9, 0xfa432946 )
#define OX_ALEMBIC_IMPORT_CLASS_ID Class_ID( 0x23f80293, 0x86a46f6c )
#define OX_ARNOLD_ORNATRIX_MODIFIER_CLASS_ID Class_ID( 0x98ab6a0a, 0xfbc8b6fb )
#define SCATTER_MODIFIER_CLASS_ID Class_ID( 0x9f4d2279, 0x7dec6644 )
#define OX_VRAY_SCENE_EXPORT_CLASS_ID Class_ID( 0x9b7b492, 0xaae8b78f )

#define ORNAOBJ_CLASS_ID Class_ID( 0x56bda05f, 0x7447a77f )
#define ORNAMGUIDEGENOBJ_CLASS_ID Class_ID( 0x5b8f6267, 0x391b5b7a )
#define GUIDESFROMSHAPEOBJECT_CLASS_ID Class_ID( 0x28feb915, 0x3b70baec )

#define ORNACHEAP_CLASS_ID Class_ID( 0xe3295a8, 0x5ee6b973 )
#define ORNAFAKEFUR_CLASS_ID Class_ID( 0x163204fb, 0x22d60f11 )
#define ORNATRANSLUCENT_CLASS_ID Class_ID( 0x3b377bf4, 0x74f12f5 )
#define ORNAEXPENSIVE_CLASS_ID Class_ID( 0xf150b75, 0x3aff8dbc )
#define ORNARAYTRACE_CLASS_ID Class_ID( 0x7d4eaa35, 0x5f69defe )
#define ORNARAYTRACEEX_CLASS_ID Class_ID( 0x27cc9d0, 0x93a47de )
#define ORNARAYTRACEGPU_CLASS_ID Class_ID( 0x3bf75f97, 0x74392840 )
#define ORNAEFFECT_CLASS_ID Class_ID( 0x1037148f, 0x3ceca343 )
#define ORNASHADOW_CLASS_ID Class_ID( 0x4ac1d2b2, 0x203e9c0e )
#define ORNADEEPSHADOW_CLASS_ID Class_ID( 0x753631f1, 0x29481199 )
#define ORNARTSHADOW_CLASS_ID Class_ID( 0x33640c4e, 0x27cf4841 )
#define ORNATEXELMAP_CLASS_ID Class_ID( 0x7eea59cc, 0x1207c8ac )
#define ORNAGUIDECHANMAP_CLASS_ID Class_ID( 0xaac0bd3, 0x335b0c17 )

#define ORNABGUIDES_CLASS_ID Class_ID( 0x5dd975bb, 0x76deb1c1 )
#define ORNABHAIR_CLASS_ID Class_ID( 0x5b5837f8, 0x2331602e )
#define ORNABBHAIR_CLASS_ID Class_ID( 0x3c071391, 0x41d070f0 )
#define ORNAVRAYOBJECT_CLASS_ID Class_ID( 0x4c870860, 0x1b4d7c48 )
#define ORNAHAIRPRESET_CLASS_ID Class_ID( 0x19c44c52, 0x4fe812d2 )
#define ORNAPRESETMAKER_CLASS_ID Class_ID( 0x55544c18, 0x2b7018ee )

// Mental Ray
#define ORNAMROBJECT_CLASS_ID Class_ID( 0x49b046a3, 0x378039fb )
#define ORNAMRSHADER_CLASS_ID Class_ID( 0x7dd840c1, 0x43446eb5 )
#define ORNAMRTEXSHADER_CLASS_ID Class_ID( 0x6c1c29ff, 0x16000b74 )


namespace Ephere { namespace Plugins { namespace Autodesk { namespace Max { namespace Ornatrix
{

enum OrnatrixInterfaceIDs
{
	//! Interface ID to use with a 3dsmax Animatable::GetInterface( ULONG ) method to get a pointer to IHair__deprecated
	IHairInterfaceID = I_USERINTERFACE + 4414,

	//! Interface ID to use with a 3dsmax Animatable::GetInterface( ULONG ) method to get a pointer to Ephere::Plugins::Ornatrix::IHair__deprecated
	IHostIndependentHairInterfaceID = IHairInterfaceID + 10,

	//! Interface ID to use with a 3dsmax Animatable::GetInterface( ULONG ) method to get a pointer to Ephere::Plugins::Ornatrix::IHair
	IHostIndependentHairInterface3ID = IHairInterfaceID + 12,

	//! Interface ID to use with a 3dsmax Animatable::GetInterface( ULONG ) method to get a pointer to IGuides
	IGuidesInterfaceID = I_USERINTERFACE + 51035,

	//! Interface ID to use with a 3dsmax Animatable::GetInterface( ULONG ) method to get a pointer to Ephere::Plugins::Ornatrix::IGuides
	IHostIndependentGuidesInterfaceID = IGuidesInterfaceID + 10,

	//! Interface ID to use with a 3dsmax Animatable::GetInterface( ULONG ) method to get a pointer to IHairRenderingPropertiesContainer
	IHairRenderingPropertiesContainerInterfaceID = I_USERINTERFACE + 4415,

	//! Interface ID to use with a 3dsmax Animatable::GetInterface( ULONG ) method to get a pointer to IHairRenderingPropertiesContainer
	IHostIndependentHairRenderingPropertiesContainerInterfaceID = IHairRenderingPropertiesContainerInterfaceID + 10
};

} } } } } // Ephere::Plugins::Autodesk::Max::Ornatrix


namespace Ephere { namespace Ornatrix
{
class IHair;
class IHairRenderingPropertiesContainer;
} }

namespace Ephere { namespace Plugins { namespace Ornatrix
{

class IHair__deprecated;

/** Gets Ornatrix hair interface from a 3dsmax object.
	@return IHair interface pointer if object was Ornatrix hair, nullptr otherwise
*/
inline Ephere::Ornatrix::IHair* GetHairInterface( Object& maxObject )
{
	return static_cast<Ephere::Ornatrix::IHair*>( maxObject.GetInterface( Autodesk::Max::Ornatrix::IHostIndependentHairInterface3ID ) );
}

/** Gets Ornatrix hair interface from a 3dsmax scene node.
	@return IHair interface pointer if object was Ornatrix hair, nullptr otherwise
*/
inline Ephere::Ornatrix::IHair* GetHairInterface( INode& maxNode, TimeValue time = 0 )
{
	return GetHairInterface( *maxNode.EvalWorldState( time ).obj );
}


/** Gets old (deprecated) Ornatrix hair interface and rendering properties from a 3dsmax object.
@return true if object was Ornatrix hair, false otherwise
*/
inline bool GetHairInterfaces( Object& maxObject, IHair__deprecated*& hairInterface, IHairRenderingPropertiesContainer*& renderingProperties )
{
	using namespace Autodesk::Max::Ornatrix;
	hairInterface = static_cast<IHair__deprecated*>( maxObject.GetInterface( IHostIndependentHairInterfaceID ) );
	if( hairInterface == nullptr )
	{
		return false;
	}

	renderingProperties = static_cast<IHairRenderingPropertiesContainer*>( maxObject.GetInterface( IHostIndependentHairRenderingPropertiesContainerInterfaceID ) );
	if( renderingProperties == nullptr )
	{
		static DefaultHairRenderingPropertiesContainer defaultContainer;
		renderingProperties = &defaultContainer;
	}

	return true;
}

/** Gets old (deprecated) Ornatrix hair interface and rendering properties from a 3dsmax scene node.
@return true if object was Ornatrix hair, false otherwise
*/
inline bool GetHairInterfaces( INode& maxNode, IHair__deprecated*& hairInterface, IHairRenderingPropertiesContainer*& renderingProperties, TimeValue time = 0 )
{
	return GetHairInterfaces( *maxNode.EvalWorldState( time ).obj, hairInterface, renderingProperties );
}

} } } // Ephere::Plugins::Ornatrix
