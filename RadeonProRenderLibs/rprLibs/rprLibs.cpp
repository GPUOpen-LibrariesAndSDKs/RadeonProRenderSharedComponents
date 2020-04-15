#include "rprLibs.h"

#include "rprDeprecatedApi.h"

#include "math/matrix.h"
#include "math/mathutils.h"

#pragma warning(push)
#pragma warning(disable:4251 4244 4275 4146)		//Not the best practice, but there are way too many warnings from the header that pollutes the output log
	#include "openvdb/openvdb.h"
	#include "openvdb/points/PointDataGrid.h"
	#include "openvdb/tools/Interpolation.h"
#pragma warning(pop)

#include <string>
#include <vector>
#include <mutex>

namespace
{
	const float defaultDensity = 100.f;                        //RPR take density value of 100 as fully opaque
	const float defaultColor[3] = { 0.05f, 0.05f, 0.05f };        //Default color of black
	const float defaultEmission[3] = { 0.0f, 0.0f, 0.0f };     //Default to no emission
}

namespace
{
	const float temperatureToColorMapMaxTemperature = 10000.0f;
	const float temperatureToColor[] = 
	{
		0.000000f, 0.000000f, 0.000000f,
		0.043055f, 0.001184f, 0.000000f,
		0.172220f, 0.004734f, 0.000000f,
		0.387495f, 0.010652f, 0.000000f,
		0.688881f, 0.018937f, 0.000000f,
		1.076376f, 0.029590f, 0.000000f,
		1.549982f, 0.042609f, 0.000000f,
		2.109697f, 0.057996f, 0.000000f,
		2.755523f, 0.075749f, 0.000000f,
		3.487458f, 0.095870f, 0.000000f,
		4.305504f, 0.118358f, 0.000000f,
		4.093179f, 0.181527f, 0.000000f,
		3.825340f, 0.261242f, 0.000000f,
		3.555754f, 0.341438f, 0.000000f,
		3.317587f, 0.412200f, 0.000000f,
		3.128523f, 0.468227f, 0.000000f,
		2.980078f, 0.512269f, 0.000837f,
		2.851015f, 0.550663f, 0.000555f,
		2.734544f, 0.585126f, 0.002127f,
		2.625639f, 0.616902f, 0.008050f,
		2.520566f, 0.646888f, 0.020404f,
		2.417980f, 0.675474f, 0.039308f,
		2.319993f, 0.702280f, 0.062315f,
		2.228321f, 0.726976f, 0.087614f,
		2.144101f, 0.749375f, 0.113732f,
		2.068048f, 0.769382f, 0.139490f,
		1.999053f, 0.787293f, 0.165230f,
		1.935324f, 0.803566f, 0.191690f,
		1.876358f, 0.818386f, 0.218510f,
		1.821721f, 0.831915f, 0.245382f,
		1.771028f, 0.844294f, 0.272032f,
		1.723882f, 0.855615f, 0.298710f,
		1.679905f, 0.865969f, 0.325642f,
		1.638817f, 0.875464f, 0.352566f,
		1.600369f, 0.884200f, 0.379247f,
		1.564341f, 0.892263f, 0.405471f,
		1.530534f, 0.899689f, 0.431446f,
		1.498762f, 0.906515f, 0.457389f,
		1.468852f, 0.912809f, 0.483121f,
		1.440653f, 0.918631f, 0.508478f,
		1.414028f, 0.924039f, 0.533308f,
		1.388875f, 0.929047f, 0.557769f,
		1.365094f, 0.933666f, 0.582038f,
		1.342575f, 0.937941f, 0.605999f,
		1.321218f, 0.941913f, 0.629540f,
		1.300933f, 0.945620f, 0.652556f,
		1.281665f, 0.949065f, 0.675165f,
		1.263360f, 0.952251f, 0.697502f,
		1.245944f, 0.955208f, 0.719493f,
		1.229348f, 0.957964f, 0.741064f,
		1.213509f, 0.960544f, 0.762144f,
		1.198397f, 0.962949f, 0.782820f,
		1.183984f, 0.965177f, 0.803191f,
		1.170216f, 0.967249f, 0.823211f,
		1.157044f, 0.969183f, 0.842833f,
		1.144423f, 0.970999f, 0.862011f,
		1.132336f, 0.972695f, 0.880806f,
		1.120770f, 0.974267f, 0.899292f,
		1.109684f, 0.975730f, 0.917440f,
		1.099042f, 0.977098f, 0.935221f,
		1.088811f, 0.978385f, 0.952606f,
		1.078865f, 0.979599f, 0.969861f,
		1.069246f, 0.980733f, 0.986951f,
		1.060096f, 0.981779f, 1.003535f,
		1.051465f, 0.982740f, 1.019433f,
		1.043333f, 0.983624f, 1.034613f,
		1.035526f, 0.984448f, 1.049445f,
		1.027849f, 0.985226f, 1.064346f,
		1.020264f, 0.985968f, 1.079330f,
		1.012814f, 0.986676f, 1.094254f,
		1.005628f, 0.987343f, 1.108806f,
		0.998809f, 0.987955f, 1.122821f,
		0.992293f, 0.988513f, 1.136480f,
		0.986022f, 0.989028f, 1.149848f,
		0.979940f, 0.989509f, 1.162988f,
		0.973987f, 0.989969f, 1.175962f,
		0.968216f, 0.990400f, 1.188687f,
		0.962691f, 0.990791f, 1.201078f,
		0.957371f, 0.991150f, 1.213189f,
		0.952206f, 0.991485f, 1.225082f,
		0.947148f, 0.991803f, 1.236823f,
		0.942231f, 0.992101f, 1.248356f,
		0.937506f, 0.992370f, 1.259601f,
		0.932943f, 0.992616f, 1.270603f,
		0.928506f, 0.992844f, 1.281409f,
		0.924161f, 0.993059f, 1.292072f,
		0.919932f, 0.993259f, 1.302546f,
		0.915856f, 0.993438f, 1.312772f,
		0.911909f, 0.993600f, 1.322785f,
		0.908067f, 0.993749f, 1.332622f,
		0.904302f, 0.993889f, 1.342326f,
		0.900634f, 0.994017f, 1.351860f,
		0.897089f, 0.994130f, 1.361179f,
		0.893649f, 0.994231f, 1.370311f,
		0.890296f, 0.994321f, 1.379286f,
		0.887009f, 0.994405f, 1.388138f,
		0.883604f, 0.994481f, 1.397407f,
		0.880092f, 0.994549f, 1.407082f,
		0.876759f, 0.994606f, 1.416328f,
		0.873905f, 0.994653f, 1.424269f,
		0.871841f, 0.994688f, 1.429996f
	};
}

