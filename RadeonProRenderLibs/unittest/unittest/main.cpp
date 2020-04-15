#include <gtest/gtest.h>

#include "RadeonProRender.h"
#include "rprDeprecatedApi.h"
#include "RprLoadStore.h"
#include "Math/mathutils.h"
#include "common/common.h"

#include <cassert>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <array>

#include "../rprLibs/rprLibs.h"

#define CHECK_RPR_CALL(x) ASSERT_EQ((x), RPR_SUCCESS)

static
void setIBL( float r, float g, float b, rpr_context& context, rpr_scene& scene,
	rpr_light& light, rpr_image& image )
{
	std::array<float, 3> backgroundColor = { r,g,b };
	rpr_image_format format = { 3, RPR_COMPONENT_TYPE_FLOAT32 };
	rpr_uint imageSize = 1;
	rpr_image_desc desc = { imageSize, imageSize, 0, static_cast<rpr_uint>(imageSize * imageSize * 3 * sizeof(float)), 0 };
	std::vector<std::array<float, 3>> imageData(imageSize * imageSize, backgroundColor);

	CHECK_RPR_CALL(rprContextCreateImage(context, format, &desc, imageData.data(), &image));
	CHECK_RPR_CALL(rprContextCreateEnvironmentLight(context, &light));
	CHECK_RPR_CALL(rprEnvironmentLightSetImage(light, image));
	CHECK_RPR_CALL(rprEnvironmentLightSetIntensityScale(light, 1.0f));
	CHECK_RPR_CALL(rprSceneAttachLight(scene, light));
}

static
void saveImageToFile( const char* path, rpr_context& context, rpr_framebuffer& frame_buffer, rpr_framebuffer_desc& desc )
{
	rpr_framebuffer_format fmt = { 4, RPR_COMPONENT_TYPE_FLOAT32 };

	rpr_framebuffer frame_buffer1;
	CHECK_RPR_CALL(rprContextCreateFrameBuffer(context, fmt, &desc, &frame_buffer1));
	rpr_post_effect normalization = 0;
	CHECK_RPR_CALL( rprContextCreatePostEffect(context,RPR_POST_EFFECT_NORMALIZATION,&normalization) ); 
	CHECK_RPR_CALL( rprContextAttachPostEffect(context, normalization) ); 
	rpr_post_effect postEffectGamma = 0;
	CHECK_RPR_CALL(rprContextCreatePostEffect(context, RPR_POST_EFFECT_GAMMA_CORRECTION, &postEffectGamma));
	CHECK_RPR_CALL(rprContextAttachPostEffect(context, postEffectGamma));
	CHECK_RPR_CALL( rprContextSetParameter1f( context, "displaygamma", 2.2f ) );
	CHECK_RPR_CALL( rprContextResolveFrameBuffer( context, frame_buffer, frame_buffer1, false ) );
	CHECK_RPR_CALL( rprFrameBufferSaveToFile(frame_buffer1, path));
	rprObjectDelete( frame_buffer1 );
	rprObjectDelete( postEffectGamma );
	rprObjectDelete( normalization );
}

