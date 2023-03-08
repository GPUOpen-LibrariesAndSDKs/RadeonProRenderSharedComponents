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
#include "ImageFilter.h"

#include "RadeonProRender_CL.h"
#include "RadeonImageFilters_cl.h"
#include "RadeonImageFilters_metal.h"
#include "RadeonProRender_Metal.h"

#include <vector>
#include <cassert>
#include <stdexcept>

static bool HasGpuContext(rpr_creation_flags contextFlags)
{
#define GPU(x) RPR_CREATION_FLAGS_ENABLE_GPU##x

	rpr_creation_flags gpuMask = GPU(0) | GPU(1) | GPU(2) | GPU(3) | GPU(4) | GPU(5) | GPU(6) | GPU(7) |
		GPU(8) | GPU(9) | GPU(10) | GPU(11) | GPU(12) | GPU(13) | GPU(14) | GPU(15);

#undef GPU

	bool hasGpuContext = (contextFlags & gpuMask) != 0;

	return hasGpuContext;
}

static rpr_int GpuDeviceIdUsed(rpr_creation_flags contextFlags)
{
#define GPU(x) RPR_CREATION_FLAGS_ENABLE_GPU##x

	std::vector<rpr_int> gpu_ids =
	{
		GPU(0), GPU(1), GPU(2), GPU(3), GPU(4), GPU(5), GPU(6), GPU(7), GPU(8), GPU(9),
		GPU(10), GPU(11), GPU(12), GPU(13), GPU(14), GPU(15)
	};

#undef GPU

	for (rpr_int i = 0; i < gpu_ids.size(); i++ )
	{	
		if ((contextFlags & gpu_ids[i]) != 0)
			return i;
	}
	
	return -1;
}

std::vector<rpr_char> GetRprCachePath(rpr_context rprContext);

ImageFilter::ImageFilter(const rpr_context rprContext, std::uint32_t width, std::uint32_t height, const std::string& modelsPath, bool forceCPUContext /*= false*/) :
	mWidth(width),
	mHeight(height),
	mModelsPath(modelsPath),
	mInputOverrideWidth(0),
	mInputOverrideHeight(0)
{
	rpr_creation_flags contextFlags = 0;
	rpr_int rprStatus = rprContextGetInfo(rprContext, RPR_CONTEXT_CREATION_FLAGS, sizeof(rpr_creation_flags), &contextFlags, nullptr);
	assert(RPR_SUCCESS == rprStatus);
	
	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get context parameters.");

	std::vector<rpr_char> path = GetRprCachePath(rprContext);
	bool haveNoCache = path.empty() || (path[0] == (rpr_char)'\0');

	if (forceCPUContext)
	{
		mRifContext.reset(new RifContextCPU(rprContext));
	}
	else if (contextFlags & RPR_CREATION_FLAGS_ENABLE_METAL)
	{
		mRifContext.reset( new RifContextGPUMetal(rprContext) );
	}
	else if (HasGpuContext(contextFlags) && !haveNoCache)
	{
		mRifContext.reset(new RifContextGPU(rprContext));
	}
	else
	{
		mRifContext.reset(new RifContextCPU(rprContext));
	}

	rif_image_desc desc = { mWidth, mHeight, 0, 0, 0, 4, RIF_COMPONENT_TYPE_FLOAT32 };

	mRifContext->CreateOutput(desc);
}

ImageFilter::~ImageFilter()
{
	if (mRifFilter != nullptr)
	{
		mRifFilter->DetachFilter( mRifContext.get() );
	}
}

void ImageFilter::CreateFilter(RifFilterType rifFilteType, bool useOpenImageDenoise)
{
	switch (rifFilteType)
	{
	case RifFilterType::BilateralDenoise:
		mRifFilter.reset( new RifFilterBilateral( mRifContext.get() ) );
		break;

	case RifFilterType::LwrDenoise:
		mRifFilter.reset( new RifFilterLwr( mRifContext.get(), mWidth, mHeight) );
		break;

	case RifFilterType::EawDenoise:
		mRifFilter.reset( new RifFilterEaw( mRifContext.get(), mWidth, mHeight) );
		break;

	case RifFilterType::MlDenoise:
		mRifFilter.reset( new RifFilterMl(mRifContext.get(), mWidth, mHeight, mModelsPath, useOpenImageDenoise) );
		break;

	case RifFilterType::MlDenoiseColorOnly:
		mRifFilter.reset(new RifFilterMlColorOnly(mRifContext.get(), mWidth, mHeight, mModelsPath, useOpenImageDenoise));
		break;

	case RifFilterType::ShadowCatcher:
		mRifFilter.reset(new RifFilterShadowCatcher(mRifContext.get(), mWidth, mHeight, mModelsPath, useOpenImageDenoise));
		break;

	case RifFilterType::ReflectionCatcher:
		mRifFilter.reset(new RifFilterReflectionCatcher(mRifContext.get(), mWidth, mHeight, mModelsPath, useOpenImageDenoise));
		break;

	case RifFilterType::ShadowReflectionCatcher:
		mRifFilter.reset(new RifFilterShadowReflectionCatcher(mRifContext.get(), mWidth, mHeight, mModelsPath, useOpenImageDenoise));
		break;

	case RifFilterType::Upscaler:
		mRifFilter.reset(new RifFilterUpscaler(mRifContext.get(), mWidth, mHeight, mModelsPath));
		break;

	case RifFilterType::LinearTonemap:
		mRifFilter.reset(new RifFilterLinearTonemap(mRifContext.get(), mWidth, mHeight, mModelsPath));
		break;

	case RifFilterType::PhotoLinearTonemap:
		mRifFilter.reset(new RifFilterPhotoLinearTonemap(mRifContext.get(), mWidth, mHeight, mModelsPath));
		break;

	case RifFilterType::AutoLinearTonemap:
		mRifFilter.reset(new RiffilterAutoLinearTonemap(mRifContext.get(), mWidth, mHeight, mModelsPath));
		break;

	case RifFilterType::MaxWhiteTonemap:
		mRifFilter.reset(new RifFilterMaxWhiteTonemap(mRifContext.get(), mWidth, mHeight, mModelsPath));
		break;

	case RifFilterType::ReinhardTonemap:
		mRifFilter.reset(new RifFilterReinhardTonemap(mRifContext.get(), mWidth, mHeight, mModelsPath));
		break;

	default:
		assert("Unknown filter type");
	}
}

void ImageFilter::DeleteFilter()
{
	if (mRifFilter != nullptr)
	{
		mRifFilter->DetachFilter( mRifContext.get() );
	}
}

void ImageFilter::AddInput(RifFilterInput inputId, const rpr_framebuffer rprFrameBuffer, float sigma) const
{
	rif_image_desc desc = { mWidth, mHeight, 0, 0, 0, 4, RIF_COMPONENT_TYPE_FLOAT32 };

	rif_image rifImage = mRifContext->CreateRifImage(rprFrameBuffer, desc);

	if (typeid(*mRifContext) == typeid(RifContextGPU) || typeid(*mRifContext) == typeid(RifContextGPUMetal))
	{
		mRifFilter->AddInput(inputId, rifImage, sigma);
	}
	else
	{
		mRifFilter->AddInput(inputId, rifImage, rprFrameBuffer, sigma);
	}
}