struct GridData {
	std::vector<float>    values;
	std::vector<uint32_t> indices;
	std::vector<float>    valueLUT;

	void DuplicateWithUniformValue(GridData &target, float valueChannel0, float valueChannel1, float valueChannel2) const
	{
		target.indices = indices;

		//color grid has one uniform color
		target.valueLUT.clear();
		target.valueLUT.push_back(valueChannel0);
		target.valueLUT.push_back(valueChannel1);
		target.valueLUT.push_back(valueChannel2);

		target.values.resize(values.size(), 0);
	}
};

struct GridParams
{
	float m_emissionScale;
};

class RPRVDBHeteroVolume : public IRPRVDBHeteroVolume
{
public:
	RPRVDBHeteroVolume()
	{
		m_cubeMaterial = nullptr;
		m_cubeShape = nullptr;
		m_densityGrid = nullptr;
		m_albedoGrid = nullptr;
		m_emissionGrid = nullptr;
	}
	virtual void attachToScene(rpr_scene rprScene)
	{
		rprSceneAttachHeteroVolume(rprScene, m_volume);
		rprSceneAttachShape(rprScene, m_cubeShape);
	}
	virtual void detachFromScene(rpr_scene rprScene)
	{
		rprSceneDetachShape(rprScene, m_cubeShape);
		rprSceneDetachHeteroVolume(rprScene, m_volume);
	}
	virtual void release()
	{
		rprObjectDelete(m_cubeMaterial);
		rprObjectDelete(m_cubeShape);
		rprObjectDelete(m_densityGrid);
		rprObjectDelete(m_albedoGrid);
		rprObjectDelete(m_emissionGrid);
		rprObjectDelete(m_volume);
		delete this;
	}
	virtual rpr_material_node getCubeMaterial() { return m_cubeMaterial; }
	virtual rpr_shape getCubeShape() { return m_cubeShape; }
	virtual rpr_grid getDensityGrid() { return m_densityGrid; }
	virtual rpr_grid getAlbedoGrid() { return m_albedoGrid; }
	virtual rpr_grid getEmissionGrid() { return m_emissionGrid; }
	virtual rpr_hetero_volume getHeteroVolume() { return m_volume; }
	virtual void getTranslation(float outTranslation[3]) {
		outTranslation[0] = m_translation[0];
		outTranslation[1] = m_translation[1];
		outTranslation[2] = m_translation[2];
	}
	virtual void getScale(float outScale[3]) {
		outScale[0] = m_scale[0];
		outScale[1] = m_scale[1];
		outScale[2] = m_scale[2];
	}

public:
	rpr_material_node m_cubeMaterial;
	rpr_shape         m_cubeShape;
	rpr_grid          m_densityGrid;
	rpr_grid          m_albedoGrid;
	rpr_grid          m_emissionGrid;
	rpr_hetero_volume m_volume;
	float             m_translation[3];
	float             m_scale[3];
};

