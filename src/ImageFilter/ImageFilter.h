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
#pragma once

#include "RadeonProRender.h"
#include "RadeonImageFilters.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <array>


enum class RifFilterType
{
	BilateralDenoise,
	LwrDenoise,
	EawDenoise,
	MlDenoise,
	MlDenoiseColorOnly,

	ShadowCatcher,
	ReflectionCatcher,
	ShadowReflectionCatcher,
	Upscaler,

	LinearTonemap,
	PhotoLinearTonemap,
	AutoLinearTonemap,
	MaxWhiteTonemap,
	ReinhardTonemap,
};

enum RifFilterInput
{
	RifColor,
	RifNormal,
	RifDepth,
	RifWorldCoordinate,
	RifObjectId,
	RifTrans,
	RifAlbedo,
	RifOpacity,
	RifBackground,
	RifShadowCatcher,
	RifReflectionCatcher,
	RifMattePass,
	RifMaxInput
};

enum class RifParamType
{
	RifInt,
	RifFloat,
	RifOther  //param not applied to filter
};

union RifData
{
	rif_int   i;
	rif_float f;
};

struct RifParam
{
	RifParamType mType;
	RifData      mData;

	operator rif_float() const { return mData.f; }

	RifParam() {}
	RifParam(RifParamType type, rif_float data) : mType(type), mData() { mData.f = data; }
	RifParam(RifParamType type, rif_int data) : mType(type), mData() { mData.i = data; }
};

class RifContextWrapper;
class RifFilterWrapper;

class ImageFilter final
{
	std::unique_ptr<RifContextWrapper> mRifContext;

	std::unique_ptr<RifFilterWrapper> mRifFilter;

	std::uint32_t mWidth;
	std::uint32_t mHeight;
	std::string mModelsPath;

	std::uint32_t mInputOverrideWidth;
	std::uint32_t mInputOverrideHeight;

public:
	explicit ImageFilter(const rpr_context rprContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath = std::string(), bool forceCPUContext = false);
	~ImageFilter();

	void CreateFilter(RifFilterType rifFilteType, bool useOpenImageDenoise = false);
	void DeleteFilter();

	void AddInput(RifFilterInput inputId, const rpr_framebuffer rprFrameBuffer, float sigma) const;
	void AddInput(RifFilterInput inputId, float* memPtr, size_t size, float sigma) const;
	void AddParam(std::string name, RifParam param) const;

	void AttachFilter() const;

	void Run() const;

	std::vector<float> GetData() const;

	void SetInputOverrideSize(std::uint32_t width, std::uint32_t height);
};



class RifContextWrapper
{
protected:
	rif_context mRifContextHandle = nullptr;
	rif_command_queue mRifCommandQueueHandle = nullptr;
	rif_image mOutputRifImage = nullptr;

public:
	virtual ~RifContextWrapper();

	const rif_context Context() const;
	const rif_command_queue Queue() const;
	const rif_image Output() const;

	void CreateOutput(const rif_image_desc& desc);

	virtual rif_image CreateRifImage(const rpr_framebuffer rprFrameBuffer, const rif_image_desc& desc) const = 0;
	void UpdateInputs(const RifFilterWrapper* rifFilter) const;

};

class RifContextGPU final : public RifContextWrapper
{
	const rif_backend_api_type rifBackendApiType = RIF_BACKEND_API_OPENCL;

public:
	explicit RifContextGPU(const rpr_context rprContext);
	virtual ~RifContextGPU();

	virtual rif_image CreateRifImage(const rpr_framebuffer rprFrameBuffer, const rif_image_desc& desc) const override;
};

class RifContextGPUMetal final : public RifContextWrapper
{
	const rif_backend_api_type rifBackendApiType = RIF_BACKEND_API_METAL;
    
public:
	explicit RifContextGPUMetal(const rpr_context rprContext);
	virtual ~RifContextGPUMetal();
    
	virtual rif_image CreateRifImage(const rpr_framebuffer rprFrameBuffer, const rif_image_desc& desc) const override;
};

class RifContextCPU final : public RifContextWrapper
{
#ifdef __APPLE__
	const rif_backend_api_type rifBackendApiType = RIF_BACKEND_API_METAL;
#else
    const rif_backend_api_type rifBackendApiType = RIF_BACKEND_API_OPENCL;
#endif

public:
	explicit RifContextCPU(const rpr_context rprContext);
	virtual ~RifContextCPU();

