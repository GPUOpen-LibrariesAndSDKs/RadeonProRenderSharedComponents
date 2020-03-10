/**********************************************************************
Copyright 2020 Advanced Micro Devices, Inc
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
********************************************************************/
#include "XMLMaterialExportCommon.h"
#include "Logger.h"
#include <stack>
#include <fstream>
#include <ostream>
#include <sstream>
#include <iostream>
#include "RPRStringIDMapper.h"

// execute FireRender func and check for an error
#define CHECK_NO_ERROR(func)	{ \
								rpr_int status = func; \
								if (status != RPR_SUCCESS) \
									throw std::runtime_error("Radeon ProRender error (" + std::to_string(status) + ") in " + std::string(__FILE__) + ":" + std::to_string(__LINE__)); \
								}

RPRStringIDMapper g_rprStringMapper;

namespace
{
	const std::string kTab = "    "; // default 4spaces tab for xml writer
	const int kVersion = RPR_VERSION_MAJOR_MINOR_REVISION;

	struct Param
	{
		std::string type;
		std::string value;
	};

	class XmlWriter
	{
	public:
		XmlWriter(const std::string& file)
			: m_doc(file)
			, top_written(true)
		{}
		~XmlWriter()
		{
			endDocument();
		}
		//write header
		void startDocument()
		{
			m_doc << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
		}
		void endDocument()
		{
			size_t size = m_nodes.size();
			for (size_t i = 0; i < size; ++i)
			{
				endElement();
			}
		}
		void startElement(const std::string& node_name)
		{
			//write prev node with atts
			if (m_nodes.size() != 0 && top_written)
			{
				std::string tab = "";
				for (int i = 0; i < m_nodes.size() - 1; ++i) tab += kTab;
				const Node& node = m_nodes.top();
				m_doc << tab << "<" << node.name << "";
				for (const auto& at : node.atts)
				{
					m_doc << " " << at.type << "=\"" << at.value << "\""; // name="value"
				}
				m_doc << ">" << std::endl;
			}

			m_nodes.push({ node_name });
			top_written = true;
		}

		void endElement()
		{
			std::string tab = "";
			for (int i = 0; i < m_nodes.size() - 1; ++i) tab += kTab;
			const Node& node = m_nodes.top();
			if (top_written)
			{
				m_doc << tab << "<" << node.name << "";
				for (const auto& at : node.atts)
				{
					m_doc << " " << at.type << "=\"" << at.value << "\""; // name="value"
				}
				m_doc << "/>" << std::endl;
			}
			else
				m_doc << tab << "</" << node.name << ">" << std::endl;

			m_nodes.pop();
			top_written = false;
		}

		void writeAttribute(const std::string& name, const std::string& value)
		{
			Node& node = m_nodes.top();
			node.atts.push_back({ name, value });
		}

		void writeTextElement(const std::string& name, const std::string& text)
		{
			std::string tab = "";
			for (int i = 0; i < m_nodes.size() - 1; ++i) tab += kTab;
			if (m_nodes.size() != 0 && top_written)
			{
				const Node& node = m_nodes.top();
				m_doc << tab << "<" << node.name << "";
				for (const auto& at : node.atts)
				{
					m_doc << " " << at.type << "=\"" << at.value << "\""; // name="value"
				}
				m_doc << ">" << std::endl;
			}
			tab += kTab;
			//<name>text</name>
			m_doc << tab << "<" << name << ">" << text << "</" << name << ">" << std::endl;
			top_written = false;

		}

	private:
		std::ofstream m_doc;

		struct Node
		{
			std::string name;
			std::vector<Param> atts;
		};

		std::stack<Node> m_nodes;
		bool top_written; // show is element in top of m_nodes stack already written into xml or not.
	};

}

void InitParamList(void)
{

}