static void InitializeOpenVDB()
{
	static bool bInitialized = false;
	if (!bInitialized)
	{
		openvdb::initialize();
		bInitialized = true;
	}
}

void ReadFloatGrid(openvdb::FloatGrid::Ptr grid, const openvdb::Coord &coordOffset, float valueOffset, float valueScale, std::vector<uint32_t> &outDensityGridOnIndices, std::vector<float> &outDensityGridOnValueIndices)
{
	openvdb::CoordBBox gridOnBB = grid->evalActiveVoxelBoundingBox();
	for (openvdb::FloatGrid::ValueOnIter iter = grid->beginValueOn(); iter; ++iter) 
	{
		openvdb::Coord curCoord = iter.getCoord() + coordOffset;
		outDensityGridOnIndices.push_back(curCoord.x());
		outDensityGridOnIndices.push_back(curCoord.y());
		outDensityGridOnIndices.push_back(curCoord.z());

		float value = (float)(grid->getAccessor().getValue(iter.getCoord()));
		outDensityGridOnValueIndices.push_back((value + valueOffset) * valueScale);
	}
}

#define RPR_CHECK(x) if (!(RPR_SUCCESS == x)) return nullptr;

rpr_shape CreateCubeShape(rpr_context rprContext)
{
	struct vertex
	{
		rpr_float pos[3];
		rpr_float norm[3];
		rpr_float tex[2];
	};

	// Cube geometry
	const static vertex cube_data[] =
	{
		{ -0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f, 0.f, 0.f },
		{  0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f, 0.f, 0.f },
		{  0.5f, 0.5f, 0.5f , 0.f, 1.f, 0.f, 0.f, 0.f },
		{  -0.5f, 0.5f, 0.5f , 0.f, 1.f, 0.f, 0.f, 0.f},

		{  -0.5f, -0.5f, -0.5f , 0.f, -1.f, 0.f, 0.f, 0.f },
		{  0.5f, -0.5f, -0.5f , 0.f, -1.f, 0.f, 0.f, 0.f },
		{  0.5f, -0.5f, 0.5f , 0.f, -1.f, 0.f, 0.f, 0.f },
		{  -0.5f, -0.5f, 0.5f , 0.f, -1.f, 0.f, 0.f, 0.f },

		{  -0.5f, -0.5f, 0.5f , -1.f, 0.f, 0.f, 0.f, 0.f },
		{  -0.5f, -0.5f, -0.5f , -1.f, 0.f, 0.f, 0.f, 0.f },
		{  -0.5f, 0.5f, -0.5f , -1.f, 0.f, 0.f, 0.f, 0.f },
		{  -0.5f, 0.5f, 0.5f , -1.f, 0.f, 0.f, 0.f, 0.f },

		{  0.5f, -0.5f, 0.5f ,  1.f, 0.f, 0.f, 0.f, 0.f },
		{  0.5f, -0.5f, -0.5f ,  1.f, 0.f, 0.f, 0.f, 0.f },
		{  0.5f, 0.5f, -0.5f ,  1.f, 0.f, 0.f, 0.f, 0.f },
		{  0.5f, 0.5f, 0.5f ,  1.f, 0.f, 0.f, 0.f, 0.f },

		{  -0.5f, -0.5f, -0.5f ,  0.f, 0.f, -1.f , 0.f, 0.f },
		{  0.5f, -0.5f, -0.5f ,  0.f, 0.f, -1.f , 0.f, 0.f },
		{  0.5f, 0.5f, -0.5f ,  0.f, 0.f, -1.f, 0.f, 0.f },
		{  -0.5f, 0.5f, -0.5f ,  0.f, 0.f, -1.f, 0.f, 0.f },

		{  -0.5f, -0.5f, 0.5f , 0.f, 0.f, 1.f, 0.f, 0.f },
		{  0.5f, -0.5f, 0.5f , 0.f, 0.f,  1.f, 0.f, 0.f },
		{  0.5f, 0.5f, 0.5f , 0.f, 0.f, 1.f, 0.f, 0.f },
		{  -0.5f, 0.5f, 0.5f , 0.f, 0.f, 1.f, 0.f, 0.f },
	};

	// Cube indices
	static const rpr_int indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	// Number of vertices per face
	static const rpr_int num_face_vertices[] =
	{
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
	};

	// Create cube mesh
	rpr_shape cube;
	{
		RPR_CHECK(rprContextCreateMesh(rprContext,
			(rpr_float const*)&cube_data[0], 24, sizeof(vertex),
			(rpr_float const*)((char*)&cube_data[0] + sizeof(rpr_float) * 3), 24, sizeof(vertex),
			(rpr_float const*)((char*)&cube_data[0] + sizeof(rpr_float) * 6), 24, sizeof(vertex),
			(rpr_int const*)indices, sizeof(rpr_int),
			(rpr_int const*)indices, sizeof(rpr_int),
			(rpr_int const*)indices, sizeof(rpr_int),
			num_face_vertices, 12, &cube));
	}

	return cube;
}

