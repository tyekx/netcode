#include "AnimationSet.h"

void AnimationSet::FreeController(AnimationController * rawPtr) {
	std::unique_ptr<AnimationController> wrappedPtr{ rawPtr };
	wrappedPtr->animationSet.reset();
	freedControllers.emplace_back(std::move(wrappedPtr));
}

std::shared_ptr<AnimationController> AnimationSet::MakeNewController() {
	if(numActiveControllers >= MAX_INSTANCE_COUNT) {
		return nullptr;
	}

	++numActiveControllers;

	return std::shared_ptr<AnimationController>(
		new AnimationController(shared_from_this()),
		std::bind(&AnimationSet::FreeController, this, std::placeholders::_1));
}

std::shared_ptr<AnimationController> AnimationSet::ReuseController() {
	std::unique_ptr<AnimationController> ctrl = std::move(freedControllers.back());
	freedControllers.pop_back();
	AnimationController * rawPtr = ctrl.release();
	rawPtr->animationSet = shared_from_this();
	return std::shared_ptr<AnimationController>(rawPtr,
		std::bind(&AnimationSet::FreeController, this, std::placeholders::_1));
}

void AnimationSet::Clear() {
	numInstances = 0;
}

uint32_t AnimationSet::GetNumInstances() const {
	return numInstances;
}

void AnimationSet::UploadConstants(Netcode::Graphics::IResourceContext * context) {
	instanceData.numInstances = GetNumInstances();
	context->CopyConstants(instanceCbuffer, &instanceData, sizeof(AnimInstanceConstants));
}

void AnimationSet::BindResources(Netcode::Graphics::IRenderContext * context) {
	context->ResourceBarrier(resultBuffer, Netcode::Graphics::ResourceState::NON_PIXEL_SHADER_RESOURCE | Netcode::Graphics::ResourceState::COPY_SOURCE, Netcode::Graphics::ResourceState::UNORDERED_ACCESS);
	context->FlushResourceBarriers();
	context->SetConstantBuffer(0, instanceCbuffer);
	context->SetConstantBuffer(1, animationStaticCbuffer);
	context->SetShaderResources(2, animKeysView);
}

int32_t AnimationSet::Activate(const std::vector<Netcode::Animation::BlendItem> & blendPlan) {
	if(numInstances < MAX_INSTANCE_COUNT) {
		uint32_t v = numInstances++;

		instanceData.instances[v].numStates = static_cast<uint32_t>(blendPlan.size());

		for(size_t i = 0; i < blendPlan.size(); ++i) {
			instanceData.instances[v].animationIndices[i] = blendPlan[i].clipId;
			instanceData.instances[v].weights[i] = blendPlan[i].weight;
			instanceData.instances[v].normalizedTimes[i] = blendPlan[i].normalizedTime;
			instanceData.instances[v].frameIndices[2 * i] = blendPlan[i].beginFrameIndex;
			instanceData.instances[v].frameIndices[2 * i + 1] = blendPlan[i].endFrameIndex;
		}

		return static_cast<int32_t>(v);
	} else {
		return -1;
	}
}