// return true if success
bool ParseRPR (
	rpr_material_node material,
	std::set<rpr_material_node>& nodeList,
	const std::string& folder,
	bool originIsUberColor, // set to TRUE if this 'material' is connected to a COLOR input of the UBER material
	std::unordered_map<rpr_image, RPR_MATERIAL_XML_EXPORT_TEXTURE_PARAM>& textureParameter,
	const std::string& material_name,
	bool exportImageUV
)
{
	// check if the node already stored
	if (nodeList.find(material) != nodeList.end())
	{
		return true;
	}

	nodeList.insert(material);

	rpr_int status = RPR_SUCCESS;

	rpr_material_node_type type = 0;
	rprMaterialNodeGetInfo(material, RPR_MATERIAL_NODE_TYPE, sizeof(rpr_material_node_type), &type, nullptr);

#ifdef  _DEBUG
	{
		std::string typeName;
		g_rprStringMapper.RPRMaterialType_id_to_string(type,typeName);
		DebugPrint("processed node name = %s", typeName.c_str());
	}
#endif

	size_t input_count = 0;
	status = rprMaterialNodeGetInfo(material, RPR_MATERIAL_NODE_INPUT_COUNT, sizeof(size_t), &input_count, nullptr);

	for (int i = 0; i < input_count; ++i)
	{
		rpr_int in_type;
		status = rprMaterialNodeGetInputInfo(material, i, RPR_MATERIAL_NODE_INPUT_TYPE, sizeof(in_type), &in_type, nullptr);
		if (in_type == RPR_MATERIAL_NODE_INPUT_TYPE_NODE)
		{
			size_t read_count = 0;
			uint32_t id = 0;
			CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(material, i, RPR_MATERIAL_NODE_INPUT_NAME, sizeof(uint32_t), &id, nullptr));
			std::string param_name;
			g_rprStringMapper.RPRMaterialInput_id_to_string(id,param_name);

			rpr_material_node ref_node = nullptr;
			status = rprMaterialNodeGetInputInfo(material, i, RPR_MATERIAL_NODE_INPUT_VALUE, sizeof(ref_node), &ref_node, nullptr);
			if (ref_node)
			{

				if (!exportImageUV  // if UV not exported
					&& type == RPR_MATERIAL_NODE_IMAGE_TEXTURE
					&& param_name.size() == 3 && param_name[0] == 'u' && param_name[1] == 'v' && param_name[2] == 0
					)
				{
					// not supported atm
				}
				else
				{
					ParseRPR(ref_node, nodeList, folder, originIsUberColor, textureParameter, material_name, exportImageUV);
				}
			}
		}
		else if (in_type == RPR_MATERIAL_NODE_INPUT_TYPE_IMAGE)
		{
			rpr_image img = nullptr;
			status = rprMaterialNodeGetInputInfo(material, i, RPR_MATERIAL_NODE_INPUT_VALUE, sizeof(img), &img, nullptr);
			size_t name_size = 0;
			status = rprImageGetInfo(img, RPR_OBJECT_NAME, 0, nullptr, &name_size);
			//create file if filename is empty(name == "\0")
			if (name_size == 1)
			{
				// not supported atm
			}
			else
			{
				std::string mat_name;
				mat_name.resize(name_size - 1);
				rprImageGetInfo(img, RPR_OBJECT_NAME, name_size, &mat_name[0], nullptr);
			}

			textureParameter[img].useGamma = originIsUberColor ? true : false;
		}
	}

	return true;
}