RPRVDBHeteroVolume* CreateHeteroVolume(rpr_context rprContext, const std::vector<uint32_t>& densityGridOnIndices, const std::vector<float>& densityGridOnValueIndices, const std::vector<float>& densityGridValues,
	const std::vector<uint32_t>& colorGridOnIndices, const std::vector<float>& colorGridOnValueIndices, const std::vector<float>& colorGridValues,
	const std::vector<uint32_t>& emissiveGridOnIndices, const std::vector<float>& emissiveGridOnValueIndices, const std::vector<float>& emissiveGridValues,
	const openvdb::math::Coord& gridSize, const GridParams& params )
{
	rpr_hetero_volume heteroVolume = nullptr;
	RPR_CHECK(rprContextCreateHeteroVolume(rprContext, &heteroVolume));

	rpr_grid rprGridDensity;
	RPR_CHECK(rprContextCreateGrid(rprContext, &rprGridDensity
		, gridSize[0], gridSize[1], gridSize[2], &densityGridOnIndices[0]
		, densityGridOnIndices.size() / 3, RPR_GRID_INDICES_TOPOLOGY_XYZ_U32
		, &densityGridOnValueIndices[0], densityGridOnValueIndices.size() * sizeof(densityGridOnValueIndices[0])
		, 0));

	rpr_grid rprGridAlbedo;
	RPR_CHECK(rprContextCreateGrid(rprContext, &rprGridAlbedo
		, gridSize[0], gridSize[1], gridSize[2], &colorGridOnIndices[0]
		, colorGridOnIndices.size() / 3, RPR_GRID_INDICES_TOPOLOGY_XYZ_U32
		, &colorGridOnValueIndices[0], colorGridOnValueIndices.size() * sizeof(colorGridOnValueIndices[0])
		, 0));

	rpr_grid rprGridEmission;
	RPR_CHECK(rprContextCreateGrid(rprContext, &rprGridEmission
		, gridSize[0], gridSize[1], gridSize[2], &emissiveGridOnIndices[0]
		, emissiveGridOnIndices.size() / 3, RPR_GRID_INDICES_TOPOLOGY_XYZ_U32
		, &emissiveGridOnValueIndices[0], emissiveGridOnValueIndices.size() * sizeof(emissiveGridOnValueIndices[0])
		, 0));

	RPR_CHECK(rprHeteroVolumeSetDensityGrid(heteroVolume, rprGridDensity));
	RPR_CHECK(rprHeteroVolumeSetDensityLookup(heteroVolume, &densityGridValues[0], (rpr_uint)densityGridValues.size() / 3));
	RPR_CHECK(rprHeteroVolumeSetAlbedoGrid(heteroVolume, rprGridAlbedo));
	RPR_CHECK(rprHeteroVolumeSetAlbedoLookup(heteroVolume, &colorGridValues[0], (rpr_uint)colorGridValues.size() / 3));
	RPR_CHECK(rprHeteroVolumeSetEmissionGrid(heteroVolume, rprGridEmission));
	rprHeteroVolumeSetEmissionScale( heteroVolume, params.m_emissionScale );
	RPR_CHECK(rprHeteroVolumeSetEmissionLookup(heteroVolume, &emissiveGridValues[0], (rpr_uint)emissiveGridValues.size() / 3));

	RPRVDBHeteroVolume *volume = new RPRVDBHeteroVolume();
	volume->m_volume = heteroVolume;
	volume->m_densityGrid = rprGridDensity;
	volume->m_albedoGrid = rprGridAlbedo;
	volume->m_emissionGrid = rprGridEmission;

	return volume;
}