	virtual rif_image CreateRifImage(const rpr_framebuffer rprFrameBuffer, const rif_image_desc& desc) const override;
};



struct RifInput
{
	rif_image mRifImage = nullptr;
	float     mSigma = 0.0f;

	RifInput(rif_image rifImage, float sigma);
	virtual ~RifInput();
	virtual void Update() = 0;
};

using RifInputPtr = std::shared_ptr<RifInput>;

struct RifInputGPU : public RifInput
{
	RifInputGPU(rif_image rifImage, float sigma);
	virtual ~RifInputGPU();
	void Update() override;
};

struct RifInputGPUCPU : public RifInput
{
	rpr_framebuffer mRprFrameBuffer = nullptr;

	RifInputGPUCPU(rif_image rifImage, const rpr_framebuffer rprFrameBuffer, float sigma);
	virtual ~RifInputGPUCPU();
	void Update() override;
};

struct RifInputCPU : public RifInput
{
	float* mMemPtr = nullptr;
	size_t mSize = 0;

	RifInputCPU(rif_image rifImage, float* memPtr, size_t size, float sigma);
	virtual ~RifInputCPU();
	void Update() override;
};

class RifFilterWrapper
{
	friend class RifContextWrapper;
	friend class RifContextCPU;

protected:
	rif_image_filter mRifImageFilterHandle = nullptr;

	std::vector<rif_image_filter> mAuxFilters;
	std::vector<rif_image> mAuxImages;

	std::unordered_map<RifFilterInput, RifInputPtr> mInputs;
	std::unordered_map<std::string, RifParam> mParams;

public:
	virtual ~RifFilterWrapper();

	void AddInput(RifFilterInput inputId, const rif_image rifImage, float sigma);
	void AddInput(RifFilterInput inputId, const rif_image rifImage, const rpr_framebuffer rprFrameBuffer, float sigma);
	void AddInput(RifFilterInput inputId, const rif_image rifImage, float* memPtr, size_t size, float sigma);
	void AddParam(std::string name, RifParam param);

	virtual void AttachFilter(const RifContextWrapper* rifContext) = 0;
	virtual void DetachFilter(const RifContextWrapper* rifContext) noexcept final;

	void ApplyParameters() const;

protected:
	void SetupVarianceImageFilter(const rif_image_filter inputFilter, const rif_image outVarianceImage) const;
};

class RifFilterBilateral final : public RifFilterWrapper
{
	// vector representation of inputs is needed to feed library
	std::vector<rif_image> inputImages;
	std::vector<float> sigmas;

public:
	explicit RifFilterBilateral(const RifContextWrapper* rifContext);
	virtual ~RifFilterBilateral();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};

class RifFilterLwr final : public RifFilterWrapper
{
	enum
	{
		ColorVar,
		NormalVar,
		DepthVar,
		TransVar,
		AuxFilterMax
	};

	enum
	{
		ColorVarianceImage,
		NormalVarianceImage,
		DepthVarianceImage,
		TransVarianceImage,
		AuxImageMax
	};

public:
	explicit RifFilterLwr(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height);
	virtual ~RifFilterLwr();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};

class RifFilterEaw final : public RifFilterWrapper
{
	enum
	{
		ColorVar,
		Mlaa,
		AuxFilterMax
	};

	enum
	{
		ColorVarianceImage,
		DenoisedOutputImage,
		AuxImageMax
	};

public:
	explicit RifFilterEaw(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height);
	virtual ~RifFilterEaw();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};

class RifFilterMl final : public RifFilterWrapper
{
	enum
	{
		NormalsRemapFilter,
		DepthRemapFilter,
		OutputResampleFilter,
		AuxFilterMax
	};

	enum
	{
		MlOutputRifImage,
		AuxImageMax
	};

public:
	explicit RifFilterMl(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath, bool useOpenImageDenoise);
	virtual ~RifFilterMl();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};

class RifFilterMlColorOnly final : public RifFilterWrapper
{
	enum
	{
		OutputResampleFilter,
		AuxFilterMax
	};

	enum
	{
		MlOutputRifImage,
		AuxImageMax
	};

public:
	explicit RifFilterMlColorOnly(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath, bool useOpenImageDenoise);
	virtual ~RifFilterMlColorOnly();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};

