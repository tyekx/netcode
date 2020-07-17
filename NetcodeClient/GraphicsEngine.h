#pragma once

#include <Netcode/Graphics/FrameGraph.h>
#include <Netcode/FancyIterators.hpp>
#include <Netcode/Graphics/ResourceDesc.h>
#include <Netcode/Vertex.h>
#include <DirectXPackedVector.h>
#include <Netcode/MathExt.h>
#include <variant>
#include "GameObject.h"
#include "AnimationSet.h"
#include "UI/Page.h"

using Netcode::GpuResourceRef;

using Netcode::Graphics::ResourceDesc;
using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;
using Netcode::Graphics::ResourceFlags;
using Netcode::Graphics::PrimitiveTopology;

using Netcode::Graphics::IResourceContext;
using Netcode::Graphics::IRenderContext;

/*


*/

struct RenderItem {
	GBuffer gbuffer;
	Material * material;
	PerObjectData * objectData;
	Netcode::ResourceViewsRef boneData;
	int32_t boneDataOffset;
	BoneData * debugBoneData;

	RenderItem(const ShadedMesh & shadedMesh, PerObjectData * objectData, Netcode::ResourceViewsRef boneData, int32_t boneDataOffset, BoneData * dbBoneData) :
		gbuffer{ shadedMesh.mesh->GetGBuffer() }, material{ shadedMesh.material.get() }, objectData{ objectData }, boneData{ boneData },
		boneDataOffset{ boneDataOffset }, debugBoneData{ dbBoneData } {

	}
};

class GraphicsEngine {
public:
	GpuResourceRef gbufferPass_DepthBuffer;
	GpuResourceRef gbufferPass_ColorRenderTarget;
	GpuResourceRef gbufferPass_NormalsRenderTarget;

	GpuResourceRef ssaoPass_BlurRenderTarget;
	GpuResourceRef ssaoPass_OcclusionRenderTarget;
	GpuResourceRef ssaoPass_RandomVectorTexture;

	GBuffer fsQuad;

	DXGI_FORMAT gbufferPass_DepthStencilFormat;

	Netcode::UInt2 backbufferSize;
	Netcode::UInt2 ssaoRenderTargetSize;

	Netcode::Module::IGraphicsModule * graphics;

	Netcode::ResourceViewsRef gbufferPass_RenderTargetViews;
	Netcode::ResourceViewsRef gbufferPass_DepthStencilView;
	Netcode::ResourceViewsRef lightingPass_ShaderResourceViews;

	Netcode::ResourceViewsRef skinningPass_ShaderResourceViews;

	Netcode::RootSignatureRef skinningPass_RootSignature;
	Netcode::PipelineStateRef skinningInterpolatePass_PipelineState;
	Netcode::PipelineStateRef skinningBlendPass_PipelineState;

	Netcode::RootSignatureRef skinnedGbufferPass_RootSignature;
	Netcode::PipelineStateRef skinnedGbufferPass_PipelineState;

	Netcode::RootSignatureRef gbufferPass_RootSignature;
	Netcode::PipelineStateRef gbufferPass_PipelineState;

	Netcode::RootSignatureRef lightingPass_RootSignature;
	Netcode::PipelineStateRef lightingPass_PipelineState;

	Netcode::RootSignatureRef ssaoOcclusionPass_RootSignature;
	Netcode::PipelineStateRef ssaoOcclusionPass_PipelineState;

	Netcode::RootSignatureRef ssaoBlurPass_RootSignature;
	Netcode::PipelineStateRef ssaoBlurPass_PipelineState;

	Netcode::SpriteBatchRef uiPass_SpriteBatch;

	uint64_t perFrameCbuffer;

	void SetPerFrameCb(IRenderContext * context, int slot) {
		if(perFrameCbuffer == 0) {
			perFrameCbuffer = context->SetConstants(slot, *perFrameData);
		} else {
			context->SetConstants(slot, perFrameCbuffer);
		}
	}

public:

	PerFrameData * perFrameData;
	SsaoData * ssaoData;

