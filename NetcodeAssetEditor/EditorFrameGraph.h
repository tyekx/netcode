#pragma once

#include <Netcode/Graphics/FrameGraph.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include <Netcode/Graphics/UploadBatch.h>
#include <Netcode/Modules.h>
#include <Netcode/FancyIterators.hpp>
#include <DirectXTex.h>
#include <NetcodeAssetLib/IntermediateModel.h>
#include <Netcode/HandleTypes.h>

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
public:
	Netcode::UInt2 backbufferSize;

	GBuffer fsQuad;

	Ref<Netcode::RootSignature> gbufferPass_RootSignature;
	Ref<Netcode::PipelineState> gbufferPass_PipelineState;

	Ref<Netcode::RootSignature> boneVisibilityPass_RootSignature;
	Ref<Netcode::PipelineState> boneVisibilityPass_PipelineState;

	Ref<Netcode::RootSignature> envmapPass_RootSignature;
	Ref<Netcode::PipelineState> envmapPass_PipelineState;

	Ref<Netcode::RootSignature> colliderPass_RootSignature;
	Ref<Netcode::PipelineState> colliderPass_PipelineState;

	Ref<Netcode::GpuResource> cloudynoonTexture;
	Ref<Netcode::ResourceViews> cloudynoonView;

	Ref<Netcode::GpuResource> gbufferPass_DepthStencil;
	Ref<Netcode::ResourceViews> gbufferPass_DepthStencilView;



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

		auto uploadBatch = g->resources->CreateUploadBatch();
		uploadBatch->Upload(fsQuad.vertexBuffer, vData, sizeof(vData));
		uploadBatch->Barrier(fsQuad.vertexBuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

		g->frame->SyncUpload(uploadBatch);
	}

	void Create_GBufferPass_SizeDependentResources() {
		gbufferPass_DepthStencil.reset();

		gbufferPass_DepthStencil = graphics->resources->CreateDepthStencil(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::DEPTH_READ);

		gbufferPass_DepthStencilView->CreateDSV(gbufferPass_DepthStencil.get());
	}

	void Create_GBufferPass_PipelineState() {
		auto inputLayoutBuilder = graphics->CreateInputLayoutBuilder();
		inputLayoutBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		inputLayoutBuilder->AddInputElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("BINORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT);
		inputLayoutBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT);
		auto inputLayout = inputLayoutBuilder->Build();

		auto shaderBuilder = graphics->CreateShaderBuilder();
		auto vs = shaderBuilder->LoadBytecode(L"Editor_GBufferPass_Vertex.cso");
		auto ps = shaderBuilder->LoadBytecode(L"Editor_GBufferPass_Pixel.cso");

		auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
		gbufferPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		Netcode::DepthStencilDesc depthStencilDesc;
		depthStencilDesc.depthEnable = true;

		auto psoBuilder = graphics->CreateGPipelineStateBuilder();
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetDepthStencilFormat(graphics->GetDepthStencilFormat());
		psoBuilder->SetNumRenderTargets(1);
		psoBuilder->SetRenderTargetFormat(0, graphics->GetBackbufferFormat());
		psoBuilder->SetDepthStencilState(depthStencilDesc);
		psoBuilder->SetRootSignature(gbufferPass_RootSignature);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType::TRIANGLE);
		gbufferPass_PipelineState = psoBuilder->Build();
	}

	void Create_GBufferPass_PermanentResources(Netcode::Module::IGraphicsModule * g) {
		Create_GBufferPass_PipelineState();
		gbufferPass_DepthStencilView = graphics->resources->CreateDepthStencilView();
		Create_GBufferPass_SizeDependentResources();
	}

	void Create_BoneVisibilityPass_PermanentResources(Netcode::Module::IGraphicsModule * g) {
		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		ilBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT);
		auto inputLayout = ilBuilder->Build();

		auto shaderBuilder = g->CreateShaderBuilder();
		auto vs = shaderBuilder->LoadBytecode(L"Editor_BoneVisibilityPass_Vertex.cso");
		auto ps = shaderBuilder->LoadBytecode(L"Editor_BoneVisibilityPass_Pixel.cso");

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
		Ref<Netcode::TextureBuilder> textureBuilder = g->CreateTextureBuilder();
		textureBuilder->LoadTextureCube(L"compiled/textures/envmaps/cloudynoon.dds");
		textureBuilder->SetStateAfterUpload(ResourceState::PIXEL_SHADER_RESOURCE);

		cloudynoonTexture = textureBuilder->Build();
		g->resources->SetDebugName(cloudynoonTexture, L"Cloudynoon TextureCube");

		cloudynoonView = g->resources->CreateShaderResourceViews(1);
		cloudynoonView->CreateSRV(0, cloudynoonTexture.get());

		auto shaderBuilder = g->CreateShaderBuilder();
		auto vs = shaderBuilder->LoadBytecode(L"envmapPass_Vertex.cso");
		auto ps = shaderBuilder->LoadBytecode(L"envmapPass_Pixel.cso");
		auto rootSigBuilder = g->CreateRootSignatureBuilder();

		envmapPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		auto inputLayout = ilBuilder->Build();

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

		auto psoBuilder = g->CreateGPipelineStateBuilder();
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
		auto inputLayout = ilBuilder->Build();

		auto shaderBuilder = g->CreateShaderBuilder();
		auto vs = shaderBuilder->LoadBytecode(L"Editor_ColliderPass_Vertex.cso");
		auto ps = shaderBuilder->LoadBytecode(L"Editor_ColliderPass_Pixel.cso");

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

	void CreateGbufferPass(Ptr<Netcode::FrameGraphBuilder> builder) {
		builder->CreateRenderPass("Gbuffer pass", [this](IResourceContext * ctx) ->void {
			ctx->Writes(static_cast<uintptr_t>(0));
			ctx->Writes(gbufferPass_DepthStencil.get());
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

			for(auto & [gb, mat] : Zip(gbufferPass_Input, gbufferPass_InputMaterials)) {

				if(mat->GetType() != Netcode::MaterialType::BRDF) {
					continue;
				}

				ctx->SetConstants(0, *perFrameData);
				ctx->SetConstants(1, *boneData);
				ctx->SetConstants(2, *perObjectData);
				ctx->SetConstants(3, *lightData);

				struct BrdfConstBuffer {
					Netcode::Float4 diffuseAlbedo;
					Netcode::Float3 fresnelR0;
					float roughness;
					Netcode::Float2 tiles;
					Netcode::Float2 tilesOffset;
					float displacementScale;
					float displacementBias;
					float textureSpaceHandednessFactor;
					uint32_t texturesFlags;
				};

				using ParamId = Netcode::MaterialParamId;

				BrdfConstBuffer buffer;
				buffer.diffuseAlbedo = mat->GetOptionalParameter<Netcode::Float4>(ParamId::DIFFUSE_ALBEDO, Netcode::Float4::Zero);
				buffer.fresnelR0 = mat->GetOptionalParameter<Netcode::Float3>(ParamId::FRESNEL_R0, Netcode::Float3{ 0.05f, 0.05f, 0.05f });
				buffer.roughness = mat->GetOptionalParameter<float>(ParamId::ROUGHNESS, 36.0f) / 256.0f;
				buffer.tiles = mat->GetOptionalParameter<Netcode::Float2>(ParamId::TEXTURE_TILES, Netcode::Float2::One);
				buffer.tilesOffset = mat->GetOptionalParameter<Netcode::Float2>(ParamId::TEXTURE_TILES_OFFSET, Netcode::Float2::Zero);
				buffer.displacementScale = mat->GetOptionalParameter<float>(ParamId::DISPLACEMENT_SCALE, 0.01f);
				buffer.displacementBias = mat->GetOptionalParameter<float>(ParamId::DISPLACEMENT_BIAS, 0.42f);
				buffer.textureSpaceHandednessFactor = 1.0f;
				buffer.texturesFlags = 0;

				for(uint32_t i = 0; i < 6; ++i) {
					auto res = mat->GetResource(i);
					if(res != nullptr) {
						buffer.texturesFlags |= (1 << i);
					}
				}

				ctx->SetConstants(4, buffer);
				ctx->SetShaderResources(5, mat->GetResourceView(0));
				ctx->SetShaderResources(6, cloudynoonView);

				ctx->SetVertexBuffer(gb.vertexBuffer);
				ctx->SetIndexBuffer(gb.indexBuffer);
				ctx->DrawIndexed(gb.indexCount);
			}

			ctx->ResourceBarrier(gbufferPass_DepthStencil, ResourceState::DEPTH_WRITE, ResourceState::DEPTH_READ);
			ctx->FlushResourceBarriers();
		});
	}

	void CreateBoneVisibilityPass(Ptr<Netcode::FrameGraphBuilder> builder) {
		builder->CreateRenderPass("Bone visibility pass", [this](IResourceContext * ctx) ->void {
			ctx->Reads(gbufferPass_DepthStencil.get());
			ctx->Writes(static_cast<uintptr_t>(0));
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

	void CreateBackgroundPass(Ptr<Netcode::FrameGraphBuilder> builder) {
		builder->CreateRenderPass("Background pass", [this](IResourceContext * ctx) ->void {
			ctx->Reads(gbufferPass_DepthStencil.get());
			ctx->Reads(static_cast<uintptr_t>(1));
			ctx->Writes(static_cast<uintptr_t>(0));
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

	void CreateDebugPass(Ptr<Netcode::FrameGraphBuilder> builder) {
		uint32_t numLines = 10;

		for(int32_t i = 0; i <= numLines; ++i) {
			float w = 100.0f;
			float z = -w / (2.0f) + ((static_cast<float>(i) / static_cast<float>(numLines)) * (w));
			float x0 = -w / 2.0f;
			float x1 = w / 2.0f;

			Netcode::Float3 p0{ x0, 0.0f, z };
			Netcode::Float3 p1{ x1, 0.0f, z };

			Netcode::Float3 p2{ z, 0.0f, x0 };
			Netcode::Float3 p3{ z, 0.0f, x1 };

			graphics->debug->DrawLine(p0, p1, Netcode::Float3{ 0.8f, 0.2f, 0.1f });
			graphics->debug->DrawLine(p2, p3, Netcode::Float3{ 0.1f, 0.2f, 0.8f });
		}

		builder->CreateRenderPass("Debug Pass", [this](IResourceContext * ctx) -> void {
			ctx->Writes(static_cast<uintptr_t>(1));
			graphics->debug->UploadResources(ctx);
		},
		[this](IRenderContext * ctx) -> void {
			ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencil);
			graphics->debug->Draw(ctx, perFrameData->ViewProj);
		});
	}

	void CreateColliderPass(Ptr<Netcode::FrameGraphBuilder> builder) {
		for(const Netcode::Intermediate::Collider & collider : colliderPass_Input) {
			using CT = Netcode::Asset::ColliderType;

			Netcode::Float4x4 transform = Netcode::Float4x4::Identity;

			if(boneData != nullptr) {
				if(collider.boneReference >= 0 && collider.boneReference < 128) {
					transform = Netcode::Matrix{ boneData->ToRootTransform[collider.boneReference] }.Transpose();
				}
			}

			switch(collider.type) {
				case CT::BOX:
					graphics->debug->DrawBox(collider.localRotation, collider.localPosition, collider.boxArgs, Netcode::Float3{ 0.7f, 0.7f, 0.7f }, transform);
					break;
				case CT::SPHERE:
					graphics->debug->DrawSphere(collider.localPosition, collider.sphereArgs, Netcode::Float3{ 0.7f, 0.7f, 0.7f }, transform);
					break;
				case CT::CAPSULE:
					graphics->debug->DrawCapsule(collider.localRotation, collider.localPosition, collider.capsuleArgs.y, collider.capsuleArgs.x, Netcode::Float3{ 0.7f, 0.7f, 0.7f }, transform);
					break;
				default: break;
			}
		}
	}
public:
	Netcode::Module::IGraphicsModule * graphics;


	void OnResized(int x, int y) {
		Netcode::UInt2 newSize{ static_cast<uint32_t>(x), static_cast<uint32_t>(y) };

		if(newSize.x != backbufferSize.x || newSize.y != backbufferSize.y) {
			backbufferSize = newSize;
			if(backbufferSize.x != 0 && backbufferSize.y != 0) {

				Create_GBufferPass_SizeDependentResources();
			}
		}
	}

	PerFrameData * perFrameData;
	BoneData * boneData;
	BoneVisibilityData * boneVisibilityData;
	PerObjectData * perObjectData;
	LightData * lightData;
	float cameraWorldDistance;

	std::vector<GBuffer> gbufferPass_Input;
	std::vector<Ref<Netcode::Material>> gbufferPass_InputMaterials;
	std::vector<Netcode::Intermediate::Collider> colliderPass_Input;

	void CreatePermanentResources(Netcode::Module::IGraphicsModule * g) {
		graphics = g;
		CreateFSQuad(g);
		Create_GBufferPass_PermanentResources(g);
		Create_BackgroundPass_PermanentResources(g);
		Create_BoneVisibilityPass_PermanentResources(g);
		Create_ColliderPass_PermanentResource(g);
	}

	void CreateFrameGraph(Ptr<Netcode::FrameGraphBuilder> builder) {
		CreateGbufferPass(builder);
		CreateBoneVisibilityPass(builder);
		CreateBackgroundPass(builder);
		CreateColliderPass(builder);
		CreateDebugPass(builder);
	}

};