void ImageFilter::AddInput(RifFilterInput inputId, float* memPtr, size_t size, float sigma) const
{
	rif_image_desc desc = { mInputOverrideWidth > 0 ? mInputOverrideWidth : mWidth, 
							mInputOverrideHeight > 0 ? mInputOverrideHeight : mHeight, 
							0, 0, 0, 4, RIF_COMPONENT_TYPE_FLOAT32 };

	rif_image rifImage = nullptr;

	rif_int rifStatus = rifContextCreateImage(mRifContext->Context(), &desc, nullptr, &rifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF image.");

	mRifFilter->AddInput(inputId, rifImage, memPtr, size, sigma);
}

void ImageFilter::AddParam(std::string name, RifParam param) const
{
	mRifFilter->AddParam(name, param);
}

void ImageFilter::AttachFilter() const
{
	mRifFilter->AttachFilter( mRifContext.get() );
	mRifFilter->ApplyParameters();
}

void ImageFilter::Run() const
{
	rif_int rifStatus = RIF_SUCCESS;

	mRifContext->UpdateInputs( mRifFilter.get() );

	rifStatus = rifContextExecuteCommandQueue(mRifContext->Context(), mRifContext->Queue(), nullptr, nullptr, nullptr);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to execute queue.");
}

std::vector<float> ImageFilter::GetData() const
{
	float* output = nullptr;

	rif_int rifStatus = rifImageMap(mRifContext->Output(), RIF_IMAGE_MAP_READ, (void**) &output);
	assert(RIF_SUCCESS == rifStatus && output != nullptr);

	if (RIF_SUCCESS != rifStatus || nullptr == output)
		throw std::runtime_error("RPR denoiser failed to map output data.");

	std::vector<float> floatData(output, output + mWidth * mHeight * 4 );

	rifStatus = rifImageUnmap(mRifContext->Output(), output);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to unmap output data.");

	return std::move(floatData);
}

void ImageFilter::SetInputOverrideSize(std::uint32_t width, std::uint32_t height)
{
	mInputOverrideWidth = width;
	mInputOverrideHeight = height;
}

RifContextWrapper::~RifContextWrapper()
{
	rif_int rifStatus = RIF_SUCCESS;
	
	if (mOutputRifImage != nullptr)
	{
		rifStatus = rifObjectDelete(mOutputRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (mRifCommandQueueHandle != nullptr)
	{
		rifStatus = rifObjectDelete(mRifCommandQueueHandle);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (mRifContextHandle != nullptr)
	{
		rifStatus = rifObjectDelete(mRifContextHandle);
		assert(RIF_SUCCESS == rifStatus);
	}
}

const rif_context RifContextWrapper::Context() const
{
	return mRifContextHandle;
}

const rif_command_queue RifContextWrapper::Queue() const
{
	return mRifCommandQueueHandle;
}

const rif_image RifContextWrapper::Output() const
{
	return mOutputRifImage;
}

void RifContextWrapper::CreateOutput(const rif_image_desc& desc)
{
	rif_int rifStatus = rifContextCreateImage(mRifContextHandle, &desc, nullptr, &mOutputRifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create output image.");
}

std::vector<rpr_char> GetRprCachePath(rpr_context rprContext)
{
	size_t length;
	rpr_status rprStatus = rprContextGetInfo(rprContext, RPR_CONTEXT_CACHE_PATH, sizeof(size_t), nullptr, &length);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get cache path.");

	std::vector<rpr_char> path(length);
	rprStatus = rprContextGetInfo(rprContext, RPR_CONTEXT_CACHE_PATH, path.size(), &path[0], nullptr);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get cache path.");

	return std::move(path);
}



void RifContextWrapper::UpdateInputs(const RifFilterWrapper* rifFilter) const
{
	for (const auto& input : rifFilter->mInputs)
	{
		input.second->Update();
	}
}

RifContextGPU::RifContextGPU(const rpr_context rprContext)
{
#if defined(_WIN32) || defined(__linux__)
	int deviceCount = 0;

	rif_int rifStatus = rifGetDeviceCount(rifBackendApiType, &deviceCount);
	assert(RIF_SUCCESS == rifStatus);
	assert(deviceCount != 0);

	if (RIF_SUCCESS != rifStatus || 0 == deviceCount)
		throw std::runtime_error("RPR denoiser hasn't found compatible devices.");

	rpr_cl_context clContext;
	rpr_int rprStatus = rprContextGetInfo(rprContext, RPR_CL_CONTEXT, sizeof(rpr_cl_context), &clContext, nullptr);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get CL device context.");

	rpr_cl_device clDevice;
	rprStatus = rprContextGetInfo(rprContext, RPR_CL_DEVICE, sizeof(rpr_cl_device), &clDevice, nullptr);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get CL device.");

	rpr_cl_command_queue clCommandQueue;
	rprStatus = rprContextGetInfo(rprContext, RPR_CL_COMMAND_QUEUE, sizeof(rpr_cl_command_queue), &clCommandQueue, nullptr);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get CL command queue.");

	std::vector<rpr_char> path = GetRprCachePath(rprContext);

	rifStatus = rifCreateContextFromOpenClContext(RIF_API_VERSION, clContext, clDevice, clCommandQueue, path.data(), &mRifContextHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF context.");

	rifStatus = rifContextCreateCommandQueue(mRifContextHandle, &mRifCommandQueueHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF command queue.");
#else
    throw std::runtime_error("RifContextGPU is not implemented on OSX");
#endif
}

RifContextGPU::~RifContextGPU()
{
}

rif_image RifContextGPU::CreateRifImage(const rpr_framebuffer rprFrameBuffer, const rif_image_desc& desc) const
{
#if defined(_WIN32) || defined(__linux__)
	rif_image rifImage = nullptr;
	rpr_cl_mem clMem = nullptr;

	rpr_int rprStatus = rprFrameBufferGetInfo(rprFrameBuffer, RPR_CL_MEM_OBJECT, sizeof(rpr_cl_mem), &clMem, nullptr);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get frame buffer info.");

	rif_int rifStatus = rifContextCreateImageFromOpenClMemory(mRifContextHandle , &desc, clMem, &rifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to get frame buffer info.");

	return rifImage;
#else
    throw std::runtime_error("RifContextGPU is not implemented on OSX");
    return nullptr;
#endif
}



RifContextCPU::RifContextCPU(const rpr_context rprContext)
{
	int deviceCount = 0;
	rif_int rifStatus = rifGetDeviceCount(rifBackendApiType, &deviceCount);
	assert(RIF_SUCCESS == rifStatus);
	assert(deviceCount != 0);

	if (RIF_SUCCESS != rifStatus || 0 == deviceCount)
		throw std::runtime_error("RPR denoiser hasn't found compatible devices.");

	std::vector<rpr_char> path = GetRprCachePath(rprContext);

	rifStatus = rifCreateContext(RIF_API_VERSION, rifBackendApiType, 0, path.data(), &mRifContextHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF context.");

	rifStatus = rifContextCreateCommandQueue(mRifContextHandle, &mRifCommandQueueHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF command queue.");
}

RifContextCPU::~RifContextCPU()
{
}

rif_image RifContextCPU::CreateRifImage(const rpr_framebuffer rprFrameBuffer, const rif_image_desc& desc) const
{
	rif_image rifImage = nullptr;

	rif_int rifStatus = rifContextCreateImage(mRifContextHandle, &desc, nullptr, &rifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF image.");

	return rifImage;
}



RifContextGPUMetal::RifContextGPUMetal(const rpr_context rprContext)
{
#ifdef __APPLE__
#ifdef ENABLE_GET_APPLE_DEVICE
	int deviceCount = 0;
	rif_int rifStatus = rifGetDeviceCount(rifBackendApiType, &deviceCount);
	assert(RIF_SUCCESS == rifStatus);
	assert(deviceCount != 0);

	if (RIF_SUCCESS != rifStatus || 0 == deviceCount)
		throw std::runtime_error("RPR denoiser hasn't found compatible devices.");
	
	rpr_creation_flags contextFlags = 0;
	rpr_int rprStatus = rprContextGetInfo(rprContext, RPR_CONTEXT_CREATION_FLAGS, sizeof(rpr_creation_flags), &contextFlags, nullptr);
	assert(RPR_SUCCESS == rprStatus);
    
    
    rpr_metal_device pMetalDevice = nullptr;
    rpr_metal_command_queue pMetalCommandQueue = nullptr;
    
    rprStatus = rprContextGetInfo(rprContext, RPR_METAL_DEVICE, sizeof(rpr_metal_device), &pMetalDevice, nullptr);
    assert(RIF_SUCCESS == rifStatus);
    assert(pMetalDevice != nullptr);

    rprStatus = rprContextGetInfo(rprContext, RPR_METAL_COMMAND_QUEUE, sizeof(rpr_metal_command_queue), &pMetalCommandQueue, nullptr);
    assert(RIF_SUCCESS == rifStatus);
    assert(pMetalCommandQueue != nullptr);
    
    
	std::vector<rpr_char> path = GetRprCachePath(rprContext);

	rifStatus = rifCreateContextFromMetalContext(RIF_API_VERSION, pMetalDevice, pMetalCommandQueue, path.data(), &mRifContextHandle);
    
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF context.");
    
	rifStatus = rifContextCreateCommandQueue(mRifContextHandle, &mRifCommandQueueHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF command queue.");
#else
	int deviceCount = 0;
	rif_int rifStatus = rifGetDeviceCount(rifBackendApiType, &deviceCount);
	assert(RIF_SUCCESS == rifStatus);
	assert(deviceCount != 0);

	if (RIF_SUCCESS != rifStatus || 0 == deviceCount)
		throw std::runtime_error("RPR denoiser hasn't found compatible devices.");

	std::vector<rpr_char> path = GetRprCachePath(rprContext);

	rifStatus = rifCreateContext(RIF_API_VERSION, rifBackendApiType, 0, path.data(), &mRifContextHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF context.");

	rifStatus = rifContextCreateCommandQueue(mRifContextHandle, &mRifCommandQueueHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create RIF command queue.");
#endif
#endif
}

RifContextGPUMetal::~RifContextGPUMetal()
{
}

rif_image RifContextGPUMetal::CreateRifImage(const rpr_framebuffer rprFrameBuffer, const rif_image_desc& desc) const
{
	rif_image rifImage = nullptr;
#if defined(__APPLE__)
	rpr_cl_mem clMem = nullptr;

	rpr_int rprStatus = rprFrameBufferGetInfo(rprFrameBuffer, RPR_CL_MEM_OBJECT, sizeof(rpr_cl_mem), &clMem, nullptr);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get frame buffer info.");

	
	size_t fbSize;
	rprStatus = rprFrameBufferGetInfo(rprFrameBuffer, RPR_FRAMEBUFFER_DATA, 0, NULL, &fbSize);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to acquire frame buffer info.");

	rif_int rifStatus = rifContextCreateImageFromMetalMemory(mRifContextHandle , &desc, clMem, fbSize, &rifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to get frame buffer info.");
#endif
	return rifImage;
}



RifInput::RifInput(rif_image rifImage, float sigma) :
	mRifImage(rifImage),
	mSigma(sigma)
{
}

RifInput::~RifInput()
{
	rif_int rifStatus = rifObjectDelete(mRifImage);
	assert(RIF_SUCCESS == rifStatus);
}

RifInputGPU::RifInputGPU(rif_image rifImage, float sigma) :
	RifInput(rifImage, sigma)
{
}

RifInputGPU::~RifInputGPU()
{
}

void RifInputGPU::Update()
{
}

RifInputGPUCPU::RifInputGPUCPU(rif_image rifImage, const rpr_framebuffer rprFrameBuffer, float sigma) :
	RifInput(rifImage, sigma),
	mRprFrameBuffer(rprFrameBuffer)
{
}

RifInputGPUCPU::~RifInputGPUCPU()
{
}

void RifInputGPUCPU::Update()
{
	size_t sizeInBytes = 0;
	size_t retSize = 0;
	void* imageData = nullptr;

	// verify image size
	rif_int rifStatus = rifImageGetInfo(mRifImage, RIF_IMAGE_DATA_SIZEBYTE, sizeof(size_t), (void*) &sizeInBytes, &retSize);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to get RIF image info.");

	size_t fbSize;
	rpr_int rprStatus = rprFrameBufferGetInfo(mRprFrameBuffer, RPR_FRAMEBUFFER_DATA, 0, NULL, &fbSize);
	assert(RPR_SUCCESS == rprStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to acquire frame buffer info.");

	assert(sizeInBytes == fbSize);

	if (sizeInBytes != fbSize)
		throw std::runtime_error("RPR denoiser failed to match RIF image and frame buffer sizes.");

	// resolve framebuffer data to rif image
	rifStatus = rifImageMap(mRifImage, RIF_IMAGE_MAP_WRITE, &imageData);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus || nullptr == imageData)
		throw std::runtime_error("RPR denoiser failed to acquire RIF image.");

	rprStatus = rprFrameBufferGetInfo(mRprFrameBuffer, RPR_FRAMEBUFFER_DATA, fbSize, imageData, NULL);
	assert(RPR_SUCCESS == rprStatus);

	// try to unmap at first, then raise a possible error

	rifStatus = rifImageUnmap(mRifImage, imageData);
	assert(RIF_SUCCESS == rifStatus);

	if (RPR_SUCCESS != rprStatus)
		throw std::runtime_error("RPR denoiser failed to get data from frame buffer.");

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to unmap output data.");
}

RifInputCPU::RifInputCPU(rif_image rifImage, float* memPtr, size_t size, float sigma) :
	RifInput(rifImage, sigma),
	mMemPtr(memPtr),
	mSize(size)
{
}

RifInputCPU::~RifInputCPU()
{
}

void RifInputCPU::Update()
{
	size_t sizeInBytes = 0;
	size_t retSize = 0;
	void* imageData = nullptr;

	// verify image size
	rif_int rifStatus = rifImageGetInfo(mRifImage, RIF_IMAGE_DATA_SIZEBYTE, sizeof(size_t), (void*) &sizeInBytes, &retSize);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to get RIF image info.");

	assert(sizeInBytes == mSize);

	// copy data to rif image
	rifStatus = rifImageMap(mRifImage, RIF_IMAGE_MAP_WRITE, &imageData);
	assert(RIF_SUCCESS == rifStatus);

	std::memcpy(imageData, mMemPtr, mSize);

	rifStatus = rifImageUnmap(mRifImage, imageData);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to unmap output data.");
}



RifFilterWrapper::~RifFilterWrapper()
{
	rif_int rifStatus = RIF_SUCCESS;

	mInputs.clear();

	for (const rif_image& auxImage : mAuxImages)
	{
		rifStatus = rifObjectDelete(auxImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	for (const rif_image_filter& auxFilter : mAuxFilters)
	{
		rifStatus = rifObjectDelete(auxFilter);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (mRifImageFilterHandle != nullptr)
	{
		rifStatus = rifObjectDelete(mRifImageFilterHandle);
		assert(RIF_SUCCESS == rifStatus);
	}
}

void RifFilterWrapper::AddInput(RifFilterInput inputId, const rif_image rifImage, float sigma)
{
	RifInputPtr input = std::make_shared<RifInputGPU>(rifImage, sigma);
	mInputs[inputId] = input;
}

void RifFilterWrapper::AddInput(RifFilterInput inputId, const rif_image rifImage, const rpr_framebuffer rprFrameBuffer, float sigma)
{
	RifInputPtr input = std::make_shared<RifInputGPUCPU>(rifImage, rprFrameBuffer, sigma);
	mInputs[inputId] = input;
}

void RifFilterWrapper::AddInput(RifFilterInput inputId, const rif_image rifImage, float* memPtr, size_t size, float sigma)
{
	RifInputPtr input = std::make_shared<RifInputCPU>(rifImage, memPtr, size, sigma);
	mInputs[inputId] = input;
}

void RifFilterWrapper::AddParam(std::string name, RifParam param)
{
	mParams[name] = param;
}

void RifFilterWrapper::DetachFilter(const RifContextWrapper* rifContext) noexcept
{
	rif_int rifStatus = RIF_SUCCESS;

	for (const rif_image_filter& auxFilter : mAuxFilters)
	{
		rifStatus = rifCommandQueueDetachImageFilter(rifContext->Queue(), auxFilter);
		assert(RIF_SUCCESS == rifStatus);
	}

	rifStatus = rifCommandQueueDetachImageFilter(rifContext->Queue(), mRifImageFilterHandle);
	assert(RIF_SUCCESS == rifStatus);
}

void RifFilterWrapper::SetupVarianceImageFilter(const rif_image_filter inputFilter, const rif_image outVarianceImage) const
{
	rif_int rifStatus = rifImageFilterSetParameterImage(inputFilter, "positionsImg", mInputs.at(RifWorldCoordinate)->mRifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(inputFilter, "normalsImg", mInputs.at(RifNormal)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(inputFilter, "meshIdsImg", mInputs.at(RifObjectId)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(inputFilter, "outVarianceImg", outVarianceImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to setup variance filter.");
}

void RifFilterWrapper::ApplyParameters() const
{
	rif_int rifStatus = RIF_SUCCESS;

	for (const auto& param : mParams)
	{
		switch (param.second.mType)
		{
		case RifParamType::RifInt:
			rifStatus = rifImageFilterSetParameter1u(mRifImageFilterHandle, param.first.c_str(), param.second.mData.i);
			break;

		case RifParamType::RifFloat:
			rifStatus = rifImageFilterSetParameter1f(mRifImageFilterHandle, param.first.c_str(), param.second.mData.f);
			break;

		case RifParamType::RifOther: // don't apply such parameter to image filter
			if (param.first == "enable16bitCompute")
			{
				rifStatus = rifImageFilterSetComputeType(mRifImageFilterHandle, (param.second.mData.i == 0) ? RIF_COMPUTE_TYPE_FLOAT : RIF_COMPUTE_TYPE_HALF);
			}
			break;
		}

		assert(RIF_SUCCESS == rifStatus);

		if (RIF_SUCCESS != rifStatus)
			throw std::runtime_error("RPR denoiser failed to apply parameters.");
	}
}



RifFilterBilateral::RifFilterBilateral(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_BILATERAL_DENOISE, &mRifImageFilterHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create Bilateral filter.");
}

RifFilterBilateral::~RifFilterBilateral()
{
}

void RifFilterBilateral::AttachFilter(const RifContextWrapper* rifContext)
{
	for (const auto& input : mInputs)
	{
		inputImages.push_back(input.second->mRifImage);
		sigmas.push_back(input.second->mSigma);
	}

	rif_int rifStatus = rifImageFilterSetParameterImageArray( mRifImageFilterHandle, "inputs", &inputImages[0],
		static_cast<rif_int>( inputImages.size() ) );
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterFloatArray( mRifImageFilterHandle, "sigmas", &sigmas[0],
			static_cast<rif_int>( sigmas.size() ) );
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter1u( mRifImageFilterHandle, "inputsNum",
			static_cast<rif_int>( inputImages.size() ) );
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to apply parameters.");

	rifStatus = rifCommandQueueAttachImageFilter( rifContext->Queue(), mRifImageFilterHandle, 
		mInputs.at(RifColor)->mRifImage, rifContext->Output() );
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");
}



RifFilterLwr::RifFilterLwr(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height)
{
	// main LWR filter
	rif_int rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_LWR_DENOISE, &mRifImageFilterHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create LWR filter.");

	// auxillary LWR filters
	mAuxFilters.resize(AuxFilterMax, nullptr);

	for (rif_image_filter& auxFilter : mAuxFilters)
	{
		rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_TEMPORAL_ACCUMULATOR, &auxFilter);
		assert(RIF_SUCCESS == rifStatus);

		if (RIF_SUCCESS != rifStatus)
			throw std::runtime_error("RPR denoiser failed to create auxillary filter.");
	}

	// auxillary LWR images
	rif_image_desc desc = { width, height, 0, 0, 0, 4, RIF_COMPONENT_TYPE_FLOAT32 };

	mAuxImages.resize(AuxImageMax, nullptr);

	for (rif_image& auxImage : mAuxImages)
	{
		rifStatus = rifContextCreateImage(rifContext->Context(), &desc, nullptr, &auxImage);
		assert(RIF_SUCCESS == rifStatus);

		if (RIF_SUCCESS != rifStatus)
			throw std::runtime_error("RPR denoiser failed to create auxillary image.");
	}
}

RifFilterLwr::~RifFilterLwr()
{
}

void RifFilterLwr::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// make variance image filters
	SetupVarianceImageFilter(mAuxFilters[ColorVar], mAuxImages[ColorVarianceImage]);

	SetupVarianceImageFilter(mAuxFilters[NormalVar], mAuxImages[NormalVarianceImage]);

	SetupVarianceImageFilter(mAuxFilters[DepthVar], mAuxImages[DepthVarianceImage]);

	SetupVarianceImageFilter(mAuxFilters[TransVar], mAuxImages[TransVarianceImage]);

	// Configure Filter
	rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "vColorImg", mAuxImages[ColorVarianceImage]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "normalsImg", mInputs.at(RifNormal)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "vNormalsImg", mAuxImages[NormalVarianceImage]);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "depthImg", mInputs.at(RifDepth)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "vDepthImg", mAuxImages[DepthVarianceImage]);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "transImg", mInputs.at(RifTrans)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}
	
	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "vTransImg", mAuxImages[TransVarianceImage]);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to apply parameters.");

	// attach filters
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[TransVar],
		mInputs.at(RifTrans)->mRifImage, mAuxImages[TransVarianceImage]);
	assert(RIF_SUCCESS == rifStatus);


	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[DepthVar],
			mInputs.at(RifDepth)->mRifImage, mAuxImages[DepthVarianceImage]);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[NormalVar],
			mInputs.at(RifNormal)->mRifImage, mAuxImages[NormalVarianceImage]);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[ColorVar],
			mInputs.at(RifColor)->mRifImage, mAuxImages[ColorVarianceImage]);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
			mInputs.at(RifColor)->mRifImage, rifContext->Output());
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");
}



RifFilterEaw::RifFilterEaw(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height)
{
	// main EAW filter
	rif_int rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_EAW_DENOISE, &mRifImageFilterHandle);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create EAW filter.");

	// auxillary EAW filters
	mAuxFilters.resize(AuxFilterMax, nullptr);

	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_TEMPORAL_ACCUMULATOR, &mAuxFilters[ColorVar]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create auxillary filter.");

	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_MLAA, &mAuxFilters[Mlaa]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create auxillary filter.");

	// auxillary rif images
	rif_image_desc desc = { width, height, 0, 0, 0, 4, RIF_COMPONENT_TYPE_FLOAT32 };

	mAuxImages.resize(AuxImageMax, nullptr);

	rifStatus = rifContextCreateImage(rifContext->Context(), &desc, nullptr, &mAuxImages[ColorVarianceImage]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create auxillary image.");

	rifStatus = rifContextCreateImage(rifContext->Context(), &desc, nullptr, &mAuxImages[DenoisedOutputImage]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create auxillary image.");
}

RifFilterEaw::~RifFilterEaw()
{
}

void RifFilterEaw::AttachFilter(const RifContextWrapper* rifContext)
{
	// setup inputs
	rif_int rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "normalsImg", mInputs.at(RifNormal)->mRifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "transImg", mInputs.at(RifTrans)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "colorVar", mInputs.at(RifColor)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	// setup sigmas
	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter1f(mRifImageFilterHandle, "colorSigma", mInputs.at(RifColor)->mSigma);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter1f(mRifImageFilterHandle, "normalSigma", mInputs.at(RifNormal)->mSigma);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter1f(mRifImageFilterHandle, "depthSigma", mInputs.at(RifDepth)->mSigma);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter1f(mRifImageFilterHandle, "transSigma", mInputs.at(RifTrans)->mSigma);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to apply parameters.");

	// setup color variance filter
	SetupVarianceImageFilter(mAuxFilters[ColorVar], mAuxImages[ColorVarianceImage]);

	// setup MLAA filter
	rifStatus = rifImageFilterSetParameterImage(mAuxFilters[Mlaa], "normalsImg", mInputs.at(RifNormal)->mRifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to apply parameters.");

	rifStatus = rifImageFilterSetParameterImage(mAuxFilters[Mlaa], "meshIDImg", mInputs.at(RifObjectId)->mRifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to apply parameters.");

	// attach filters
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[ColorVar],
		mInputs.at(RifColor)->mRifImage, rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");

	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle, rifContext->Output(),
		mAuxImages[DenoisedOutputImage]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");

	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[Mlaa], mAuxImages[DenoisedOutputImage],
		rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");
}



RifFilterMl::RifFilterMl(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath, bool useOpenImageDenoise)
{
	rif_int rifStatus = RIF_SUCCESS;

	// main ML filter
	if (useOpenImageDenoise)
	{
		rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_OPENIMAGE_DENOISE, &mRifImageFilterHandle);
	}
	else
	{
		rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_AI_DENOISE, &mRifImageFilterHandle);
	}
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create ML filter.");
	
	rifStatus = rifImageFilterSetParameterString(mRifImageFilterHandle, "modelPath", modelsPath.c_str());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to set ML filter models path.");

	mAuxImages.resize(AuxImageMax, nullptr);

	// temporary output for ML denoiser with 3 components per pixel (by design of the filter)
	rif_image_desc desc = { width, height, 0, 0, 0, 3, RIF_COMPONENT_TYPE_FLOAT32 };

	rifStatus = rifContextCreateImage(rifContext->Context(), &desc, nullptr, &mAuxImages[MlOutputRifImage]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create output image.");

	// Normals remap filter setup
	mAuxFilters.resize(AuxFilterMax, nullptr);

	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_REMAP_RANGE, &mAuxFilters[NormalsRemapFilter]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create remap filter.");

	rifStatus = rifImageFilterSetParameter1f(mAuxFilters[NormalsRemapFilter], "dstLo", 0.0f);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter1f(mAuxFilters[NormalsRemapFilter], "dstHi", +1.0f);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to set remap filter parameters.");

	// Depth remap filter setup
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_REMAP_RANGE, &mAuxFilters[DepthRemapFilter]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create remap filter.");

	rifStatus = rifImageFilterSetParameter1f(mAuxFilters[DepthRemapFilter], "dstLo", 0.0f);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter1f(mAuxFilters[DepthRemapFilter], "dstHi", 1.0f);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to set remap filter parameters.");

	// resampler filter setup
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_RESAMPLE, &mAuxFilters[OutputResampleFilter]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create resampler filter.");

	rifStatus = rifImageFilterSetParameter1u(mAuxFilters[OutputResampleFilter], "interpOperator", RIF_IMAGE_INTERPOLATION_NEAREST);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter2u(mAuxFilters[OutputResampleFilter], "outSize", width, height);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to set resampler parameters.");
}

RifFilterMl::~RifFilterMl()
{
}

void RifFilterMl::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "colorImg", mInputs.at(RifColor)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	rifStatus = rifImageFilterSetParameter1u(mRifImageFilterHandle, "useHDR", 1);
	assert(RIF_SUCCESS == rifStatus);	

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to setup ML filter.");

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "normalsImg", mInputs.at(RifNormal)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "depthImg", mInputs.at(RifDepth)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "albedoImg", mInputs.at(RifAlbedo)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to apply parameters.");

	// attach remap filters
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[NormalsRemapFilter],
		mInputs.at(RifNormal)->mRifImage, mInputs.at(RifNormal)->mRifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");

	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[DepthRemapFilter],
		mInputs.at(RifDepth)->mRifImage, mInputs.at(RifDepth)->mRifImage);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");

	// attach ML filter (main)
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, mAuxImages[MlOutputRifImage]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");

	// attach output resampler filter
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[OutputResampleFilter],
		mAuxImages[MlOutputRifImage], rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach resampler filter to queue.");
}



RifFilterMlColorOnly::RifFilterMlColorOnly(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath, bool useOpenImageDenoise)
{
	rif_int rifStatus = RIF_SUCCESS;

	// main ML filter
	if (useOpenImageDenoise)
	{
		rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_OPENIMAGE_DENOISE, &mRifImageFilterHandle);
	}
	else
	{
		rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_AI_DENOISE, &mRifImageFilterHandle);
	}

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create ML filter.");

	rifStatus = rifImageFilterSetParameterString(mRifImageFilterHandle, "modelPath", modelsPath.c_str());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to set ML filter models path.");

	mAuxImages.resize(AuxImageMax, nullptr);

	// temporary output for ML denoiser with 3 components per pixel (by design of the filter)
	rif_image_desc desc = { width, height, 0, 0, 0, 3, RIF_COMPONENT_TYPE_FLOAT32 };

	rifStatus = rifContextCreateImage(rifContext->Context(), &desc, nullptr, &mAuxImages[MlOutputRifImage]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create output image.");

	mAuxFilters.resize(AuxFilterMax, nullptr);

	// resampler filter setup
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_RESAMPLE, &mAuxFilters[OutputResampleFilter]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create resampler filter.");

	rifStatus = rifImageFilterSetParameter1u(mAuxFilters[OutputResampleFilter], "interpOperator", RIF_IMAGE_INTERPOLATION_NEAREST);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameter2u(mAuxFilters[OutputResampleFilter], "outSize", width, height);
		assert(RIF_SUCCESS == rifStatus);
	}

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to set resampler parameters.");
}

RifFilterMlColorOnly::~RifFilterMlColorOnly()
{
}

void RifFilterMlColorOnly::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	if (RIF_SUCCESS == rifStatus)
	{
		rifStatus = rifImageFilterSetParameterImage(mRifImageFilterHandle, "colorImg", mInputs.at(RifColor)->mRifImage);
		assert(RIF_SUCCESS == rifStatus);
	}

	rifStatus = rifImageFilterSetParameter1u(mRifImageFilterHandle, "useHDR", 1);
	assert(RIF_SUCCESS == rifStatus);
	
	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to setup ML filter.");

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to apply parameters.");

	// attach ML filter (main)
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, mAuxImages[MlOutputRifImage]);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");

	// attach output resampler filter
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mAuxFilters[OutputResampleFilter],
		mAuxImages[MlOutputRifImage], rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach resampler filter to queue.");
}

RifSCInternal::RifSCInternal(rif_context context, rif_image_filter_type type)
	: m_pFilter(nullptr)
	, m_connections()
{
	rif_int res = rifContextCreateImageFilter(context, type, &m_pFilter); 
	assert(RIF_SUCCESS == res);

	if (RIF_SUCCESS != res)
		throw std::runtime_error("RPR denoiser failed to create auxillary filter.");
}

RifSCInternal::~RifSCInternal()
{
	if (!m_pFilter)
		return;

	rif_int res = rifObjectDelete(m_pFilter);
	assert(RIF_SUCCESS == res);
}

RifSCInternal::operator rif_image_filter() const
{
	return m_pFilter;
}

void RifSCInternal::SetInput4f(const char *inputName, float r, float g, float b, float a)
{
	assert(nullptr != m_pFilter);
	assert(nullptr != inputName);

	if (!m_pFilter || !inputName)
		return;

	rif_int res = rifImageFilterSetParameter4f(m_pFilter, inputName, r, g, b, a);
	assert(RIF_SUCCESS == res);
}

void RifSCInternal::SetInputImage(const char *inputName, rif_image input)
{
	assert(nullptr != m_pFilter);
	assert(nullptr != inputName);

	if (!m_pFilter || !inputName)
		return;

	rif_int res = rifImageFilterSetParameterImage(m_pFilter, inputName, input);
	assert(RIF_SUCCESS == res);
}

void RifSCInternal::SaveDependency(const std::shared_ptr<RifSCInternal>& fromTemporary)
{
	m_connections.emplace_back();
	m_connections.back() = fromTemporary;
}

RifSCWrapper::RifSCWrapper()
{
	// this is dummy constructor for empty wrapper initialization
	// such instance is expected to be replaced by ibject create proper way
}

RifSCWrapper::RifSCWrapper(const rif_context pRifContext, const rif_image pInputImage)
	: m_inputType(IMAGE)
	, m_inputImage(pInputImage)
	, m_inputValues()
	, m_pContext(pRifContext)
	, m_filter()
{
	// NOTICE we should create filter ONLY after 2 inputs AND operation are set
	// however we can set only one input for the wrapper dirrectly;
	// filter is supposed to be created by arithmetic operations!
}

RifSCWrapper::RifSCWrapper(const rif_context pRifContext, float x, float y, float z, float w)
	: m_inputType(FLOAT)
	, m_inputImage(nullptr)
	, m_inputValues({ x, y, z, w })
	, m_pContext(pRifContext)
	, m_filter()
{
	// NOTICE we should create filter ONLY after 2 inputs AND operation are set
	// however we can set only one input for the wrapper dirrectly;
	// filter is supposed to be created by arithmetic operations!
}

RifSCWrapper::RifSCWrapper(const rif_context pRifContext, float x)
	: m_inputType(FLOAT)
	, m_inputImage(nullptr)
	, m_inputValues({ x, x, x, x })
	, m_pContext(pRifContext)
	, m_filter()
{
	// NOTICE we should create filter ONLY after 2 inputs AND operation are set
	// however we can set only one input for the wrapper dirrectly;
	// filter is supposed to be created by arithmetic operations!
}

RifSCWrapper::RifSCWrapper(const rif_context pRifContext, int operation)
	: m_pContext(pRifContext)
	, m_inputType(NOT_SET)
	, m_inputImage(nullptr)
	, m_inputValues()
	, m_filter(std::make_shared<RifSCInternal>(pRifContext, operation))
{
}

RifSCWrapper::RifSCWrapper(const RifSCWrapper& other)
	: m_inputType(other.m_inputType)
	, m_inputImage(other.m_inputImage)
	, m_inputValues(other.m_inputValues)
	, m_pContext(other.m_pContext)
	, m_filter(other.m_filter)
{}

RifSCWrapper& RifSCWrapper::operator=(const RifSCWrapper& other)
{

	// self-assignment
	if (this == &other)
		return *this;

	this->m_inputType = other.m_inputType;
	this->m_inputImage = other.m_inputImage;
	this->m_inputValues = other.m_inputValues;
	this->m_pContext = other.m_pContext;
	this->m_filter = other.m_filter;

	return *this;
}

void RifSCWrapper::SetInputs(RifSCInternal& filter, const RifSCWrapper& w1, const RifSCWrapper& w2)
{
	// determine names of inputs
	if (w1.m_inputType == RifSCWrapper::FLOAT)
	{
		char* lhs_input_name = "lhsVec";
		filter.SetInput4f(lhs_input_name, w1.m_inputValues[0], w1.m_inputValues[1], w1.m_inputValues[2], w1.m_inputValues[3]);
	}
	else // image or filter
	{
		char* lhs_input_name = "lhsImg";
		filter.SetInputImage(lhs_input_name, w1.m_inputImage);
	}

	if (w2.m_inputType == RifSCWrapper::FLOAT)
	{
		char* rhs_input_name = "rhsVec";
		filter.SetInput4f(rhs_input_name, w2.m_inputValues[0], w2.m_inputValues[1], w2.m_inputValues[2], w2.m_inputValues[3]);
	}
	else // image or filter
	{
		char* rhs_input_name = "rhsImg";
		filter.SetInputImage(rhs_input_name, w2.m_inputImage);
	}
}

RifSCWrapper operator+ (const RifSCWrapper& w1, const RifSCWrapper& w2)
{
	assert(w1.m_inputType != RifSCWrapper::NOT_SET);
	assert(w2.m_inputType != RifSCWrapper::NOT_SET);

	int operation = RIF_IMAGE_FILTER_ADD;
	RifSCWrapper filterWrapper(w1.m_pContext, operation);
	RifSCInternal& filter = *filterWrapper.m_filter;
	RifSCWrapper::SetInputs(filter, w1, w2);
	filterWrapper.m_inputType = RifSCWrapper::FILTER;
	filterWrapper.m_inputImage = (rif_image)(rif_image_filter)filter;

	filter.SaveDependency(w1.m_filter);
	filter.SaveDependency(w2.m_filter);

	return filterWrapper;
}

RifSCWrapper operator* (const RifSCWrapper& w1, const RifSCWrapper& w2)
{
	assert(w1.m_inputType != RifSCWrapper::NOT_SET);
	assert(w2.m_inputType != RifSCWrapper::NOT_SET);

	int operation = RIF_IMAGE_FILTER_MUL;
	RifSCWrapper filterWrapper(w1.m_pContext, operation);
	RifSCInternal& filter = *filterWrapper.m_filter;
	RifSCWrapper::SetInputs(filter, w1, w2);
	filterWrapper.m_inputType = RifSCWrapper::FILTER;
	filterWrapper.m_inputImage = (rif_image)(rif_image_filter)filter;

	filter.SaveDependency(w1.m_filter);
	filter.SaveDependency(w2.m_filter);

	return filterWrapper;
}

RifSCWrapper operator- (const RifSCWrapper& w1, const RifSCWrapper& w2)
{
	assert(w1.m_inputType != RifSCWrapper::NOT_SET);
	assert(w2.m_inputType != RifSCWrapper::NOT_SET);

	int operation = RIF_IMAGE_FILTER_SUB;
	RifSCWrapper filterWrapper(w1.m_pContext, operation);
	RifSCInternal& filter = *filterWrapper.m_filter;
	RifSCWrapper::SetInputs(filter, w1, w2);
	filterWrapper.m_inputType = RifSCWrapper::FILTER;
	filterWrapper.m_inputImage = (rif_image)(rif_image_filter)filter;

	filter.SaveDependency(w1.m_filter);
	filter.SaveDependency(w2.m_filter);

	return filterWrapper;
}

RifSCWrapper RifSCWrapper::min(const RifSCWrapper& w1, const RifSCWrapper& w2)
{
	assert(w1.m_inputType != RifSCWrapper::NOT_SET);
	assert(w2.m_inputType != RifSCWrapper::NOT_SET);

	int operation = RIF_IMAGE_FILTER_MIN;
	RifSCWrapper filterWrapper(w1.m_pContext, operation);
	RifSCInternal& filter = *filterWrapper.m_filter;
	RifSCWrapper::SetInputs(filter, w1, w2);
	filterWrapper.m_inputType = RifSCWrapper::FILTER;
	filterWrapper.m_inputImage = (rif_image)(rif_image_filter)filter;

	filter.SaveDependency(w1.m_filter);
	filter.SaveDependency(w2.m_filter);

	return filterWrapper;
}

const RifSCInternal& RifSCWrapper::GetInternalFilter(void)
{
	return *m_filter;
}

RifFilterShadowCatcher::RifFilterShadowCatcher(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath, bool useOpenImageDenoise)
{
}

RifFilterShadowCatcher::~RifFilterShadowCatcher()
{
	mRifImageFilterHandle = nullptr; // image filter is destroyed automatically within m_res
}

void RifFilterShadowCatcher::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// setup shadow catcher inputs
	RifSCWrapper color(rifContext->Context(), mInputs.at(RifColor)->mRifImage);
	RifSCWrapper mattePass(rifContext->Context(), mInputs.at(RifMattePass)->mRifImage);
	RifSCWrapper shadowCatcher(rifContext->Context(), mInputs.at(RifShadowCatcher)->mRifImage);
	RifSCWrapper shadowColor(rifContext->Context(), mParams["shadowColor[0]"], mParams["shadowColor[1]"], mParams["shadowColor[2]"], 1.0f);
	RifSCWrapper shadowTransp(rifContext->Context(), mParams["shadowWeight"] - mParams["shadowTransp"]);
	RifSCWrapper const1(rifContext->Context(), 1.0f);

	// mattePass * (1 - sc) + (color - mattePass)
	RifSCWrapper step1 = shadowCatcher * shadowTransp * (const1 - shadowColor);
	RifSCWrapper step2 = mattePass * (const1 - step1);
	m_res = step2 + (color - mattePass);

	mRifImageFilterHandle = m_res.GetInternalFilter();

	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");
}

RifFilterReflectionCatcher::RifFilterReflectionCatcher(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath, bool useOpenImageDenoise)
{
}

RifFilterReflectionCatcher::~RifFilterReflectionCatcher()
{
	mRifImageFilterHandle = nullptr; // image filter is destroyed automatically within m_res
}

void RifFilterReflectionCatcher::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// setup shadow catcher inputs
	RifSCWrapper noAlpha(rifContext->Context(), 1.0f, 1.0f, 1.0f, 0.0f);
	RifSCWrapper color(rifContext->Context(), mInputs.at(RifColor)->mRifImage);
	RifSCWrapper opacity(rifContext->Context(), mInputs.at(RifOpacity)->mRifImage);
	RifSCWrapper reflectionCatcher(rifContext->Context(), mInputs.at(RifReflectionCatcher)->mRifImage);
	RifSCWrapper const1(rifContext->Context(), 1.0f);
	RifSCWrapper background(rifContext->Context(), mInputs.at(RifBackground)->mRifImage);

	// background * (1 - (opacity + rc)) + color
	RifSCWrapper step1 = const1 - (opacity + reflectionCatcher) * noAlpha;
	RifSCWrapper step2 = background * step1;
	m_res = step2 + color;

	mRifImageFilterHandle = m_res.GetInternalFilter();

	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");
}

RifFilterShadowReflectionCatcher::RifFilterShadowReflectionCatcher(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath, bool useOpenImageDenoise)
{
}

RifFilterShadowReflectionCatcher::~RifFilterShadowReflectionCatcher()
{
	mRifImageFilterHandle = nullptr; // image filter is destroyed automatically within m_res
}

void RifFilterShadowReflectionCatcher::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// setup shadow catcher inputs
	RifSCWrapper noAlpha(rifContext->Context(), 1.0f, 1.0f, 1.0f, 0.0f);
	RifSCWrapper color(rifContext->Context(), mInputs.at(RifColor)->mRifImage);
	RifSCWrapper opacity(rifContext->Context(), mInputs.at(RifOpacity)->mRifImage);
	RifSCWrapper shadowCatcher(rifContext->Context(), mInputs.at(RifShadowCatcher)->mRifImage);
	RifSCWrapper shadowColor(rifContext->Context(), mParams["shadowColor[0]"], mParams["shadowColor[1]"], mParams["shadowColor[2]"], 1.0f);
	RifSCWrapper reflectionCatcher(rifContext->Context(), mInputs.at(RifReflectionCatcher)->mRifImage);
	RifSCWrapper const1(rifContext->Context(), 1.0f);
	RifSCWrapper shadowTransp(rifContext->Context(), mParams["shadowWeight"] - mParams["shadowTransp"]);
	RifSCWrapper background(rifContext->Context(), mInputs.at(RifBackground)->mRifImage);
	RifSCWrapper mattePass(rifContext->Context(), mInputs.at(RifBackground)->mRifImage);

	// ((background * (1 - (opacity + rc)) + mattePass) * (1 - sc)) + (color - mattePass)
	RifSCWrapper step1 = const1 - (opacity + reflectionCatcher) * noAlpha;
	RifSCWrapper step2 = background * step1;
	RifSCWrapper step3 = shadowCatcher * shadowTransp * (const1 - shadowColor);
	RifSCWrapper step4 = (step2 + mattePass) * (const1 - step3);
	m_res = step4 + (color - mattePass);

	mRifImageFilterHandle = m_res.GetInternalFilter();

	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");
}

RifFilterUpscaler::RifFilterUpscaler(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath)
{
	rif_int rifStatus = RIF_SUCCESS;

	// Upscaler filter
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_AI_UPSCALE, &mRifImageFilterHandle);

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to create Upscale filter.");

	rifStatus = rifImageFilterSetParameterString(mRifImageFilterHandle, "modelPath", modelsPath.c_str());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to set ML filter models path.");

	rifStatus = rifImageFilterSetParameter1u(mRifImageFilterHandle, "mode", RIF_AI_UPSCALE_MODE_FAST_2X);
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to set RIF_AI_UPSCALE_MODE_BEST_2X parameter.");
}

RifFilterUpscaler::~RifFilterUpscaler()
{

}

void RifFilterUpscaler::AttachFilter(const RifContextWrapper* rifContext)
{
	// attach Upscale filter
	rif_int rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR denoiser failed to attach filter to queue.");
}

RifFilterLinearTonemap::RifFilterLinearTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath)
{
	rif_int rifStatus = RIF_SUCCESS;

	// linear tonemap filter
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_LINEAR_TONEMAP, &mRifImageFilterHandle);

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to create Linear Tonemap filter.");
}

RifFilterLinearTonemap::~RifFilterLinearTonemap()
{

}

void RifFilterLinearTonemap::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// attach linear tonemap filter
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to attach Linear Tonemap filter to queue.");
}