RPRVDBHeteroVolume* CreateVolume(rpr_context rprContext, rpr_material_system rprMaterialSystem, const std::vector<uint32_t>& densityGridOnIndices, const std::vector<float>& densityGridOnValueIndices, const std::vector<float>& densityGridValues,
	const std::vector<uint32_t>& colorGridOnIndices, const std::vector<float>& colorGridOnValueIndices, const std::vector<float>& colorGridValues,
	const std::vector<uint32_t>& emissiveGridOnIndices, const std::vector<float>& emissiveGridOnValueIndices, const std::vector<float>& emissiveGridValues,
	const openvdb::math::Coord& gridSize, const openvdb::math::Vec3d& voxelSize, const openvdb::math::Vec3d& gridBBLow,
	const GridParams& params )
{
	//Create volume
	RPRVDBHeteroVolume* heteroVolume = CreateHeteroVolume(rprContext,
		densityGridOnIndices, densityGridOnValueIndices, densityGridValues,
		colorGridOnIndices, colorGridOnValueIndices, colorGridValues,
		emissiveGridOnIndices, emissiveGridOnValueIndices, emissiveGridValues,
		gridSize, params );
	if (!heteroVolume) {
		return nullptr;
	}

	//Create cube shape to attach volume to
	rpr_shape cube = CreateCubeShape(rprContext);
	if (!cube) {
		return nullptr;
	}
	heteroVolume->m_cubeShape = cube;

	//create material
	rpr_material_node materialTransparent = nullptr;
	RPR_CHECK(rprMaterialSystemCreateNode(rprMaterialSystem, RPR_MATERIAL_NODE_TRANSPARENT, &materialTransparent));
	RPR_CHECK(rprMaterialNodeSetInputF(materialTransparent, "color", 1.0f, 1.0f, 1.0f, 1.0f));
	RPR_CHECK(rprShapeSetMaterial(cube, materialTransparent));
	heteroVolume->m_cubeMaterial = materialTransparent;

	//attach volume to cube
	RPR_CHECK(rprShapeSetHeteroVolume(cube, heteroVolume->m_volume));

	// Set the transform 
	openvdb::math::Vec3d translation = voxelSize * gridSize.asVec3d() / 2.0 + gridBBLow;
	openvdb::math::Vec3d scale = voxelSize * gridSize.asVec3d();
	RadeonProRender::matrix m = RadeonProRender::translation(RadeonProRender::float3(float(translation[0]), float(translation[1]), float(translation[2])))
		* RadeonProRender::scale(RadeonProRender::float3(float(scale[0]), float(scale[1]), float(scale[2])));
	RPR_CHECK(rprShapeSetTransform(cube, RPR_TRUE, &m.m00));
	RPR_CHECK(rprHeteroVolumeSetTransform(heteroVolume->m_volume, RPR_TRUE, &m.m00));
	heteroVolume->m_translation[0] = float(translation[0]);
	heteroVolume->m_translation[1] = float(translation[1]);
	heteroVolume->m_translation[2] = float(translation[2]);
	heteroVolume->m_scale[0] = float(scale[0]);
	heteroVolume->m_scale[1] = float(scale[1]);
	heteroVolume->m_scale[2] = float(scale[2]);

	return heteroVolume;
}

