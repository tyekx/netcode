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
	Ref<Netcode::GpuResource> resultBuffer;
	Ref<Netcode::GpuResource> resultReadbackBuffer;
	Ref<Netcode::GpuResource> instanceCbuffer;
	Ref<Netcode::GpuResource> animationStaticCbuffer;
	Ref<Netcode::GpuResource> animationKeysBuffer;
	Ref<Netcode::GpuResource> intermediateBuffer;

	Ref<Netcode::ResourceViews> ctrlView;
	Ref<Netcode::ResourceViews> animKeysView;
	
	uint32_t numInstances;
	uint32_t numActiveControllers;
	std::vector<std::unique_ptr<AnimationController>> freedControllers;

	AnimInstanceConstants instanceData;

	BoneData results[MAX_INSTANCE_COUNT];

	void FreeController(Ptr<AnimationController> rawPtr);

	Ref<AnimationController> MakeNewController();

	Ref<AnimationController> ReuseController();

	AnimationSet() = default;

public:

	Ref<Netcode::GpuResource> GetIntermediateResource() const {
		return intermediateBuffer;
	}

	void Clear();

	uint32_t GetNumInstances() const;

	Ref<Netcode::ResourceViews> GetResultsView() const {
		return ctrlView;
	}

	void CopyResults(Netcode::Graphics::IRenderContext * context);

	Ref<Netcode::GpuResource> GetResultReadbackBuffer() const {
		return resultReadbackBuffer;
	}

	BoneData * GetData() {
		return results;
	}

	void UploadConstants(Netcode::Graphics::IResourceContext * context);

	void BindResources(Netcode::Graphics::IRenderContext * context);

	int32_t Activate(const std::vector<Netcode::Animation::BlendItem> & blendPlan);

	AnimationSet(Netcode::Module::IGraphicsModule * graphics, Netcode::ArrayView<Netcode::Asset::Animation> animations, Netcode::ArrayView<Netcode::Asset::Bone> bones);

	Ref<AnimationController> CreateController();
};
