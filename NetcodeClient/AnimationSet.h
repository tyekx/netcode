#pragma once
#include <memory>
#include <functional>
#include <vector>
#include "AnimationController.h"
#include <NetcodeAssetLib/Animation.h>
#include <Netcode/HandleTypes.h>
#include <Netcode/Modules.h>

#include "ConstantBufferTypes.h"
#include "Asset.h"

class AnimationSet : public std::enable_shared_from_this<AnimationSet> {
public:
	constexpr static uint32_t MAX_INSTANCE_COUNT = 32;
	constexpr static uint32_t MAX_ANIMATION_COUNT = 64;
private:
	Netcode::GpuResourceRef animationStaticCbuffer;
	Netcode::GpuResourceRef structuredBuffer;
	uint32_t numActiveControllers;
	std::vector<std::unique_ptr<AnimationController>> freedControllers;

	void FreeController(AnimationController * rawPtr) {
		std::unique_ptr<AnimationController> wrappedPtr{ rawPtr };
		freedControllers.emplace_back(std::move(wrappedPtr));
	}

	std::shared_ptr<AnimationController> MakeNewController() {
		if(numActiveControllers >= MAX_INSTANCE_COUNT) {
			return nullptr;
		}
		++numActiveControllers;

		return std::shared_ptr<AnimationController>(
			new AnimationController(shared_from_this()),
			std::bind(&AnimationSet::FreeController, this, std::placeholders::_1));
	}

	std::shared_ptr<AnimationController> ReuseController() {
		std::unique_ptr<AnimationController> ctrl = std::move(freedControllers.back());
		freedControllers.pop_back();
		return std::shared_ptr<AnimationController>(std::move(ctrl));
	}

public:
	AnimationSet(Netcode::Module::IGraphicsModule* graphics, Netcode::ArrayView<Netcode::Asset::Animation> animations, Netcode::ArrayView<Netcode::Asset::Bone> bones) {
		animationStaticCbuffer = graphics->resources->CreateConstantBuffer(sizeof(AnimationStaticConstants));
		
		AnimationStaticConstants cbufferData;
		cbufferData.numAnimations =static_cast<uint32_t>(animations.Size());
		cbufferData.numBones = static_cast<uint32_t>(bones.Size());

		for(size_t i = 0; i < bones.Size(); ++i) {
			cbufferData.parentIndices[i] = bones[i].parentId;
			cbufferData.offsetMatrices[i] = bones[i].transform;
		}

		uint32_t sumKeys = 0;
		for(size_t i = 0; i < animations.Size(); ++i) {
			cbufferData.startIndices[i] = sumKeys;
			sumKeys += animations[i].keysLength;
		}
		
		structuredBuffer = graphics->resources->CreateStructuredBuffer(sumKeys * sizeof(GpuAnimationKey),
			sizeof(GpuAnimationKey),
			Netcode::Graphics::ResourceType::PERMANENT_DEFAULT,
			Netcode::Graphics::ResourceState::COPY_DEST,
			Netcode::Graphics::ResourceFlags::NONE);

		Netcode::Graphics::UploadBatch uploadBatch;
		uploadBatch.Upload(structuredBuffer, nullptr, 0);
		uploadBatch.ResourceBarrier(structuredBuffer,
			Netcode::Graphics::ResourceState::COPY_DEST,
			Netcode::Graphics::ResourceState::NON_PIXEL_SHADER_RESOURCE);
		graphics->frame->SyncUpload(uploadBatch);

		
	}

	std::shared_ptr<AnimationController> CreateController() {
		if(freedControllers.empty()) {
			return MakeNewController();
		} else {
			return ReuseController();
		}
	}
};