RifFilterPhotoLinearTonemap::RifFilterPhotoLinearTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath)
{
	rif_int rifStatus = RIF_SUCCESS;

	// photolinear tonemap filter
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_PHOTO_LINEAR_TONEMAP, &mRifImageFilterHandle);

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to create Photo Linear Tonemap filter.");
}

RifFilterPhotoLinearTonemap::~RifFilterPhotoLinearTonemap()
{

}

void RifFilterPhotoLinearTonemap::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// attach photolinear tonemap filter
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to attach Photo Linear Tonemap filter to queue.");
}

RiffilterAutoLinearTonemap::RiffilterAutoLinearTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath)
{
	rif_int rifStatus = RIF_SUCCESS;

	// autolinear tonemap filter
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_AUTOLINEAR_TONEMAP, &mRifImageFilterHandle);

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to create Auto Linear Tonemap filter.");
}

RiffilterAutoLinearTonemap::~RiffilterAutoLinearTonemap()
{

}

void RiffilterAutoLinearTonemap::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// attach auto linear tonemap filter
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to attach Auto Linear Tonemap filter to queue.");
}

RifFilterMaxWhiteTonemap::RifFilterMaxWhiteTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath)
{
	rif_int rifStatus = RIF_SUCCESS;

	// max white tonemap filter
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_MAXWHITE_TONEMAP, &mRifImageFilterHandle);

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to create Max White Tonemap filter.");
}

