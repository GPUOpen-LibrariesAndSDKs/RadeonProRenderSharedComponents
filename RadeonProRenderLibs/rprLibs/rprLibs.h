#include <RadeonProRender.h>

#ifdef RPRLIB_DLL
	#define RPRLIB_API __declspec(dllexport)
#else
	#define RPRLIB_API __declspec(dllimport)
#endif

class IRPRVDBHeteroVolume {
public:
	virtual void attachToScene(rpr_scene rprScene) = 0;
	virtual void detachFromScene(rpr_scene rprScene) = 0;
	virtual void release() = 0;
	virtual rpr_material_node getCubeMaterial() = 0;
	virtual rpr_shape getCubeShape() = 0;
	virtual rpr_grid getDensityGrid() = 0;
	virtual rpr_grid getAlbedoGrid() = 0;
	virtual rpr_grid getEmissionGrid() = 0;
	virtual rpr_hetero_volume getHeteroVolume() = 0;
	virtual void getTranslation(float outTranslation[3]) = 0;
	virtual void getScale(float outScale[3]) = 0;
};

extern "C" {
	RPRLIB_API IRPRVDBHeteroVolume* rprLibCreateHeterogeneousVolumeFromVDB(rpr_context rprContext, rpr_material_system rprMaterialSystem, const char* pathToTheOpenVDB, float temperatureOffset, float temperatureScale, float temperatureColorMul = 1.f);
}
