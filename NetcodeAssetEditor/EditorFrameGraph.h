#pragma once

#include <Netcode/Graphics/FrameGraph.h>
#include <Netcode/Modules.h>

#include "GBuffer.h"
#include "ConstantBufferTypes.h"

using Netcode::Graphics::ResourceState;
using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceFlags;
using Netcode::Graphics::ResourceDesc;
using Netcode::Graphics::PrimitiveTopology;
using Netcode::Graphics::ResourceDimension;
using Netcode::Graphics::IRenderContext;
using Netcode::Graphics::IResourceContext;

class EditorFrameGraph {

	GBuffer fsQuad;

	Netcode::RootSignatureRef gbufferPass_RootSignature;
	Netcode::PipelineStateRef gbufferPass_PipelineState;

	Netcode::RootSignatureRef boneVisibilityPass_RootSignature;
	Netcode::PipelineStateRef boneVisibilityPass_PipelineState;

	Netcode::RootSignatureRef envmapPass_RootSignature;
	Netcode::PipelineStateRef envmapPass_PipelineState;

	Netcode::RootSignatureRef colliderPass_RootSignature;
	Netcode::PipelineStateRef colliderPass_PipelineState;

	uint64_t cloudynoonTexture;
	Netcode::ResourceViewsRef cloudynoonView;

	uint64_t gbufferPass_DepthStencil;
	Netcode::ResourceViewsRef gbufferPass_DepthStencilView;



	void CreateFSQuad(Netcode::Module::IGraphicsModule * g) {
		struct PT_Vert {
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT2 texCoord;
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

	void Create_GBufferPass_PermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		ilBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT);
		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();


		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"Editor_GBufferPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"Editor_GBufferPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		gbufferPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

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
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		gbufferPass_PipelineState = psoBuilder->Build();

		gbufferPass_DepthStencil = g->resources->CreateDepthStencil(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::DEPTH_READ);
		gbufferPass_DepthStencilView = g->resources->CreateDepthStencilView();
		gbufferPass_DepthStencilView->CreateDSV(gbufferPass_DepthStencil);
	}