RifFilterMaxWhiteTonemap::~RifFilterMaxWhiteTonemap()
{

}

void RifFilterMaxWhiteTonemap::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// attach max white tonemap filter
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to attach Max White Tonemap filter to queue.");
}

RifFilterReinhardTonemap::RifFilterReinhardTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
	const std::string& modelsPath)
{
	rif_int rifStatus = RIF_SUCCESS;

	// Reinhard tonemap filter
	rifStatus = rifContextCreateImageFilter(rifContext->Context(), RIF_IMAGE_FILTER_REINHARD02_TONEMAP, &mRifImageFilterHandle);

	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to create Reinhard Tonemap filter.");
}

RifFilterReinhardTonemap::~RifFilterReinhardTonemap()
{

}

void RifFilterReinhardTonemap::AttachFilter(const RifContextWrapper* rifContext)
{
	rif_int rifStatus = RIF_SUCCESS;

	// attach Reinhard tonemap filter
	rifStatus = rifCommandQueueAttachImageFilter(rifContext->Queue(), mRifImageFilterHandle,
		mInputs.at(RifColor)->mRifImage, rifContext->Output());
	assert(RIF_SUCCESS == rifStatus);

	if (RIF_SUCCESS != rifStatus)
		throw std::runtime_error("RPR failed to attach Reinhard Tonemap filter to queue.");
}

