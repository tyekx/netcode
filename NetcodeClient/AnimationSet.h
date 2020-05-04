#pragma once
#include <memory>
#include <functional>
#include <vector>
#include "AnimationController.h"
#include <NetcodeAssetLib/Animation.h>
#include <NetcodeAssetLib/Bone.h>

#include <Netcode/HandleTypes.h>
#include <Netcode/Modules.h>

#include "ConstantBufferTypes.h"
#include "Asset.h"

class AnimationSet : public std::enable_shared_from_this<AnimationSet> {
public:
	constexpr static uint32_t MAX_STATE_COUNT = 8;
	constexpr static uint32_t MAX_INSTANCE_COUNT = 32;
	constexpr static uint32_t MAX_ANIMATION_COUNT = 64;
private:
	Netcode::GpuResourceRef resultBuffer;
	Netcode::GpuResourceRef resultReadbackBuffer;
	Netcode::GpuResourceRef instanceCbuffer;
	Netcode::GpuResourceRef animationStaticCbuffer;
	Netcode::GpuResourceRef animationKeysBuffer;
	Netcode::GpuResourceRef intermediateBuffer;

	Netcode::ResourceViewsRef ctrlView;
	Netcode::ResourceViewsRef animKeysView;
	
	uint32_t numInstances;
	uint32_t numActiveControllers;
	std::vector<std::unique_ptr<AnimationController>> freedControllers;

	AnimInstanceConstants instanceData;

	void FreeController(AnimationController * rawPtr);

	std::shared_ptr<AnimationController> MakeNewController();

	std::shared_ptr<AnimationController> ReuseController();

	AnimationSet() = default;

public:

	void Clear(Netcode::Graphics::IRenderContext * context);

	uint32_t GetNumInstances() const;

	Netcode::ResourceViewsRef GetResultsView() const {
		return ctrlView;
	}

	void CopyResults(Netcode::Graphics::IRenderContext * context) {
		context->ResourceBarrier(resultBuffer, Netcode::Graphics::ResourceState::UNORDERED_ACCESS, Netcode::Graphics::ResourceState::NON_PIXEL_SHADER_RESOURCE | Netcode::Graphics::ResourceState::COPY_SOURCE);
		context->FlushResourceBarriers();
		size_t resultSize = BoneData::MAX_BONE_COUNT * numActiveControllers * 2 * sizeof(DirectX::XMFLOAT4X4);
		if(resultSize > 0) {
			context->CopyBufferRegion(resultReadbackBuffer, resultBuffer, resultSize);
		}
	}

	Netcode::GpuResourceRef GetResultReadbackBuffer() const {
		return resultReadbackBuffer;
	}

	void UploadConstants(Netcode::Graphics::IResourceContext * context);

	void BindResources(Netcode::Graphics::IRenderContext * context);

	int32_t Activate(const std::vector<Netcode::Animation::BlendItem> & blendPlan);

	AnimationSet(Netcode::Module::IGraphicsModule * graphics, Netcode::ArrayView<Netcode::Asset::Animation> animations, Netcode::ArrayView<Netcode::Asset::Bone> bones);

	std::shared_ptr<AnimationController> CreateController();
};