class RprLibTest : public ::testing::Test 
{
protected:
	RprLibTest(): m_width( 800 ), m_height( 600 )
	{
	}
	void SetUp() override 
	{
		// Indicates whether the last operation has succeeded or not
		rpr_int status = RPR_SUCCESS;
		// Create OpenCL context using a single GPU 

		// Register Tahoe ray tracing plugin.
		rpr_int tahoePluginID = rprRegisterPlugin("Tahoe64.dll");
		assert(tahoePluginID != -1);
		rpr_int plugins[] = { tahoePluginID };
		size_t pluginCount = sizeof(plugins) / sizeof(plugins[0]);

		// Create context using a single GPU 
		status = rprCreateContext(RPR_API_VERSION, plugins, pluginCount, RPR_CREATION_FLAGS_ENABLE_GPU0, NULL, NULL, &context);

		// Set active plugin.
		CHECK_RPR_CALL(rprContextSetActivePlugin(context, plugins[0]));


		CHECK_RPR_CALL(rprContextCreateMaterialSystem(context, 0, &matsys));
		// Check if it is created successfully
		ASSERT_EQ(status, RPR_SUCCESS);

		// Create a scene
		CHECK_RPR_CALL(rprContextCreateScene(context, &scene));
		// Set scene to render for the context
		CHECK_RPR_CALL(rprContextSetScene(context, scene));


		{
			CHECK_RPR_CALL(rprContextCreateCamera(context, &camera));

			// Position camera in world space: 
			// Camera position is (0,35,400)
			// Camera aimed at (0,35,0)
			// Camera up vector is (0,1,0)
			CHECK_RPR_CALL(rprCameraLookAt(camera, 0, 35, 400, 0, 35, 0, 0, 1, 0));

			CHECK_RPR_CALL(rprCameraSetFocalLength(camera, 75.f));

			// Set camera for the scene
			CHECK_RPR_CALL(rprSceneSetCamera(scene, camera));
		}

		// Create framebuffer to store rendering result
		desc.fb_width = m_width;
		desc.fb_height = m_height;

		// 4 component 32-bit float value each
		rpr_framebuffer_format fmt = { 4, RPR_COMPONENT_TYPE_FLOAT32 };
		CHECK_RPR_CALL(rprContextCreateFrameBuffer(context, fmt, &desc, &frame_buffer));

		// Clear framebuffer to black color
		CHECK_RPR_CALL(rprFrameBufferClear(frame_buffer));

		// Set framebuffer for the context
		CHECK_RPR_CALL(rprContextSetAOV(context, RPR_AOV_COLOR, frame_buffer));

		// Set max depth
		CHECK_RPR_CALL(rprContextSetParameter1u(context, "maxRecursion", 10));

	}

	void TearDown() override 
	{
		CHECK_RPR_CALL(rprObjectDelete(matsys));
		CHECK_RPR_CALL(rprObjectDelete(scene));
		CHECK_RPR_CALL(rprObjectDelete(camera));
		CHECK_RPR_CALL(rprObjectDelete(frame_buffer));
		CHECK_RPR_CALL(rprObjectDelete(context));
	}

	void transform(IRPRVDBHeteroVolume* volume, float* translation)
	{
		float t[3];
		volume->getTranslation(t);
		for (int i = 0; i < 3; i++)
			t[i] += translation[i];
		float scale[3];
		volume->getScale(scale);
		RadeonProRender::matrix m = RadeonProRender::translation(RadeonProRender::float3(float(t[0]), float(t[1]), float(t[2])))
			* RadeonProRender::scale(RadeonProRender::float3(float(scale[0]), float(scale[1]), float(scale[2])));
		CHECK_RPR_CALL(rprShapeSetTransform(volume->getCubeShape(), RPR_TRUE, &m.m00));
		CHECK_RPR_CALL(rprHeteroVolumeSetTransform(volume->getHeteroVolume(), RPR_TRUE, &m.m00));
	}

	void saveImageToFile( const char* path )
	{
		rpr_framebuffer_format fmt = { 4, RPR_COMPONENT_TYPE_FLOAT32 };

		rpr_framebuffer frame_buffer1;
		CHECK_RPR_CALL(rprContextCreateFrameBuffer(context, fmt, &desc, &frame_buffer1));
		rpr_post_effect normalization = 0;
		CHECK_RPR_CALL( rprContextCreatePostEffect(context,RPR_POST_EFFECT_NORMALIZATION,&normalization) ); 
		CHECK_RPR_CALL( rprContextAttachPostEffect(context, normalization) ); 
		rpr_post_effect postEffectGamma = 0;
		CHECK_RPR_CALL(rprContextCreatePostEffect(context, RPR_POST_EFFECT_GAMMA_CORRECTION, &postEffectGamma));
		CHECK_RPR_CALL(rprContextAttachPostEffect(context, postEffectGamma));
		CHECK_RPR_CALL( rprContextSetParameter1f( context, "displaygamma", 2.2f ) );
		CHECK_RPR_CALL( rprContextResolveFrameBuffer( context, frame_buffer, frame_buffer1, false ) );
		CHECK_RPR_CALL( rprFrameBufferSaveToFile(frame_buffer1, path));
		rprObjectDelete( frame_buffer1 );
		rprObjectDelete( postEffectGamma );
		rprObjectDelete( normalization );
	}

protected:
	rpr_context context = NULL;
	rpr_material_system matsys;
	rpr_scene scene;
	rpr_camera camera;
	rpr_framebuffer_desc desc;
	rpr_framebuffer frame_buffer;

