// This is common part of RPR Material Export
#pragma once

#include <RadeonProRender.h>
#include <RadeonProRender_GL.h>
#include <rprDeprecatedApi.h>

#include <list>
#include <map>
#include <vector>
#include <cassert>
#ifndef OSMac_
#include <memory>
#endif
#include <set>
#include <string>
#include <unordered_map>

struct EXTRA_IMAGE_DATA
{
	EXTRA_IMAGE_DATA()
	{
		scaleX = 1.0;
		scaleY = 1.0;
	}

	std::string path;
	float scaleX;
	float scaleY;
};

void InitParamList(void);

struct RPR_MATERIAL_XML_EXPORT_TEXTURE_PARAM
{
	RPR_MATERIAL_XML_EXPORT_TEXTURE_PARAM()
	{
		useGamma = false;
	}
	bool useGamma;
};

// return true if success
bool ParseRPR(
	rpr_material_node material,
	std::set<rpr_material_node>& nodeList,
	const std::string& folder,
	bool originIsUberColor, // set to TRUE if this 'material' is connected to a COLOR input of the UBER material
	std::unordered_map<rpr_image, RPR_MATERIAL_XML_EXPORT_TEXTURE_PARAM>& textureParameter,
	const std::string& material_name,
	bool exportImageUV
);

void ExportMaterials(const std::string& filename,
	const std::set<rpr_material_node>& nodeList,
	std::unordered_map<rpr_image, RPR_MATERIAL_XML_EXPORT_TEXTURE_PARAM>& textureParameter,
	rpr_material_node closureNode,
	const std::string& material_name,// example : "Emissive_Fluorescent_Magenta"
	const std::map<rpr_image, EXTRA_IMAGE_DATA>& extraImageData,
	bool exportImageUV
);