void ExportMaterials(const std::string& filename,
	const std::set<rpr_material_node>& nodeList,
	std::unordered_map<rpr_image, RPR_MATERIAL_XML_EXPORT_TEXTURE_PARAM>& textureParameter,
	rpr_material_node closureNode,
	const std::string& material_name,// example : "Emissive_Fluorescent_Magenta"
	const std::map<rpr_image, EXTRA_IMAGE_DATA>& extraImageData,
	bool exportImageUV
)
{
	XmlWriter writer(filename);
	if (nodeList.size() == 0)
	{
		std::cout << "MaterialExport error: materials input array is nullptr" << std::endl;
		return;
	}

	try
	{
		writer.startDocument();
		writer.startElement("material");
		writer.writeAttribute("name", material_name);

		// in case we need versioning the material XMLs...  -  unused for the moment.
		writer.writeAttribute("version_exporter", "10");

		// version of the RPR_API_VERSION used to generate this XML
		std::stringstream hex_version;
		hex_version << "0x" << std::hex << kVersion;
		writer.writeAttribute("version_rpr", hex_version.str());

		// file path as key, id - for material nodes connections
		std::map<std::string, std::pair< std::string, rpr_image >  > textures;
		std::map<void*, std::string> objects; // <object, node_name> map

		std::set<std::string> used_names; // to avoid duplicating node names

		std::string closureNodeName = "";

		// fill materials first
		for (const auto& iNode : nodeList)
		{
			rpr_material_node mat = iNode;
			if (objects.count(mat))
				continue;
			size_t name_size = 0;
			CHECK_NO_ERROR(rprMaterialNodeGetInfo(mat, RPR_OBJECT_NAME, NULL, nullptr, &name_size));
			std::string mat_node_name;
			mat_node_name.resize(name_size - 1); // exclude extra terminator
			CHECK_NO_ERROR(rprMaterialNodeGetInfo(mat, RPR_OBJECT_NAME, name_size, &mat_node_name[0], nullptr));
			int postfix_id = 0;

			if (mat_node_name.empty())
				mat_node_name = "node" + std::to_string(postfix_id);

			while (used_names.count(mat_node_name))
			{
				mat_node_name = "node" + std::to_string(postfix_id);
				++postfix_id;
			}

			if (mat == closureNode)
				closureNodeName = mat_node_name;

			objects[mat] = mat_node_name;
			used_names.insert(mat_node_name);
		}

		// closure_node is the name of the node containing the final output of the material
		writer.writeAttribute("closure_node", closureNodeName);

		// look for images
		for (const auto& iNode : nodeList)
		{
			rpr_material_node mat = iNode;
			size_t input_count = 0;
			CHECK_NO_ERROR(rprMaterialNodeGetInfo(mat, RPR_MATERIAL_NODE_INPUT_COUNT, sizeof(size_t), &input_count, nullptr));

			for (int input_id = 0; input_id < input_count; ++input_id)
			{
				rpr_material_node_type input_type;
				CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(mat, input_id, RPR_MATERIAL_NODE_INPUT_TYPE, sizeof(input_type), &input_type, nullptr));

				if (input_type != RPR_MATERIAL_NODE_INPUT_TYPE_IMAGE)
					continue;

				rpr_image img = nullptr;
				CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(mat, input_id, RPR_MATERIAL_NODE_INPUT_VALUE, sizeof(img), &img, nullptr));

				if (objects.count(img)) // already mentioned
					continue;

				std::string img_node_name = "node0";
				int postfix_id = 0;
				while (used_names.count(img_node_name))
				{
					img_node_name = "node" + std::to_string(postfix_id);
					++postfix_id;
				}

				objects[img] = img_node_name;
				used_names.insert(img_node_name);
			}
		}

		// optionally write description (at the moment there is no description)
		writer.writeTextElement("description", "");

		for (const auto& iNode : nodeList)
		{
			rpr_material_node node = iNode;

			rpr_material_node_type type;
			size_t input_count = 0;
			CHECK_NO_ERROR(rprMaterialNodeGetInfo(node, RPR_MATERIAL_NODE_TYPE, sizeof(rpr_material_node_type), &type, nullptr));
			CHECK_NO_ERROR(rprMaterialNodeGetInfo(node, RPR_MATERIAL_NODE_INPUT_COUNT, sizeof(size_t), &input_count, nullptr));
			writer.startElement("node");
			writer.writeAttribute("name", objects[node]);
			std::string typeName;
			g_rprStringMapper.RPRMaterialType_id_to_string(type,typeName);
			writer.writeAttribute("type", typeName);

			for (int input_id = 0; input_id < input_count; ++input_id)
			{
				size_t read_count = 0;
				uint32_t id = 0;
				CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(node, input_id, RPR_MATERIAL_NODE_INPUT_NAME, sizeof(uint32_t), &id, nullptr));
				std::string param_name;
				g_rprStringMapper.RPRMaterialInput_id_to_string(id,param_name);

				if (!exportImageUV  // if UV not exported
					&& type == RPR_MATERIAL_NODE_IMAGE_TEXTURE
					&& param_name.size() == 3 && param_name[0] == 'u' && param_name[1] == 'v' && param_name[2] == 0
					)
				{
					continue; // ignore this parameter
				}

				std::string type = "";
				std::string value = "";
				rpr_material_node_type input_type;
				CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(node, input_id, RPR_MATERIAL_NODE_INPUT_TYPE, sizeof(input_type), &input_type, &read_count));

				switch (input_type)
				{
					case RPR_MATERIAL_NODE_INPUT_TYPE_FLOAT4:
					{
						rpr_float fvalue[4] = { 0.f, 0.f, 0.f, 0.f };
						CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(node, input_id, RPR_MATERIAL_NODE_INPUT_VALUE, sizeof(fvalue), fvalue, nullptr));
						//fvalue converted to string
						std::stringstream ss;
						ss << fvalue[0] << ", " <<
							fvalue[1] << ", " <<
							fvalue[2] << ", " <<
							fvalue[3];
						type = "float4";
						value = ss.str();
						break;
					}

					case RPR_MATERIAL_NODE_INPUT_TYPE_UINT:
					{
						rpr_int uivalue = RPR_SUCCESS;
						CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(node, input_id, RPR_MATERIAL_NODE_INPUT_VALUE, sizeof(uivalue), &uivalue, nullptr));

						//value converted to string
						type = "uint";
						value = std::to_string(uivalue);
						break;
					}

					case RPR_MATERIAL_NODE_INPUT_TYPE_NODE:
					{
						rpr_material_node connection = nullptr;
						rpr_int res = rprMaterialNodeGetInputInfo(node, input_id, RPR_MATERIAL_NODE_INPUT_VALUE, sizeof(connection), &connection, nullptr);
						CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(node, input_id, RPR_MATERIAL_NODE_INPUT_VALUE, sizeof(connection), &connection, nullptr));
						type = "connection";
						if (!objects.count(connection) && connection)
						{
							throw std::runtime_error("input material node is missing");
						}
						value = objects[connection];
						break;
					}

					case RPR_MATERIAL_NODE_INPUT_TYPE_IMAGE:
					{
						type = "connection";
						rpr_image tex = nullptr;
						CHECK_NO_ERROR(rprMaterialNodeGetInputInfo(node, input_id, RPR_MATERIAL_NODE_INPUT_VALUE, sizeof(tex), &tex, nullptr));
						size_t name_size = 0;
						CHECK_NO_ERROR(rprImageGetInfo(tex, RPR_OBJECT_NAME, NULL, nullptr, &name_size));
						std::string tex_name;
						tex_name.resize(name_size - 1);
						CHECK_NO_ERROR(rprImageGetInfo(tex, RPR_OBJECT_NAME, name_size, &tex_name[0], nullptr));

						// replace \\ by /
						// so we ensure all paths are using same convention  ( better for Unix )
						for (int i = 0; i < tex_name.length(); i++)
						{
							if (tex_name[i] == '\\')
								tex_name[i] = '/';
						}

						// we don't want path that looks like :  "H:/RPR/MatLib/MaterialLibrary/1.0 - Copy/RadeonProRMaps/Glass_Used_normal.jpg"
						// all paths must look like "RadeonProRMaps/Glass_Used_normal.jpg"
						// the path RadeonProRMaps/  is hard coded here for the moment .. needs to be exposed in exporter GUI if we change it ?
						const std::string radeonProRMapsFolder = "/RadeonProRMaps/";
						size_t pos = tex_name.find(radeonProRMapsFolder);

						if (pos != std::string::npos)
						{
							tex_name = tex_name.substr(pos + 1);
							int a = 0;
						}

						if (!textures.count(tex_name))
						{
							int tex_node_id = 0;
							std::string tex_node_name = objects[tex];
							textures[tex_name] = std::pair<std::string, rpr_image>(tex_node_name, tex);
						}

						value = textures[tex_name].first;
						break;
					}

					default:
						throw std::runtime_error("unexpected material node input type " + std::to_string(input_type));
				}

				if (!value.empty())
				{
					writer.startElement("param");
					writer.writeAttribute("name", param_name.data());
					writer.writeAttribute("type", type);
					writer.writeAttribute("value", value);
					writer.endElement();
				}
			}

			writer.endElement();
		}

		for (const auto& tex : textures)
		{
			writer.startElement("node");
			writer.writeAttribute("name", tex.second.first);
			writer.writeAttribute("type", "INPUT_TEXTURE");
			writer.startElement("param");
			writer.writeAttribute("name", "path");
			writer.writeAttribute("type", "file_path");
			writer.writeAttribute("value", tex.first);
			writer.endElement();

			writer.startElement("param");
			writer.writeAttribute("name", "gamma");
			writer.writeAttribute("type", "float");
			bool gammaset = false;

			if (textureParameter.find(tex.second.second) != textureParameter.end())
			{
				RPR_MATERIAL_XML_EXPORT_TEXTURE_PARAM& param = textureParameter[tex.second.second];
				if (param.useGamma)
				{
					writer.writeAttribute("value", std::to_string(2.2f));
					gammaset = true;
				}
			}

			if (!gammaset)
			{
				writer.writeAttribute("value", std::to_string(1.0f));
			}

			writer.endElement();

			if (extraImageData.find(tex.second.second) != extraImageData.end())
			{
				const EXTRA_IMAGE_DATA& extra = extraImageData.at(tex.second.second);

				if (extra.scaleX != 1.0f)
				{
					writer.startElement("param");
					writer.writeAttribute("name", "tiling_u");
					writer.writeAttribute("type", "float");
					writer.writeAttribute("value", std::to_string(extra.scaleX));
					writer.endElement();
				}

				if (extra.scaleY != 1.0f)
				{
					writer.startElement("param");
					writer.writeAttribute("name", "tiling_v");
					writer.writeAttribute("type", "float");
					writer.writeAttribute("value", std::to_string(extra.scaleY));
					writer.endElement();
				}
			}

			writer.endElement();
		}

		writer.endDocument();
	}
	catch (const std::exception& ex)
	{
		std::cout << "MaterialExport error: " << ex.what() << std::endl;
	}

	return;
}