	const int m_width;
	const int m_height;
};

TEST_F(RprLibTest, OpenVDB_Smoke)
{
	rpr_light light;rpr_image image;
	setIBL( 0.5f, 0.5f, 0.5f, context, scene, light, image );

	IRPRVDBHeteroVolume *volumeSmoke = rprLibCreateHeterogeneousVolumeFromVDB(context, matsys, "../unittest/smoke.vdb", 0.0f, 1.0f);
	ASSERT_NE(volumeSmoke, nullptr);
	volumeSmoke->attachToScene(scene);

	{
		rpr_grid g0 = volumeSmoke->getDensityGrid();
		rpr_grid g1 = volumeSmoke->getAlbedoGrid();
		rpr_grid g2 = volumeSmoke->getEmissionGrid();
		rpr_hetero_volume h = volumeSmoke->getHeteroVolume();
		rpr_shape s = volumeSmoke->getCubeShape();

		const int n = 256;
		char name[n];
		rprGridGetInfo(g0, RPR_OBJECT_NAME, n, name, 0);
		printf("density: %s\n", name);
		rprGridGetInfo(g1, RPR_OBJECT_NAME, n, name, 0);
		printf("albedo: %s\n", name);
		rprGridGetInfo(g2, RPR_OBJECT_NAME, n, name, 0);
		printf("emission: %s\n", name);
		rprHeteroVolumeGetInfo(h, RPR_OBJECT_NAME, n, name, 0);
		printf("hetero: %s\n", name);
		rprShapeGetInfo(s, RPR_OBJECT_NAME, n, name, 0);
		printf("shape: %s\n", name);
	}

	// Progressively render an image
	for (int i = 0; i < NUM_ITERATIONS; ++i)
	{
		CHECK_RPR_CALL(rprContextRender(context));
	}

	volumeSmoke->detachFromScene(scene);
	volumeSmoke->release();
	volumeSmoke = nullptr;

	saveImageToFile( "smoke.png" );

	CHECK_RPR_CALL(rprObjectDelete(image));
	CHECK_RPR_CALL(rprObjectDelete(light));
}

TEST_F(RprLibTest, OpenVDB_Fire)
{
	rpr_light light;rpr_image image;
	setIBL( 0.1f, 0.1f, 0.1f, context, scene, light, image );

	IRPRVDBHeteroVolume *volumeFire = rprLibCreateHeterogeneousVolumeFromVDB(context, matsys, "../unittest/fire.vdb", 0.0f, 100.0f, 0.9f);
	ASSERT_NE(volumeFire, nullptr);
	volumeFire->attachToScene(scene);

	// Progressively render an image
	for (int i = 0; i < NUM_ITERATIONS; ++i)
	{
		CHECK_RPR_CALL(rprContextRender(context));
	}

	volumeFire->detachFromScene(scene);
	volumeFire->release();
	volumeFire = nullptr;

	std::cout << "Rendering finished.\n";

	// Save the result to file
	saveImageToFile( "fire.png" );

	CHECK_RPR_CALL(rprObjectDelete(image));
	CHECK_RPR_CALL(rprObjectDelete(light));
}