__declspec(dllexport) IRPRVDBHeteroVolume* rprLibCreateHeterogeneousVolumeFromVDB(rpr_context rprContext, rpr_material_system rprMaterialSystem, const char* pathToTheOpenVDB, float temperatureOffset, float temperatureScale, float temperatureColorMul)
{
	InitializeOpenVDB();

	std::string vdbPathString(pathToTheOpenVDB);

	openvdb::io::File file(vdbPathString);

	try {
		file.open();
	}
	catch (openvdb::IoError e)
	{
		fprintf(stderr, "Error opening vdb file %s\n", pathToTheOpenVDB);
		return nullptr;
	}

	openvdb::GridPtrVecPtr grids = file.getGrids();

	std::set<std::string> gridNames;
	for (auto name = file.beginName(); name != file.endName(); ++name)
	{
		gridNames.insert(*name);
	}
	if (gridNames.empty())
	{
		fprintf(stderr, "vdb file %s has no grids\n", pathToTheOpenVDB);
		return nullptr;
	}

	bool hasDensity = gridNames.find("density") != gridNames.end();
	bool hasTemperature = gridNames.find("temperature") != gridNames.end();

	openvdb::FloatGrid::Ptr densityGrid = (hasDensity) ? openvdb::gridPtrCast<openvdb::FloatGrid>(file.readGrid("density")) : nullptr;
	openvdb::FloatGrid::Ptr temperatureGrid = (hasTemperature) ? openvdb::gridPtrCast<openvdb::FloatGrid>(file.readGrid("temperature")) : nullptr;

	// These parameters could potentially be passed-in from API, reserved here and just request the existing grids.
	bool bNeedColor = hasTemperature;
	bool bNeedDensity = hasDensity;
	bool bNeedEmissive = hasTemperature;

	bool bNeedToReadDensityGrid = bNeedDensity && hasDensity;
	if (bNeedToReadDensityGrid && !densityGrid.get())
	{
		fprintf(stderr, "vdb file %s density grid doesn't have float type.\n", pathToTheOpenVDB);
		bNeedToReadDensityGrid = false;
	}

	bool bNeedToReadTemperatureGrid = (bNeedColor || bNeedEmissive) && hasTemperature;
	if (bNeedToReadTemperatureGrid && !temperatureGrid.get())
	{
		fprintf(stderr, "vdb file %s temperature grid doesn't have float type.\n", pathToTheOpenVDB);
		bNeedToReadTemperatureGrid = false;
	}

	if (!bNeedToReadDensityGrid && !bNeedToReadTemperatureGrid)
	{
		fprintf(stderr, "vdb file %s does not have the needed grids.\n", pathToTheOpenVDB);
		return nullptr;
	}

	//If we need to read from both grids, check compatibility
	if (bNeedToReadDensityGrid && bNeedToReadTemperatureGrid)
	{
		if (densityGrid->voxelSize() != temperatureGrid->voxelSize())
			fprintf(stderr, "vdb file %s has different voxel sizes for density grid and temperature grid. Taking voxel size of density grid\n", pathToTheOpenVDB);
		if (densityGrid->transform() != temperatureGrid->transform())
			fprintf(stderr, "vdb file %s has different transform for density grid and temperature grid. Taking transform of density grid\n", pathToTheOpenVDB);
	}

	openvdb::Vec3d voxelSize = bNeedToReadDensityGrid ? densityGrid->voxelSize() : temperatureGrid->voxelSize();
	openvdb::math::Transform gridTransform = bNeedToReadDensityGrid ? densityGrid->transform() : temperatureGrid->transform();
	openvdb::CoordBBox gridOnBB;
	if (bNeedToReadDensityGrid)
		gridOnBB.expand(densityGrid->evalActiveVoxelBoundingBox());
	if (bNeedToReadTemperatureGrid)
		gridOnBB.expand(temperatureGrid->evalActiveVoxelBoundingBox());
	openvdb::Coord gridOnBBSize = gridOnBB.extents();

	GridData srcDensityGridData;
	GridData srcTemperatureGridData;
	GridData defaultEmissionGridData;
	GridData defaultColorGridData;
	GridData defaultDensityGridData;

	GridData *pDensityGridData = nullptr;
	GridData *pColorGridData = nullptr;
	GridData *pEmissiveGridData = nullptr;

	if (bNeedToReadDensityGrid)
	{
		float minVal, maxVal;
		densityGrid->evalMinMax(minVal, maxVal);
		float valueScale = (maxVal <= minVal) ? 1.0f : (1.0f / (maxVal - minVal));
		ReadFloatGrid(densityGrid, -gridOnBB.min(), -minVal, valueScale, srcDensityGridData.indices, srcDensityGridData.values);
		srcDensityGridData.valueLUT.push_back(minVal);
		srcDensityGridData.valueLUT.push_back(minVal);
		srcDensityGridData.valueLUT.push_back(minVal);
		srcDensityGridData.valueLUT.push_back(maxVal*1.1f);
		srcDensityGridData.valueLUT.push_back(maxVal*1.1f);
		srcDensityGridData.valueLUT.push_back(maxVal*1.1f);

		if (bNeedDensity)
			pDensityGridData = &srcDensityGridData;
	}
	if (bNeedToReadTemperatureGrid)
	{
		ReadFloatGrid(temperatureGrid, -gridOnBB.min(), temperatureOffset, 1.f, srcTemperatureGridData.indices, srcTemperatureGridData.values);
		for (int i = 0; i < (int)sizeof(temperatureToColor) / sizeof(temperatureToColor[0]); i++)
			srcTemperatureGridData.valueLUT.push_back(temperatureToColor[i]*temperatureColorMul);

//		if (bNeedColor)
//			pColorGridData = &srcTemperatureGridData;
		if (bNeedEmissive)
			pEmissiveGridData = &srcTemperatureGridData;
	}

	if (!pDensityGridData)
	{
		srcTemperatureGridData.DuplicateWithUniformValue(defaultDensityGridData, defaultDensity, defaultDensity, defaultDensity);
		pDensityGridData = &defaultDensityGridData;
	}
	if (!pEmissiveGridData)
	{
		srcDensityGridData.DuplicateWithUniformValue(defaultEmissionGridData, defaultEmission[0], defaultEmission[1], defaultEmission[2]);
		pEmissiveGridData = &defaultEmissionGridData;
	}
	if (!pColorGridData)
	{
		srcDensityGridData.DuplicateWithUniformValue(defaultColorGridData, defaultColor[0], defaultColor[1], defaultColor[2]);
		pColorGridData = &defaultColorGridData;
	}

	file.close();

	openvdb::math::Vec3d gridMin = gridTransform.indexToWorld(gridOnBB.min());
	openvdb::math::Vec3d gridBBLow = gridMin - voxelSize / 2;

	GridParams params;
	{
		params.m_emissionScale = temperatureScale/temperatureToColorMapMaxTemperature;
	}
	RPRVDBHeteroVolume *volume = CreateVolume(rprContext, rprMaterialSystem, pDensityGridData->indices, pDensityGridData->values, pDensityGridData->valueLUT,
		pColorGridData->indices, pColorGridData->values, pColorGridData->valueLUT, pEmissiveGridData->indices, pEmissiveGridData->values, pEmissiveGridData->valueLUT,
		gridOnBBSize, voxelSize, gridBBLow, params);

	{
		rpr_grid g = volume->getDensityGrid();
		if (g)
		{
			rprObjectSetName(g, (densityGrid)? densityGrid->getName().c_str(): "densityGrid");
		}
	}
	{
		rpr_grid g = volume->getEmissionGrid();
		if( g )
		{
			rprObjectSetName(g, (temperatureGrid)?temperatureGrid->getName().c_str():"temperatureGrid");
		}
	}
	{
		rpr_grid g = volume->getAlbedoGrid();
		if (g)
		{
			rprObjectSetName(g, "albedoGrid");
		}
	}
	{
		rpr_hetero_volume g = volume->getHeteroVolume();
		if (g)
			rprObjectSetName(g, "heteroVol");
	}
	{
		rpr_shape o = volume->getCubeShape();
		if (o)
			rprObjectSetName(o, "volumeCube");
	}
	return volume;
}
