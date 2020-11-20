#pragma once

#include <DirectXTex.h>
#include <Netcode/UI/PageManager.h>
#include <Netcode/FancyIterators.hpp>
#include <Netcode/Graphics/FrameGraph.h>
#include <Netcode/Graphics/UploadBatch.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include <Netcode/Graphics/ResourceDesc.h>
#include <Netcode/Vertex.h>
#include <DirectXPackedVector.h>
#include <Netcode/MathExt.h>
#include <variant>
#include "GameObject.h"
#include "AnimationSet.h"

using Netcode::GpuResource;

using Netcode::Graphics::IResourceContext;
using Netcode::Graphics::IRenderContext;

/*


*/

struct RenderItem {
	GBuffer gbuffer;
	Ref<Netcode::Material> material;
	PerObjectData * objectData;
	Ref<Netcode::ResourceViews> boneData;
	int32_t boneDataOffset;
	BoneData * debugBoneData;

	RenderItem(const ShadedMesh & shadedMesh, PerObjectData * objectData, Ref<Netcode::ResourceViews> boneData, int32_t boneDataOffset, BoneData * dbBoneData) :
		gbuffer{ shadedMesh.mesh->GetGBuffer() }, material{ shadedMesh.material }, objectData{ objectData }, boneData{ boneData },
		boneDataOffset{ boneDataOffset }, debugBoneData{ dbBoneData } {

	}
};

/*
 * decides where to connect the item to the render pipeline and what LOD to use based on the content of the components
 * void AddRenderInput(Ptr<Transform> transformComponent, Ptr<Model> modelComponent, Ptr<Animatiom> animationComponent) 
 * renderLayer: decides if we need to put it in a special bucket
 */

class GraphicsEngine {
public:
	Ref<GpuResource> gbufferPass_DepthBuffer;
	Ref<GpuResource> lightingData_StructuredBuffer;
	//Ref<GpuResource> gbufferPass_ColorRenderTarget;
	//Ref<GpuResource> gbufferPass_NormalsRenderTarget;
	//Ref<GpuResource> gbufferPass_SpecularRenderTarget;

	Ref<GpuResource> ssaoPass_BlurRenderTarget;
	Ref<GpuResource> ssaoPass_OcclusionRenderTarget;
	Ref<GpuResource> ssaoPass_RandomVectorTexture;

	Ref<GpuResource> prefilteredEnvmap;
	Ref<GpuResource> preIntegratedBrdf;
	Ref<Netcode::ResourceViews> prefilteredSplitSumViews;

	GBuffer fsQuad;

	DXGI_FORMAT gbufferPass_DepthStencilFormat;

	Netcode::UInt2 backbufferSize;
	Netcode::UInt2 ssaoRenderTargetSize;

	Netcode::Module::IGraphicsModule * graphics;

	//Ref<Netcode::ResourceViews> gbufferPass_RenderTargetViews;
	Ref<Netcode::ResourceViews> gbufferPass_DepthStencilView;
	Ref<Netcode::ResourceViews> lightingPass_ShaderResourceViews;

	Ref<Netcode::ResourceViews> skinningPass_ShaderResourceViews;

	Ref<Netcode::RootSignature> skinningPass_RootSignature;
	Ref<Netcode::PipelineState> skinningInterpolatePass_PipelineState;
	Ref<Netcode::PipelineState> skinningBlendPass_PipelineState;

	Ref<Netcode::RootSignature> skinnedGbufferPass_RootSignature;
	Ref<Netcode::PipelineState> skinnedGbufferPass_PipelineState;

	Ref<Netcode::RootSignature> gbufferPass_RootSignature;
	Ref<Netcode::PipelineState> gbufferPass_PipelineState;

	Ref<Netcode::RootSignature> lightingPass_RootSignature;
	Ref<Netcode::PipelineState> lightingPass_PipelineState;

	Ref<Netcode::RootSignature> ssaoOcclusionPass_RootSignature;
	Ref<Netcode::PipelineState> ssaoOcclusionPass_PipelineState;

	Ref<Netcode::RootSignature> ssaoBlurPass_RootSignature;
	Ref<Netcode::PipelineState> ssaoBlurPass_PipelineState;

	Ref<Netcode::RootSignature> envmapPass_RootSignature;
	Ref<Netcode::PipelineState> envmapPass_PipelineState;

	Ref<Netcode::RootSignature> IBLPreFilterPass_RootSignature;
	Ref<Netcode::PipelineState> IBLPreFilterPass_PipelineState;

	Ref<Netcode::RootSignature> IBLPreIntegratePass_RootSignature;
	Ref<Netcode::PipelineState> IBLPreIntegratePass_PipelineState;

	Ref<Netcode::SpriteBatch> uiPass_SpriteBatch;

	uint64_t perFrameCbuffer;

	void SetPerFrameCb(IRenderContext * context, int slot);

public:

	PerFrameData * perFrameData;
	SsaoData * ssaoData;

	std::vector<Ref<AnimationSet>> skinningPass_Input;
	std::vector<RenderItem> skinnedGbufferPass_Input;
	std::vector<RenderItem> gbufferPass_Input;
	std::vector<Netcode::Light> * sceneLights;
	Netcode::UI::PageManager* ui_Input;

private:

	void SetMaterialData(Ptr<Netcode::Material> mat, int32_t slot, IRenderContext * ctx);

	void CreateFSQuad();

	void CreateSkinnedGbufferPassPermanentResources();

	void CreateSkinningPassPermanentResources();

	void CreateGbufferPassPermanentResources();

	void CreateGbufferPassSizeDependentResources();

	void CreateSSAOBlurPassPermanentResources();

	void CreateSSAOOcclusionPassPermanentResources();

	void CreateSSAOOcclusionPassSizeDependentResources();

	//void CreateLightingPassPermanentResources();

	void CreateBackgroundPassPermanentResources();

	//void CreateLightingPassResourceViews();

	void CreateUIPassPermanentResources();

	void CreatePreGbufferPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);
	
	void CreateSkinningPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	void CreateSkinnedGbufferPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	void CreateGbufferPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	void CreatePostGbufferPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	//void CreateSSAOOcclusionPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	//void CreateSSAOBlurPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	//void CreateLightingPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	void CreateUIPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	void CreatePostProcessPass(Ptr<Netcode::FrameGraphBuilder> frameGraphBuilder);

	void CreateBackgroundPass(Ptr<Netcode::FrameGraphBuilder> builder);

	void CreateDebugPrimPass(Ptr<Netcode::FrameGraphBuilder> builder);

public:

	void Reset();

	void OnResize(int x, int y);

	void CreatePermanentResources(Netcode::Module::IGraphicsModule * g);

	void CreateFrameGraph(Ptr<Netcode::FrameGraphBuilder> builder);

	void CreateComputeFrameGraph(Ptr<Netcode::FrameGraphBuilder> builder);

	Ref<GpuResource> PreIntegrateBrdf(Ptr<Netcode::FrameGraphBuilder> builder);

	Ref<GpuResource> PrefilterEnvMap(Ptr<Netcode::FrameGraphBuilder> builder, Ref<GpuResource> sourceTexture);

	void SetGlobalEnvMap(Ref<GpuResource> preEnvMap, Ref<GpuResource> preBrdfIntegral);

	void ReadbackComputeResults();

};
