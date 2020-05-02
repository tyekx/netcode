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
	Netcode::GpuResourceRef animationStaticCbuffer;
	Netcode::GpuResourceRef animationKeysBuffer;
	Netcode::GpuResourceRef intermediateBuffer;

	Netcode::ResourceViewsRef animKeysView;
	
	uint32_t numActiveControllers;
	std::vector<std::unique_ptr<AnimationController>> freedControllers;

	void FreeController(AnimationController * rawPtr) {
		std::unique_ptr<AnimationController> wrappedPtr{ rawPtr };
		wrappedPtr->animationSet.reset();
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
		AnimationController * rawPtr = ctrl.release();
		rawPtr->animationSet = shared_from_this();
		return std::shared_ptr<AnimationController>(rawPtr,
			std::bind(&AnimationSet::FreeController, this, std::placeholders::_1));
	}

public:
	AnimationSet(Netcode::Module::IGraphicsModule* graphics, Netcode::ArrayView<Netcode::Asset::Animation> animations, Netcode::ArrayView<Netcode::Asset::Bone> bones) {
		animationStaticCbuffer = graphics->resources->CreateConstantBuffer(sizeof(AnimationStaticConstants));
		
		AnimationStaticConstants cbufferData;
		cbufferData.numAnimations = static_cast<uint32_t>(animations.Size());
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

		graphics->resources->CopyConstants(animationStaticCbuffer, &cbufferData, sizeof(AnimationStaticConstants));

		resultBuffer = graphics->resources->CreateStructuredBuffer(BoneData::MAX_BONE_COUNT * MAX_INSTANCE_COUNT * sizeof(DirectX::XMFLOAT4X4),
			sizeof(DirectX::XMFLOAT4X4),
			Netcode::Graphics::ResourceType::PERMANENT_DEFAULT,
			Netcode::Graphics::ResourceState::UNORDERED_ACCESS,
			Netcode::Graphics::ResourceFlags::ALLOW_UNORDERED_ACCESS);

		resultReadbackBuffer = graphics->resources->CreateReadbackBuffer(BoneData::MAX_BONE_COUNT * MAX_INSTANCE_COUNT * sizeof(DirectX::XMFLOAT4X4),
			Netcode::Graphics::ResourceType::PERMANENT_READBACK,
			Netcode::Graphics::ResourceFlags::DENY_SHADER_RESOURCE);

		intermediateBuffer = graphics->resources->CreateStructuredBuffer(BoneData::MAX_BONE_COUNT * MAX_STATE_COUNT * MAX_INSTANCE_COUNT * sizeof(Netcode::Asset::AnimationKey),
			sizeof(Netcode::Asset::AnimationKey),
			Netcode::Graphics::ResourceType::PERMANENT_DEFAULT,
			Netcode::Graphics::ResourceState::UNORDERED_ACCESS,
			Netcode::Graphics::ResourceFlags::ALLOW_UNORDERED_ACCESS);

		animationKeysBuffer = graphics->resources->CreateStructuredBuffer(sumKeys * sizeof(Netcode::Asset::AnimationKey),
			sizeof(Netcode::Asset::AnimationKey),
			Netcode::Graphics::ResourceType::PERMANENT_DEFAULT,
			Netcode::Graphics::ResourceState::COPY_DEST,
			Netcode::Graphics::ResourceFlags::NONE);

		animKeysView = graphics->resources->CreateShaderResourceViews(3);
		animKeysView->CreateSRV(0, animationKeysBuffer);
		animKeysView->CreateUAV(1, intermediateBuffer);
		animKeysView->CreateUAV(2, resultBuffer);

		Netcode::Graphics::UploadBatch uploadBatch;
		
		size_t offset = 0;
		for(Netcode::Asset::Animation & anim : animations) {
			size_t itemSize = anim.keysLength * sizeof(Netcode::Asset::AnimationKey);
			uploadBatch.Upload(animationKeysBuffer, anim.keys, itemSize, offset);
			offset += itemSize;
		}

		uploadBatch.ResourceBarrier(animationKeysBuffer,
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