	void Create_BoneVisibilityPass_PermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		ilBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT);
		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();

		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"Editor_BoneVisibilityPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"Editor_BoneVisibilityPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		boneVisibilityPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		Netcode::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = true;
		depthDesc.depthFunc = Netcode::ComparisonFunc::LESS_EQUAL;
		depthDesc.stencilEnable = false;
		depthDesc.frontFace.stencilDepthFailOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilFailOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilPassOp = Netcode::StencilOp::KEEP;
		depthDesc.frontFace.stencilFunc = Netcode::ComparisonFunc::NEVER;
		depthDesc.stencilReadMask = 0x00;
		depthDesc.stencilWriteMask = 0x00;
		depthDesc.depthWriteMaskZero = true;
		depthDesc.backFace = depthDesc.frontFace;
		depthDesc.backFace.stencilFunc = Netcode::ComparisonFunc::NEVER;

		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(boneVisibilityPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilState(depthDesc);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::POINT);
		boneVisibilityPass_PipelineState = psoBuilder->Build();
	}

	void Create_BackgroundPass_PermanentResources(Netcode::Module::IGraphicsModule * g) {
		Netcode::TextureBuilderRef textureBuilder = g->CreateTextureBuilder();
		textureBuilder->LoadTextureCube(L"cloudynoon.dds");
		Netcode::TextureRef cloudynoon = textureBuilder->Build();

		const Netcode::Image * img = cloudynoon->GetImage(0, 0, 0);

		cloudynoonTexture = g->resources->CreateTextureCube(img->width, img->height, img->format, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);

		g->resources->SetDebugName(cloudynoonTexture, L"Cloudynoon TextureCube");

		Netcode::Graphics::UploadBatch batch;
		batch.Upload(cloudynoonTexture, cloudynoon);
		batch.ResourceBarrier(cloudynoonTexture, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
		g->frame->SyncUpload(batch);

		cloudynoonView = g->resources->CreateShaderResourceViews(1);
		cloudynoonView->CreateSRV(0, cloudynoonTexture);

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
		psoBuilder->SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetRootSignature(envmapPass_RootSignature);
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);

		envmapPass_PipelineState = psoBuilder->Build();
	}

	void Create_ColliderPass_PermanentResource(Netcode::Module::IGraphicsModule * g) {
		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		Netcode::InputLayoutRef inputLayout = ilBuilder->Build();

		auto shaderBuilder = g->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"Editor_ColliderPass_Vertex.cso");
		Netcode::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"Editor_ColliderPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		colliderPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		Netcode::DepthStencilDesc depthStencilDesc;
		depthStencilDesc.depthEnable = true;
		depthStencilDesc.stencilEnable = false;
		depthStencilDesc.depthWriteMaskZero = true;

		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(colliderPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilState(depthStencilDesc);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::LINE);
		colliderPass_PipelineState = psoBuilder->Build();
	}

	void CreateGbufferPass(Netcode::FrameGraphBuilderRef builder) {
		builder->CreateRenderPass("Gbuffer pass", [this](IResourceContext * ctx) ->void {
			ctx->Writes(0);
			ctx->Writes(gbufferPass_DepthStencil);
		},
			[this](IRenderContext * ctx) -> void {

			ctx->ResourceBarrier(gbufferPass_DepthStencil, ResourceState::DEPTH_READ, ResourceState::DEPTH_WRITE);
			ctx->FlushResourceBarriers();

			ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
			ctx->SetScissorRect();
			ctx->SetViewport();
			ctx->ClearDepthStencil();
			ctx->SetStencilReference(0xFF);
			ctx->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
			ctx->SetRootSignature(gbufferPass_RootSignature);
			ctx->SetPipelineState(gbufferPass_PipelineState);

			for(GBuffer & gb : gbufferPass_Input) {
				ctx->SetConstants(0, *perFrameData);
				ctx->SetConstants(1, *boneData);
				ctx->SetConstants(2, *perObjectData);
				ctx->SetVertexBuffer(gb.vertexBuffer);
				ctx->SetIndexBuffer(gb.indexBuffer);
				ctx->DrawIndexed(gb.indexCount);
			}

			ctx->ResourceBarrier(gbufferPass_DepthStencil, ResourceState::DEPTH_WRITE, ResourceState::DEPTH_READ);
			ctx->FlushResourceBarriers();
		});
	}

	void CreateBoneVisibilityPass(Netcode::FrameGraphBuilderRef builder) {
		builder->CreateRenderPass("Bone visibility pass", [this](IResourceContext * ctx) ->void {
			ctx->Reads(gbufferPass_DepthStencil);
			ctx->Writes(0);
		},
			[this](IRenderContext * ctx) -> void {

			ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
			ctx->SetPrimitiveTopology(PrimitiveTopology::POINTLIST);
			ctx->SetRootSignature(boneVisibilityPass_RootSignature);
			ctx->SetPipelineState(boneVisibilityPass_PipelineState);

			for(GBuffer & gb : gbufferPass_Input) {
				ctx->SetConstants(0, *perFrameData);
				ctx->SetConstants(1, *boneData);
				ctx->SetConstants(2, *perObjectData);
				ctx->SetConstants(3, *boneVisibilityData);
				ctx->SetVertexBuffer(gb.vertexBuffer);
				ctx->SetIndexBuffer(gb.indexBuffer);
				ctx->DrawIndexed(gb.indexCount);
			}
		});
	}

	void CreateBackgroundPass(Netcode::FrameGraphBuilderRef builder) {
		builder->CreateRenderPass("Background pass", [this](IResourceContext * ctx) ->void {
			ctx->Reads(gbufferPass_DepthStencil);
			ctx->Writes(0);
		},
			[this](IRenderContext * ctx) -> void {

			ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
			ctx->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
			ctx->SetStencilReference(0);
			ctx->SetRootSignature(envmapPass_RootSignature);
			ctx->SetPipelineState(envmapPass_PipelineState);

			ctx->SetConstants(0, *perFrameData);
			ctx->SetShaderResources(1, cloudynoonView);

			ctx->SetVertexBuffer(fsQuad.vertexBuffer);
			ctx->Draw(fsQuad.vertexCount);
		});
	}


	void CreateColliderPass(Netcode::FrameGraphBuilderRef builder) {
		builder->CreateRenderPass("Collider pass", [this](IResourceContext * ctx) ->void {
			ctx->Reads(gbufferPass_DepthStencil);
			ctx->Writes(0);
		},
			[this](IRenderContext * ctx) -> void {

			ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
			ctx->SetPrimitiveTopology(PrimitiveTopology::LINELIST);
			ctx->SetRootSignature(colliderPass_RootSignature);
			ctx->SetPipelineState(colliderPass_PipelineState);

			uint32_t idx = 0;
			for(GBuffer & gb : colliderPass_Input) {
				ctx->SetConstants(0, colliderPass_DataInput.at(idx++));
				ctx->SetConstants(1, *perFrameData);
				ctx->SetConstants(2, *boneData);
				ctx->SetConstants(3, *perObjectData);
				ctx->SetVertexBuffer(gb.vertexBuffer);
				ctx->Draw(gb.vertexCount);
			}
		});
	}
public:

	PerFrameData * perFrameData;
	BoneData * boneData;
	BoneVisibilityData * boneVisibilityData;
	PerObjectData * perObjectData;

	std::vector<GBuffer> gbufferPass_Input;
	std::vector<GBuffer> colliderPass_Input;
	std::vector<ColliderData> colliderPass_DataInput;

	void CreatePermanentResources(Netcode::Module::IGraphicsModule * g) {
		CreateFSQuad(g);
		Create_GBufferPass_PermanentResources(g);
		Create_BackgroundPass_PermanentResources(g);
		Create_BoneVisibilityPass_PermanentResources(g);
		Create_ColliderPass_PermanentResource(g);
	}

	void CreateFrameGraph(Netcode::FrameGraphBuilderRef builder) {
		CreateGbufferPass(builder);
		CreateBoneVisibilityPass(builder);
		CreateBackgroundPass(builder);
		CreateColliderPass(builder);
	}

};