AnimationSet::AnimationSet(Netcode::Module::IGraphicsModule * graphics, Netcode::ArrayView<Netcode::Asset::Animation> animations, Netcode::ArrayView<Netcode::Asset::Bone> bones) : AnimationSet{} {
	animationStaticCbuffer = graphics->resources->CreateConstantBuffer(sizeof(AnimationStaticConstants));
	instanceCbuffer = graphics->resources->CreateConstantBuffer(sizeof(AnimInstanceConstants));

	AnimationStaticConstants cbufferData;
	ZeroMemory(&cbufferData, sizeof(AnimationStaticConstants));

	cbufferData.numAnimations = static_cast<uint32_t>(animations.Size());
	cbufferData.numBones = static_cast<uint32_t>(bones.Size());

	for(size_t i = 0; i < bones.Size(); ++i) {
		cbufferData.parentIndices[i] = bones[i].parentId;
		DirectX::XMMATRIX offsetMat = DirectX::XMLoadFloat4x4(&bones[i].transform);
		offsetMat = DirectX::XMMatrixTranspose(offsetMat);

		DirectX::XMStoreFloat4x4(&cbufferData.offsetMatrices[i], offsetMat);
	}

	uint32_t totalKeys = 0;
	uint32_t sumKeys = 0;
	for(size_t i = 0; i < animations.Size(); ++i) {
		cbufferData.startIndices[i] = sumKeys;
		sumKeys += animations[i].keysLength;
		totalKeys += animations[i].keysLength * animations[i].bonesLength;
	}

	graphics->resources->CopyConstants(animationStaticCbuffer, &cbufferData, sizeof(AnimationStaticConstants));

	graphics->resources->SetDebugName(animationStaticCbuffer, L"AnimStatic");

	resultBuffer = graphics->resources->CreateStructuredBuffer(BoneData::MAX_BONE_COUNT * MAX_INSTANCE_COUNT * 2 * sizeof(DirectX::XMFLOAT4X4),
		sizeof(DirectX::XMFLOAT4X4),
		Netcode::Graphics::ResourceType::PERMANENT_DEFAULT,
		Netcode::Graphics::ResourceState::NON_PIXEL_SHADER_RESOURCE | Netcode::Graphics::ResourceState::COPY_SOURCE,
		Netcode::Graphics::ResourceFlags::ALLOW_UNORDERED_ACCESS);

	resultReadbackBuffer = graphics->resources->CreateReadbackBuffer(BoneData::MAX_BONE_COUNT * MAX_INSTANCE_COUNT * 2 * sizeof(DirectX::XMFLOAT4X4),
		Netcode::Graphics::ResourceType::PERMANENT_READBACK,
		Netcode::Graphics::ResourceFlags::DENY_SHADER_RESOURCE);

	intermediateBuffer = graphics->resources->CreateStructuredBuffer(BoneData::MAX_BONE_COUNT * MAX_STATE_COUNT * MAX_INSTANCE_COUNT * sizeof(Netcode::Asset::AnimationKey),
		sizeof(Netcode::Asset::AnimationKey),
		Netcode::Graphics::ResourceType::PERMANENT_DEFAULT,
		Netcode::Graphics::ResourceState::UNORDERED_ACCESS,
		Netcode::Graphics::ResourceFlags::ALLOW_UNORDERED_ACCESS);

	animationKeysBuffer = graphics->resources->CreateStructuredBuffer(totalKeys * sizeof(Netcode::Asset::AnimationKey),
		sizeof(Netcode::Asset::AnimationKey),
		Netcode::Graphics::ResourceType::PERMANENT_DEFAULT,
		Netcode::Graphics::ResourceState::COPY_DEST,
		Netcode::Graphics::ResourceFlags::NONE);

	animKeysView = graphics->resources->CreateShaderResourceViews(3);
	animKeysView->CreateSRV(0, animationKeysBuffer);
	animKeysView->CreateUAV(1, intermediateBuffer);
	animKeysView->CreateUAV(2, resultBuffer);

	ctrlView = graphics->resources->CreateShaderResourceViews(MAX_INSTANCE_COUNT);
	constexpr uint32_t STRIDE = BoneData::MAX_BONE_COUNT * 2;
	for(uint32_t i = 0; i < MAX_INSTANCE_COUNT; ++i) {
		ctrlView->CreateSRV(i, resultBuffer, i * STRIDE, STRIDE);
	}

	Netcode::Graphics::UploadBatch uploadBatch;

	size_t offset = 0;
	for(const Netcode::Asset::Animation & anim : animations) {
		size_t itemSize = anim.keysLength * anim.bonesLength * sizeof(Netcode::Asset::AnimationKey);
		uploadBatch.Upload(animationKeysBuffer, anim.keys, itemSize, offset);
		offset += itemSize;
	}

	uploadBatch.ResourceBarrier(animationKeysBuffer,
		Netcode::Graphics::ResourceState::COPY_DEST,
		Netcode::Graphics::ResourceState::NON_PIXEL_SHADER_RESOURCE);
	graphics->frame->SyncUpload(uploadBatch);
}

std::shared_ptr<AnimationController> AnimationSet::CreateController() {
	if(freedControllers.empty()) {
		return MakeNewController();
	} else {
		return ReuseController();
	}
}