class RifSCInternal
{
protected:
	rif_image_filter m_pFilter;
	std::vector<std::shared_ptr<RifSCInternal>> m_connections;

public:
	RifSCInternal(rif_context context, rif_image_filter_type type);
	~RifSCInternal(void);
	operator rif_image_filter(void) const;

	void SetInput4f(const char *inputName, float r, float g, float b, float a);
	void SetInputImage(const char *inputName, rif_image input);

	void SaveDependency(const std::shared_ptr<RifSCInternal>& fromTemporary);
};

class RifSCWrapper
{
protected:
	std::shared_ptr<RifSCInternal> m_filter;
	rif_context m_pContext;

	enum
	{
		RIF_ADD,
		RIF_SUBTRACT,
		RIF_MULTIPLY,
		RIF_MIN,
		INPUT,
		UNDEFINED,
	};

	enum
	{
		IMAGE,
		FLOAT,
		FILTER,
		NOT_SET,
	} m_inputType;

	std::array<float, 4> m_inputValues;
	rif_image m_inputImage;

public:
	RifSCWrapper();
	RifSCWrapper(const rif_context pRifContext, const rif_image pInputImage);
	RifSCWrapper(const rif_context pRifContext, float x);
	RifSCWrapper(const rif_context pRifContext, float x, float y, float z, float w);
	RifSCWrapper(const RifSCWrapper& other);
	RifSCWrapper& operator=(const RifSCWrapper& other);

	friend RifSCWrapper operator+ (const RifSCWrapper& w1, const RifSCWrapper& w2);
	friend RifSCWrapper operator* (const RifSCWrapper& w1, const RifSCWrapper& w2);
	friend RifSCWrapper operator- (const RifSCWrapper& w1, const RifSCWrapper& w2);
	static RifSCWrapper min(const RifSCWrapper& w1, const RifSCWrapper& w2);

	const RifSCInternal& GetInternalFilter(void);

protected:
	RifSCWrapper(const rif_context pRifContext, int operation);

	static void SetInputs(RifSCInternal& filter, const RifSCWrapper& w1, const RifSCWrapper& w2);
};

class RifFilterShadowCatcher final : public RifFilterWrapper
{
	RifSCWrapper m_res;

public:
	explicit RifFilterShadowCatcher(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath, bool useOpenImageDenoise);
	virtual ~RifFilterShadowCatcher();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};

class RifFilterReflectionCatcher final : public RifFilterWrapper
{
	RifSCWrapper m_res;

public:
	explicit RifFilterReflectionCatcher(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath, bool useOpenImageDenoise);
	virtual ~RifFilterReflectionCatcher();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};


class RifFilterShadowReflectionCatcher final : public RifFilterWrapper
{
	RifSCWrapper m_res;

public:
	explicit RifFilterShadowReflectionCatcher(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath, bool useOpenImageDenoise);
	virtual ~RifFilterShadowReflectionCatcher();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};


class RifFilterUpscaler final : public RifFilterWrapper
{
public:
	explicit RifFilterUpscaler(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath);
	virtual ~RifFilterUpscaler();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};


class RifFilterLinearTonemap final : public RifFilterWrapper
{
public:
	explicit RifFilterLinearTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath);
	virtual ~RifFilterLinearTonemap();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};


class RifFilterPhotoLinearTonemap final : public RifFilterWrapper
{
public:
	explicit RifFilterPhotoLinearTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath);
	virtual ~RifFilterPhotoLinearTonemap();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};


class RiffilterAutoLinearTonemap final : public RifFilterWrapper
{
public:
	explicit RiffilterAutoLinearTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath);
	virtual ~RiffilterAutoLinearTonemap();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};

class RifFilterMaxWhiteTonemap final : public RifFilterWrapper
{
public:
	explicit RifFilterMaxWhiteTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath);
	virtual ~RifFilterMaxWhiteTonemap();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};

class RifFilterReinhardTonemap final : public RifFilterWrapper
{
public:
	explicit RifFilterReinhardTonemap(const RifContextWrapper* rifContext, std::uint32_t width, std::uint32_t height,
		const std::string& modelsPath);
	virtual ~RifFilterReinhardTonemap();

	virtual void AttachFilter(const RifContextWrapper* rifContext) override;
};