TEST_F(RprLibTest, OpenVDB_SmokeFire)
{
	rpr_light light;rpr_image image;
	setIBL( 0.1f, 0.1f, 0.1f, context, scene, light, image );

	IRPRVDBHeteroVolume *volumeSmoke = rprLibCreateHeterogeneousVolumeFromVDB(context, matsys, "../unittest/smoke.vdb", 0.0f, 1.0f);
	ASSERT_NE(volumeSmoke, nullptr);
	volumeSmoke->attachToScene(scene);
	{
		float t[3] = { -35.f,0.f,0.f };
		transform(volumeSmoke, t);
	}

	IRPRVDBHeteroVolume *volumeFire = rprLibCreateHeterogeneousVolumeFromVDB(context, matsys, "../unittest/fire.vdb", 0.0f, 100.0f, 0.9f);
	ASSERT_NE(volumeFire, nullptr);
	volumeFire->attachToScene(scene);
	{
		float t[3] = { 35.f,0.f,0.f };
		transform(volumeFire, t);
	}
	// Progressively render an image
	for (int i = 0; i < NUM_ITERATIONS; ++i)
	{
		CHECK_RPR_CALL(rprContextRender(context));
	}

	volumeSmoke->detachFromScene(scene);
	volumeSmoke->release();
	volumeSmoke = nullptr;

	volumeFire->detachFromScene(scene);
	volumeFire->release();
	volumeFire = nullptr;

	std::cout << "Rendering finished.\n";

	// Save the result to file
	saveImageToFile( "smoke_fire.png" );

	CHECK_RPR_CALL(rprObjectDelete(image));
	CHECK_RPR_CALL(rprObjectDelete(light));
}

TEST_F(RprLibTest, OpenVDB_SmokeColored)
{
	rpr_light light;rpr_image image;
	setIBL( 0.5f, 0.5f, 0.5f, context, scene, light, image );

	IRPRVDBHeteroVolume *volumeSmokeWhite = rprLibCreateHeterogeneousVolumeFromVDB(context, matsys, "../unittest/smoke.vdb", 0.0f, 1.0f);
	ASSERT_NE(volumeSmokeWhite, nullptr);
	{ // set the white color
		std::vector<float> colorWhite{1.0f, 1.0f, 1.0f};
		CHECK_RPR_CALL(rprHeteroVolumeSetAlbedoLookup(volumeSmokeWhite->getHeteroVolume(), &colorWhite[0], (rpr_uint)colorWhite.size()/3));
	}
	volumeSmokeWhite->attachToScene(scene);
	{
		float t[3] = { -35.f,0.f,0.f };
		transform(volumeSmokeWhite, t);
	}
	IRPRVDBHeteroVolume *volumeSmokeBlue = rprLibCreateHeterogeneousVolumeFromVDB(context, matsys, "../unittest/smoke.vdb", 0.0f, 1.0f);
	ASSERT_NE(volumeSmokeBlue, nullptr);
	{ // set the blue color
		std::vector<float> colorBlue{0.5f, 0.9f, 0.9f};
		CHECK_RPR_CALL(rprHeteroVolumeSetAlbedoLookup(volumeSmokeBlue->getHeteroVolume(), &colorBlue[0], (rpr_uint)colorBlue.size()/3));
	}
	volumeSmokeBlue->attachToScene(scene);
	{
		float t[3] = { 35.f,0.f,0.f };
		transform(volumeSmokeBlue, t);
	}
	// Progressively render an image
	for (int i = 0; i < NUM_ITERATIONS; ++i)
	{
		CHECK_RPR_CALL(rprContextRender(context));
	}

	volumeSmokeWhite->detachFromScene(scene);
	volumeSmokeWhite->release();
	volumeSmokeWhite = nullptr;

	volumeSmokeBlue->detachFromScene(scene);
	volumeSmokeBlue->release();
	volumeSmokeBlue = nullptr;

	// Save the result to file
	saveImageToFile( "smoke_colored.png" );

	CHECK_RPR_CALL(rprObjectDelete(image));
	CHECK_RPR_CALL(rprObjectDelete(light));
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}