	Netcode::ScratchBuffer<std::shared_ptr<AnimationSet>> skinningPass_Input;
	Netcode::ScratchBuffer<RenderItem> skinnedGbufferPass_Input;
	Netcode::ScratchBuffer<RenderItem> gbufferPass_Input;
	std::shared_ptr<UI::Page> ui_Input;

private:

	void CreateFSQuad(Netcode::Module::IGraphicsModule * g) {
		struct PT_Vert {
			Netcode::Float3 position;
			Netcode::Float2 texCoord;
		};

		PT_Vert vData[6] = {
			{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
			{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
			{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
			{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
			{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
			{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }
		};

		fsQuad.vertexBuffer = g->resources->CreateVertexBuffer(sizeof(vData), sizeof(PT_Vert), ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
		fsQuad.vertexCount = 6;
		fsQuad.indexBuffer = 0;
		fsQuad.indexCount = 0;

		Netcode::Graphics::UploadBatch uploadBatch;
		uploadBatch.Upload(fsQuad.vertexBuffer, vData, sizeof(vData));
		uploadBatch.ResourceBarrier(fsQuad.vertexBuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

		g->frame->SyncUpload(uploadBatch);
	}

	void CreateSkinnedGbufferPassPermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		ilBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT);
		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();

		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"skinningPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"gbufferPass_Pixel.cso");

		auto rsBuilder = g->CreateRootSignatureBuilder();
		skinnedGbufferPass_RootSignature = rsBuilder->BuildFromShader(vs);

		Netcode::DepthStencilDesc depthStencilDesc;
		depthStencilDesc.backFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilFailOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilPassOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

		depthStencilDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.frontFace.stencilPassOp = Netcode::StencilOp::REPLACE;
		depthStencilDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::ALWAYS;

		depthStencilDesc.depthEnable = true;
		depthStencilDesc.stencilEnable = true;
		depthStencilDesc.stencilWriteMask = 0xFF;
		depthStencilDesc.stencilReadMask = 0xFF;

		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(skinnedGbufferPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilState(depthStencilDesc);
		psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT });
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		skinnedGbufferPass_PipelineState = psoBuilder->Build();
	}

	void CreateSkinningPassPermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef interpolateCS = shaderBuilder->LoadBytecode(L"skinningPassInterpolate_Compute.cso");
		Netcode::ShaderBytecodeRef blendCS = shaderBuilder->LoadBytecode(L"skinningPassBlend_Compute.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		skinningPass_RootSignature = rootSigBuilder->BuildFromShader(interpolateCS);

		auto cpsoBuilder = g->CreateCPipelineStateBuilder();
		cpsoBuilder->SetRootSignature(skinningPass_RootSignature);
		cpsoBuilder->SetComputeShader(interpolateCS);
		skinningInterpolatePass_PipelineState = cpsoBuilder->Build();

		cpsoBuilder->SetRootSignature(skinningPass_RootSignature);
		cpsoBuilder->SetComputeShader(blendCS);
		skinningBlendPass_PipelineState = cpsoBuilder->Build();
	}

	void CreateGbufferPassPermanentResources(Netcode::Module::IGraphicsModule * g) {
		gbufferPass_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"gbufferPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"gbufferPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		gbufferPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();

		Netcode::DepthStencilDesc depthStencilDesc;
		depthStencilDesc.backFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilFailOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilPassOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

		depthStencilDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
		depthStencilDesc.frontFace.stencilPassOp = Netcode::StencilOp::REPLACE;
		depthStencilDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::ALWAYS;

		depthStencilDesc.depthEnable = true;
		depthStencilDesc.stencilEnable = true;
		depthStencilDesc.stencilWriteMask = 0xFF;
		depthStencilDesc.stencilReadMask = 0xFF;

		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(gbufferPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilState(depthStencilDesc);
		psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT });
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		gbufferPass_PipelineState = psoBuilder->Build();

		gbufferPass_RenderTargetViews = graphics->resources->CreateRenderTargetViews(2);
		gbufferPass_DepthStencilView = graphics->resources->CreateDepthStencilView();

		CreateGbufferPassSizeDependentResources();
	}

	void CreateGbufferPassSizeDependentResources() {
		gbufferPass_ColorRenderTarget.reset();
		gbufferPass_NormalsRenderTarget.reset();
		gbufferPass_DepthBuffer.reset();

		gbufferPass_ColorRenderTarget = graphics->resources->CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
		gbufferPass_NormalsRenderTarget = graphics->resources->CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
		gbufferPass_DepthBuffer = graphics->resources->CreateDepthStencil(gbufferPass_DepthStencilFormat, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ);

		graphics->resources->SetDebugName(gbufferPass_ColorRenderTarget, L"GBuffer:Color");
		graphics->resources->SetDebugName(gbufferPass_NormalsRenderTarget, L"GBuffer:Normals");
		graphics->resources->SetDebugName(gbufferPass_DepthBuffer, L"GBuffer:Depth");

		gbufferPass_RenderTargetViews->CreateRTV(0, gbufferPass_ColorRenderTarget.get());
		gbufferPass_RenderTargetViews->CreateRTV(1, gbufferPass_NormalsRenderTarget.get());
		gbufferPass_DepthStencilView->CreateDSV(gbufferPass_DepthBuffer.get());
	}

	void CreateSSAOBlurPassPermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"ssaoPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"ssaoBlurPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		ssaoBlurPass_RootSignature = rootSigBuilder->BuildFromShader(ps);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

		Netcode::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = false;
		depthDesc.stencilEnable = false;

		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();
		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(ssaoBlurPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R32_FLOAT });
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);
		ssaoBlurPass_PipelineState = psoBuilder->Build();
	}

	void CreateSSAOOcclusionPassPermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"ssaoPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"ssaoOcclusionPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		ssaoOcclusionPass_RootSignature = rootSigBuilder->BuildFromShader(ps);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

		Netcode::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = false;
		depthDesc.stencilEnable = false;

		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();
		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(ssaoOcclusionPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R32_FLOAT });
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);
		ssaoOcclusionPass_PipelineState = psoBuilder->Build();

		ssaoPass_RandomVectorTexture = g->resources->CreateTexture2D(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);

		std::unique_ptr<DirectX::PackedVector::XMCOLOR[]> colors = std::make_unique<DirectX::PackedVector::XMCOLOR[]>(256 * 256);
		for(int i = 0; i < 256; ++i)
		{
			for(int j = 0; j < 256; ++j)
			{
				Netcode::Float3 v(Netcode::RandomFloat(), Netcode::RandomFloat(), Netcode::RandomFloat());

				colors[i * 256 + j] = DirectX::PackedVector::XMCOLOR(v.z, v.y, v.x, 0.0f);
			}
		}

		Netcode::Graphics::UploadBatch upload;
		upload.Upload(ssaoPass_RandomVectorTexture, colors.get(), 256 * 256 * sizeof(DirectX::PackedVector::XMCOLOR));
		upload.ResourceBarrier(ssaoPass_RandomVectorTexture, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
		g->frame->SyncUpload(upload);

		CreateSSAOOcclusionPassSizeDependentResources();
	}

	void CreateSSAOOcclusionPassSizeDependentResources() {
		ssaoRenderTargetSize = Netcode::UInt2{ backbufferSize.x / 2, backbufferSize.y / 2 };

		ssaoRenderTargetSize.x = (ssaoRenderTargetSize.x == 0) ? 1 : ssaoRenderTargetSize.x;
		ssaoRenderTargetSize.y = (ssaoRenderTargetSize.y == 0) ? 1 : ssaoRenderTargetSize.y;

		ssaoPass_OcclusionRenderTarget.reset();
		ssaoPass_BlurRenderTarget.reset();

		ssaoPass_OcclusionRenderTarget = graphics->resources->CreateRenderTarget(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y, DXGI_FORMAT_R32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
		ssaoPass_BlurRenderTarget = graphics->resources->CreateRenderTarget(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y, DXGI_FORMAT_R32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
		
		lightingPass_ShaderResourceViews->CreateSRV(3, ssaoPass_BlurRenderTarget.get());
	}

	void CreateLightingPassPermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"lightingPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"lightingPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		lightingPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

		Netcode::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = false;
		depthDesc.stencilEnable = true;
		depthDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilPassOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::EQUAL;
		depthDesc.stencilReadMask = 0xFF;
		depthDesc.stencilWriteMask = 0x00;
		depthDesc.depthWriteMaskZero = true;
		depthDesc.backFace = depthDesc.frontFace;
		depthDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();
		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(lightingPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);
		lightingPass_PipelineState = psoBuilder->Build();

		lightingPass_ShaderResourceViews = graphics->resources->CreateShaderResourceViews(4);
		CreateLightingPassResourceViews();
	}

	void CreateLightingPassResourceViews() {
		lightingPass_ShaderResourceViews->CreateSRV(0, gbufferPass_ColorRenderTarget.get());
		lightingPass_ShaderResourceViews->CreateSRV(1, gbufferPass_NormalsRenderTarget.get());
		lightingPass_ShaderResourceViews->CreateSRV(2, gbufferPass_DepthBuffer.get());
	}

	void CreateUIPassPermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();

		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"Netcode_SpriteVS.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"Netcode_SpritePS.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		auto rootSignature = rootSigBuilder->BuildFromShader(vs);

		Netcode::BlendDesc blendState;
		Netcode::RenderTargetBlendDesc rt0Blend;
		rt0Blend.blendEnable = true;
		rt0Blend.logicOpEnable = false;
		rt0Blend.srcBlend = Netcode::BlendMode::SRC_ALPHA;
		rt0Blend.destBlend = Netcode::BlendMode::INV_SRC_ALPHA;
		rt0Blend.blendOp = Netcode::BlendOp::ADD;
		rt0Blend.srcBlendAlpha = Netcode::BlendMode::ONE;
		rt0Blend.destBlendAlpha = Netcode::BlendMode::INV_SRC_ALPHA;
		rt0Blend.blendOpAlpha = Netcode::BlendOp::ADD;
		rt0Blend.logicOp = Netcode::LogicOp::NOOP;
		rt0Blend.renderTargetWriteMask = 0x0F;

		blendState.alphaToCoverageEnabled = false;
		blendState.independentAlphaEnabled = false;
		blendState.rtBlend[0] = rt0Blend;

		Netcode::RasterizerDesc rasterizerState;
		rasterizerState.fillMode = Netcode::FillMode::SOLID;
		rasterizerState.cullMode = Netcode::CullMode::NONE;
		rasterizerState.frontCounterClockwise = false;
		rasterizerState.depthBias = 0;
		rasterizerState.depthBiasClamp = 0.0f;
		rasterizerState.slopeScaledDepthBias = 0.0f;
		rasterizerState.depthClipEnable = true;
		rasterizerState.multisampleEnable = true;
		rasterizerState.antialiasedLineEnable = false;
		rasterizerState.forcedSampleCount = 0;
		rasterizerState.conservativeRaster = false;

		Netcode::DepthStencilDesc depthStencilDesc;
		depthStencilDesc.depthEnable = false;
		depthStencilDesc.stencilEnable = false;
		depthStencilDesc.depthWriteMaskZero = true;

		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetRootSignature(rootSignature);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetBlendState(blendState);
		psoBuilder->SetRasterizerState(rasterizerState);
		psoBuilder->SetDepthStencilState(depthStencilDesc);

		auto pipelineState = psoBuilder->Build();

		Netcode::SpriteBatchBuilderRef spriteBatchBuilder = g->CreateSpriteBatchBuilder();
		spriteBatchBuilder->SetPipelineState(std::move(pipelineState));
		spriteBatchBuilder->SetRootSignature(std::move(rootSignature));
		uiPass_SpriteBatch = spriteBatchBuilder->Build();
	}


	void CreateSkinningPass(Netcode::FrameGraphBuilderRef frameGraphBuilder) {
		frameGraphBuilder->CreateRenderPass("Skinning",
			[&](IResourceContext * context) -> void {
			
				context->UseComputeContext();

				for(std::shared_ptr<AnimationSet> & animSet : skinningPass_Input) {
					animSet->UploadConstants(context);
				}

			},
			[&](IRenderContext * context) -> void {

			context->SetRootSignature(skinningPass_RootSignature);

			for(std::shared_ptr<AnimationSet> & animSet : skinningPass_Input) {
				animSet->BindResources(context);

				context->SetPipelineState(skinningInterpolatePass_PipelineState);
				context->Dispatch(8, animSet->GetNumInstances(), 1);
				
				context->UnorderedAccessBarrier(animSet->GetIntermediateResource());
				context->FlushResourceBarriers();
				
				context->SetPipelineState(skinningBlendPass_PipelineState);
				context->Dispatch(1, animSet->GetNumInstances(), 1);

				animSet->CopyResults(context);
			}

		});
	}

	void CreateSkinnedGbufferPass(Netcode::FrameGraphBuilderRef frameGraphBuilder) {
		frameGraphBuilder->CreateRenderPass("Skinned Gbuffer",
		[&](IResourceContext * context) -> void {

			context->Writes(3);

			context->Writes(gbufferPass_ColorRenderTarget);
			context->Writes(gbufferPass_NormalsRenderTarget);
			context->Writes(gbufferPass_DepthBuffer);

		},
		[&](IRenderContext * context) -> void {
			context->SetRootSignature(skinnedGbufferPass_RootSignature);
			context->SetPipelineState(skinnedGbufferPass_PipelineState);

			context->ResourceBarrier(gbufferPass_ColorRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
			context->ResourceBarrier(gbufferPass_NormalsRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
			context->ResourceBarrier(gbufferPass_DepthBuffer, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ, ResourceState::DEPTH_WRITE);
			context->FlushResourceBarriers();

			context->SetRenderTargets(gbufferPass_RenderTargetViews, gbufferPass_DepthStencilView);
			context->SetScissorRect();
			context->SetViewport();
			context->ClearRenderTarget(0);
			context->ClearRenderTarget(1);
			context->ClearDepthStencil();
			context->SetStencilReference(0xFF);

			context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
			
			for(RenderItem & item : skinnedGbufferPass_Input) {
				skinnedGbufferPass_Input.begin()->material->Apply(context);
				context->SetConstants(1, *item.objectData);
				SetPerFrameCb(context, 2);
				context->SetConstants(3, *item.debugBoneData);
				//context->SetShaderResources(3, item.boneData, item.boneDataOffset);
				context->SetVertexBuffer(item.gbuffer.vertexBuffer);
				context->SetIndexBuffer(item.gbuffer.indexBuffer);
				context->DrawIndexed(item.gbuffer.indexCount);
			}
		}
		);
	}

	void CreateGbufferPass(Netcode::FrameGraphBuilderRef frameGraphBuilder) {
		frameGraphBuilder->CreateRenderPass("Gbuffer", [&](IResourceContext * context) -> void {

			context->Reads(3);
			context->Writes(gbufferPass_ColorRenderTarget);
			context->Writes(gbufferPass_NormalsRenderTarget);
			context->Writes(gbufferPass_DepthBuffer);

		},
			[&](IRenderContext * context) -> void {

			bool isBound = false;
			void * objectData = nullptr;

			context->SetRootSignature(gbufferPass_RootSignature);
			context->SetPipelineState(gbufferPass_PipelineState);
			context->SetRenderTargets(gbufferPass_RenderTargetViews, gbufferPass_DepthStencilView);
			context->SetScissorRect();
			context->SetViewport();
			context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
			context->SetStencilReference(0xFF);
			
			for(RenderItem & item : gbufferPass_Input) {
				item.material->Apply(context);

				if(objectData != item.objectData) {
					context->SetConstants(1, *item.objectData);
					objectData = item.objectData;
				}

				if(!isBound) {
					SetPerFrameCb(context, 2);
					isBound = true;
				}

				context->SetVertexBuffer(item.gbuffer.vertexBuffer);
				context->SetIndexBuffer(item.gbuffer.indexBuffer);
				context->DrawIndexed(item.gbuffer.indexCount);
			}
			/*
			gbufferPass_Input.begin()->material->Apply(context);
			context->SetConstants(1, *(gbufferPass_Input.begin()->objectData));
			SetPerFrameCb(context, 2);
			context->SetVertexBuffer(fsQuad.vertexBuffer);
			context->Draw(fsQuad.vertexCount);*/
			context->ResourceBarrier(gbufferPass_ColorRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
			context->ResourceBarrier(gbufferPass_NormalsRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
			context->ResourceBarrier(gbufferPass_DepthBuffer, ResourceState::DEPTH_WRITE, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ);
			context->FlushResourceBarriers();
		});
	}

	void CreateSSAOOcclusionPass(Netcode::FrameGraphBuilderRef frameGraphBuilder) {
		frameGraphBuilder->CreateRenderPass("SSAO Occlusion", [&](IResourceContext * context) -> void {

			context->Reads(gbufferPass_NormalsRenderTarget);
			context->Reads(gbufferPass_DepthBuffer);

			context->Writes(ssaoPass_OcclusionRenderTarget);

		},
			[&](IRenderContext * context) -> void {
			context->SetRootSignature(ssaoOcclusionPass_RootSignature);
			context->SetPipelineState(ssaoOcclusionPass_PipelineState);

			context->SetViewport(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y);
			context->SetScissorRect(0, ssaoRenderTargetSize.x, 0, ssaoRenderTargetSize.y);
			context->ResourceBarrier(ssaoPass_OcclusionRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
			context->FlushResourceBarriers();
			context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
			context->SetRenderTargets(ssaoPass_OcclusionRenderTarget, 0);
			context->ClearRenderTarget(0);

			SetPerFrameCb(context, 0);
			context->SetConstants(1, *ssaoData);
			context->SetShaderResources(2, { gbufferPass_NormalsRenderTarget, gbufferPass_DepthBuffer, ssaoPass_RandomVectorTexture });

			context->SetVertexBuffer(fsQuad.vertexBuffer);
			context->Draw(fsQuad.vertexCount);

			context->SetRenderTargets(0, 0);
			context->ResourceBarrier(ssaoPass_OcclusionRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
			context->FlushResourceBarriers();
		});
	}

	void CreateSSAOBlurPass(Netcode::FrameGraphBuilderRef frameGraphBuilder) {
		frameGraphBuilder->CreateRenderPass("SSAO Blur", [&](IResourceContext * context) -> void {

			context->Reads(ssaoPass_OcclusionRenderTarget);
			context->Writes(ssaoPass_BlurRenderTarget);

		},
			[&](IRenderContext * context) -> void {

			context->SetRootSignature(ssaoBlurPass_RootSignature);
			context->SetPipelineState(ssaoBlurPass_PipelineState);
			context->SetViewport(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y);
			context->SetScissorRect(0, ssaoRenderTargetSize.x, 0, ssaoRenderTargetSize.y);
			context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);


			ssaoData->invRenderTargetSize = Netcode::Float2{
				1.0f / static_cast<float>(ssaoRenderTargetSize.x),
				1.0f / static_cast<float>(ssaoRenderTargetSize.y)
			};

			uint32_t isHorizontal = 0;

			GpuResourceRef renderTargets[2] = {
				ssaoPass_OcclusionRenderTarget, ssaoPass_BlurRenderTarget
			};

			for(uint32_t i = 0; i < 3; ++i) {
				GpuResourceRef currentRenderTarget = renderTargets[(i + 1) % 2];
				GpuResourceRef sourceTexture = renderTargets[i % 2];

				context->ResourceBarrier(currentRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
				context->FlushResourceBarriers();

				context->SetRenderTargets(currentRenderTarget, 0);
				context->ClearRenderTarget(0);

				context->SetShaderResources(0, { sourceTexture, gbufferPass_NormalsRenderTarget, gbufferPass_DepthBuffer });
				context->SetConstants(1, *ssaoData);

				SetPerFrameCb(context, 2);
				context->SetRootConstants(3, &isHorizontal, 1);
				context->SetVertexBuffer(fsQuad.vertexBuffer);
				context->Draw(fsQuad.vertexCount);

				context->ResourceBarrier(currentRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
				context->FlushResourceBarriers();

				if(isHorizontal == 0) {
					isHorizontal = 0xFFFFFFFF;
				} else {
					isHorizontal = 0;
				}
			}
		});
	}

	void CreateLightingPass(Netcode::FrameGraphBuilderRef frameGraphBuilder) {
		frameGraphBuilder->CreateRenderPass("Lighting", [&](IResourceContext * context) -> void {

			context->Reads(ssaoPass_BlurRenderTarget);
			context->Reads(gbufferPass_ColorRenderTarget);
			context->Reads(gbufferPass_DepthBuffer);
			context->Reads(gbufferPass_NormalsRenderTarget);

			context->Writes(2);
			context->Writes(nullptr);

		}, [&](IRenderContext * context) -> void {
			context->SetRootSignature(lightingPass_RootSignature);
			context->SetPipelineState(lightingPass_PipelineState);
			context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
			context->SetScissorRect();
			context->SetViewport();
			context->SetStencilReference(0xFF);
			context->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
			SetPerFrameCb(context, 0);
			context->SetShaderResources(1, lightingPass_ShaderResourceViews);

			// @TODO: set light data

			context->SetVertexBuffer(fsQuad.vertexBuffer);
			context->Draw(fsQuad.vertexCount);
		});
	}

	void CreateUIPass(Netcode::FrameGraphBuilderRef frameGraphBuilder) {
		frameGraphBuilder->CreateRenderPass("uiPass", [&](IResourceContext * context) -> void {
			
			context->Reads(2);
			context->Writes(nullptr);

		},
		[&](IRenderContext * context) -> void {

			Netcode::UInt2 ss = ui_Input->WindowSize();
			Netcode::Matrix vp =
				Netcode::LookToMatrix(Netcode::Float3::Zero, Netcode::Float3{ 0.0f, 0.0f, 1.0f }, Netcode::Float3::UnitY) *
				Netcode::OrtographicMatrix(static_cast<float>(ss.x), static_cast<float>(ss.y), 0.0f, UI::Control::MAX_DEPTH);
			Netcode::Matrix tex = Netcode::Float4x4{
				-1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				-1.0f, 1.0f, 0.0f, 1.0f
			};


			context->SetRenderTargets(0, 0);
			uiPass_SpriteBatch->BeginRecord(context, (vp * tex).Transpose());
			ui_Input->Render(uiPass_SpriteBatch.get());
			uiPass_SpriteBatch->EndRecord();
		});
	}

	void CreatePostProcessPass(Netcode::FrameGraphBuilderRef frameGraphBuilder) {
		frameGraphBuilder->CreateRenderPass("postProcessPass", [&](IResourceContext * context) -> void {



		},
		[&](IRenderContext * context) -> void {

			//context->SetRenderTargets(0, 0);

		});
	}

	GpuResourceRef cloudynoonTexture;
	Netcode::ResourceViewsRef cloudynoonView;

public:

	void CreateBackgroundPassPermanentResources(Netcode::Module::IGraphicsModule * g) {
		Netcode::TextureBuilderRef textureBuilder = graphics->CreateTextureBuilder();
		textureBuilder->LoadTextureCube(L"cloudynoon.dds");
		Netcode::TextureRef cloudynoon = textureBuilder->Build();

		ASSERT(cloudynoon->GetImageCount() == 6, "bad image count");

		const Netcode::Image * img = cloudynoon->GetImage(0, 0, 0);

		cloudynoonTexture = g->resources->CreateTextureCube(img->width, img->height, img->format, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);

		g->resources->SetDebugName(cloudynoonTexture, L"Cloudynoon TextureCube");

		Netcode::Graphics::UploadBatch batch;
		batch.Upload(cloudynoonTexture, cloudynoon);
		batch.ResourceBarrier(cloudynoonTexture, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
		g->frame->SyncUpload(batch);

		cloudynoonView = g->resources->CreateShaderResourceViews(1);
		cloudynoonView->CreateSRV(0, cloudynoonTexture.get());

		Netcode::ShaderBuilderRef shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"envmapPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"envmapPass_Pixel.cso");

		Netcode::RootSignatureBuilderRef rootSigBuilder = g->CreateRootSignatureBuilder();
		envmapPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();


		Netcode::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = false;
		depthDesc.stencilEnable = true;
		depthDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilPassOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::EQUAL;
		depthDesc.stencilReadMask = 0xFF;
		depthDesc.stencilWriteMask = 0x00;
		depthDesc.depthWriteMaskZero = true;
		depthDesc.backFace = depthDesc.frontFace;
		depthDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

		Netcode::GPipelineStateBuilderRef psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetRootSignature(envmapPass_RootSignature);
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);

		envmapPass_PipelineState = psoBuilder->Build();
	}

	Netcode::RootSignatureRef envmapPass_RootSignature;
	Netcode::PipelineStateRef envmapPass_PipelineState;

	void CreateBackgroundPass(Netcode::FrameGraphBuilderRef builder) {
		builder->CreateRenderPass("Background", [this](IResourceContext * ctx) ->void {
			ctx->Reads(gbufferPass_DepthBuffer);
			ctx->Writes(2);
			ctx->Writes(nullptr);
		},
			[this](IRenderContext * ctx) -> void {

			ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
			ctx->SetScissorRect();
			ctx->SetViewport();
			ctx->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
			ctx->SetStencilReference(0);

			ctx->SetRootSignature(envmapPass_RootSignature);
			ctx->SetPipelineState(envmapPass_PipelineState);

			SetPerFrameCb(ctx, 0);
			ctx->SetShaderResources(1, cloudynoonView);

			ctx->SetVertexBuffer(fsQuad.vertexBuffer);
			ctx->Draw(fsQuad.vertexCount);
		});
	}

	void CreateDebugPrimPass(Netcode::FrameGraphBuilderRef builder) {
		builder->CreateRenderPass("Debug", [this](IResourceContext * ctx) ->void {
			ctx->Reads(2);
			ctx->Writes(nullptr);
			graphics->debug->UploadResources(ctx);
		},
		[this](IRenderContext * ctx) -> void {
			graphics->debug->Draw(ctx, perFrameData->ViewProj);
		});
	}

	void Reset() {
		skinningPass_Input.Clear();
		skinnedGbufferPass_Input.Clear();
		gbufferPass_Input.Clear();
		perFrameCbuffer = 0;
	}

	void OnResize(int x, int y) {
		Netcode::UInt2 newSize{ static_cast<uint32_t>(x), static_cast<uint32_t>(y) };

		if(newSize.x != backbufferSize.x || newSize.y != backbufferSize.y) {
			backbufferSize = newSize;
			if(backbufferSize.x != 0 && backbufferSize.y != 0) {
				CreateGbufferPassSizeDependentResources();
				CreateLightingPassResourceViews();
				CreateSSAOOcclusionPassSizeDependentResources();
			}
		}
	}

	void CreatePermanentResources(Netcode::Module::IGraphicsModule * g) {
		graphics = g;
		backbufferSize = g->GetBackbufferSize();
		CreateGbufferPassPermanentResources(g);
		CreateSkinningPassPermanentResources(g);
		CreateSkinnedGbufferPassPermanentResources(g);
		CreateLightingPassPermanentResources(g);
		CreateBackgroundPassPermanentResources(g);
		CreateSSAOBlurPassPermanentResources(g);
		CreateSSAOOcclusionPassPermanentResources(g);
		CreateUIPassPermanentResources(g);
		CreateFSQuad(g);
	}

	void CreateFrameGraph(Netcode::FrameGraphBuilderRef builder) {
		CreateSkinnedGbufferPass(builder);
		CreateGbufferPass(builder);
		CreateSSAOOcclusionPass(builder);
		CreateSSAOBlurPass(builder);
		CreateLightingPass(builder);
		CreateBackgroundPass(builder);
		CreateUIPass(builder);
		CreateDebugPrimPass(builder);
	}

	void CreateComputeFrameGraph(Netcode::FrameGraphBuilderRef builder) {
		CreateSkinningPass(builder);
	}

	void ReadbackComputeResults() {
		for(auto & animSet : skinningPass_Input) {
			graphics->resources->Readback(animSet->GetResultReadbackBuffer(), animSet->GetData()->BindTransform, animSet->GetNumInstances() * sizeof(BoneData));
			animSet->Clear();
		}
	}

};
