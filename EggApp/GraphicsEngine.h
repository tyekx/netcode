#pragma once

#include <Egg/FrameGraph.h>
#include <Egg/FancyIterators.hpp>
#include <Egg/ResourceDesc.h>
#include <Egg/Vertex.h>
#include <DirectXPackedVector.h>
#include <Egg/EggMath.h>
#include "GameObject.h"

using Egg::Graphics::ResourceDesc;
using Egg::Graphics::ResourceType;
using Egg::Graphics::ResourceState;
using Egg::Graphics::ResourceFlags;
using Egg::Graphics::PrimitiveTopology;

struct RenderItem {
	GBuffer gbuffer;
	Material * material;
	PerObjectData * objectData;
	BoneData * boneData;

	RenderItem(const ShadedMesh & shadedMesh, PerObjectData * objectData, BoneData* boneData) :
		gbuffer{ shadedMesh.mesh->GetGBuffer() }, material{ shadedMesh.material.get() }, objectData{ objectData }, boneData{ boneData } {

	}
};

struct UIRenderItem {
	Egg::ResourceViewsRef texture;
	Egg::SpriteFontRef font;
	const wchar_t * text;
	DirectX::XMFLOAT4 fontColor;
	DirectX::XMFLOAT2 position;
	DirectX::XMUINT2 textureSize;
};

class GraphicsEngine {
	// handles
	uint64_t skinningPass_FilledSize;

	uint64_t gbufferPass_DepthBuffer;
	uint64_t gbufferPass_ColorRenderTarget;
	uint64_t gbufferPass_NormalsRenderTarget;

	uint64_t ssaoPass_BlurRenderTarget;
	uint64_t ssaoPass_OcclusionRenderTarget;
	uint64_t ssaoPass_RandomVectorTexture;

	GBuffer fsQuad;

	DXGI_FORMAT gbufferPass_DepthStencilFormat;

	DirectX::XMUINT2 backbufferSize;
	DirectX::XMUINT2 ssaoRenderTargetSize;

	RenderPass * skinningPass;
	RenderPass * depthPrePass;
	RenderPass * lightingPass;
	RenderPass * postProcessPass;

	Egg::Module::IGraphicsModule * graphics;

	Egg::ResourceViewsRef gbufferPass_RenderTargetViews;
	Egg::ResourceViewsRef gbufferPass_DepthStencilView;
	Egg::ResourceViewsRef lightingPass_ShaderResourceViews;

	Egg::RootSignatureRef skinningPass_RootSignature;
	Egg::PipelineStateRef skinningPass_PipelineState;

	Egg::RootSignatureRef gbufferPass_RootSignature;
	Egg::PipelineStateRef gbufferPass_PipelineState;

	Egg::RootSignatureRef lightingPass_RootSignature;
	Egg::PipelineStateRef lightingPass_PipelineState;

	Egg::RootSignatureRef ssaoOcclusionPass_RootSignature;
	Egg::PipelineStateRef ssaoOcclusionPass_PipelineState;

	Egg::RootSignatureRef ssaoBlurPass_RootSignature;
	Egg::PipelineStateRef ssaoBlurPass_PipelineState;

	Egg::SpriteFontRef testFont;

public:

	PerFrameData * perFrameData;
	SsaoData * ssaoData;

	ScratchBuffer<RenderItem> skinningPass_Input;
	ScratchBuffer<RenderItem> gbufferPass_Input;
	ScratchBuffer<UIRenderItem> uiPass_Input;




private:

	ScratchBuffer<RenderItem> skinningPass_Output;

	void CreateFSQuad(Egg::Module::IGraphicsModule * g) {
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

		Egg::Graphics::UploadBatch uploadBatch;
		uploadBatch.Upload(fsQuad.vertexBuffer, vData, sizeof(vData));
		uploadBatch.ResourceBarrier(fsQuad.vertexBuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

		g->frame->SyncUpload(uploadBatch);
	}

	void CreateSkinningPassPermanentResources(Egg::Module::IGraphicsModule * g) {
		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		ilBuilder->AddInputElement("WEIGHTS", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("BONEIDS", DXGI_FORMAT_R32_UINT);
		Egg::InputLayoutRef inputLayout = ilBuilder->Build();

		auto soBuilder = g->CreateStreamOutputBuilder();
		soBuilder->AddStreamOutputEntry("POSITION", 3, 0, 0, 0);
		soBuilder->AddStreamOutputEntry("NORMAL", 3, 0, 0, 0);
		soBuilder->AddStreamOutputEntry("TEXCOORD", 2, 0, 0, 0);
		soBuilder->AddStride(32);
		Egg::StreamOutputRef streamOutput = soBuilder->Build();

		auto shaderBuilder = g->CreateShaderBuilder();
		Egg::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"skinningPass_Vertex.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		skinningPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(skinningPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetStreamOutput(streamOutput);
		psoBuilder->SetNumRenderTargets(0);
		psoBuilder->SetPrimitiveTopologyType(Egg::Graphics::PrimitiveTopologyType::POINT);
		skinningPass_PipelineState = psoBuilder->Build();

		skinningPass_FilledSize = g->resources->CreateTypedBuffer(65536, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::STREAM_OUT, ResourceFlags::ALLOW_UNORDERED_ACCESS);
	}

	void CreateGbufferPassPermanentResources(Egg::Module::IGraphicsModule * g) {
		gbufferPass_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		auto shaderBuilder = g->CreateShaderBuilder();
		Egg::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"gbufferPass_Vertex.cso");
		Egg::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"gbufferPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		gbufferPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		Egg::InputLayoutRef inputLayout = ilBuilder->Build();

		Egg::DepthStencilDesc depthStencilDesc;
		depthStencilDesc.backFace.stencilDepthFailOp = Egg::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilFailOp = Egg::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilPassOp = Egg::StencilOp::KEEP;
		depthStencilDesc.backFace.stencilFunc = Egg::ComparisonFunc::NEVER;

		depthStencilDesc.frontFace.stencilDepthFailOp = Egg::StencilOp::KEEP;
		depthStencilDesc.frontFace.stencilFailOp = Egg::StencilOp::KEEP;
		depthStencilDesc.frontFace.stencilPassOp = Egg::StencilOp::REPLACE;
		depthStencilDesc.frontFace.stencilFunc = Egg::ComparisonFunc::ALWAYS;

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
		psoBuilder->SetPrimitiveTopologyType(Egg::Graphics::PrimitiveTopologyType::TRIANGLE);
		gbufferPass_PipelineState = psoBuilder->Build();

		gbufferPass_RenderTargetViews = graphics->resources->CreateRenderTargetViews(2);
		gbufferPass_DepthStencilView = graphics->resources->CreateDepthStencilView();

		CreateGbufferPassSizeDependentResources();
	}

	void CreateGbufferPassSizeDependentResources() {
		if(gbufferPass_ColorRenderTarget != 0) {
			graphics->resources->ReleaseResource(gbufferPass_ColorRenderTarget);
			gbufferPass_ColorRenderTarget = 0;
		} 

		if(gbufferPass_DepthBuffer != 0) {
			graphics->resources->ReleaseResource(gbufferPass_DepthBuffer);
			gbufferPass_DepthBuffer = 0;
		}

		if(gbufferPass_NormalsRenderTarget != 0) {
			graphics->resources->ReleaseResource(gbufferPass_NormalsRenderTarget);
			gbufferPass_NormalsRenderTarget = 0;
		}

		gbufferPass_ColorRenderTarget = graphics->resources->CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
		gbufferPass_NormalsRenderTarget = graphics->resources->CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
		gbufferPass_DepthBuffer = graphics->resources->CreateDepthStencil(gbufferPass_DepthStencilFormat, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ);
	
		graphics->resources->SetDebugName(gbufferPass_ColorRenderTarget, L"GBuffer:Color");
		graphics->resources->SetDebugName(gbufferPass_NormalsRenderTarget, L"GBuffer:Normals");
		graphics->resources->SetDebugName(gbufferPass_DepthBuffer, L"GBuffer:Depth");

		gbufferPass_RenderTargetViews->CreateRTV(0, gbufferPass_ColorRenderTarget);
		gbufferPass_RenderTargetViews->CreateRTV(1, gbufferPass_NormalsRenderTarget);
		gbufferPass_DepthStencilView->CreateDSV(gbufferPass_DepthBuffer);
	}

	void CreateSSAOBlurPassPermanentResources(Egg::Module::IGraphicsModule * g) {
		auto shaderBuilder = g->CreateShaderBuilder();
		Egg::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"ssaoPass_Vertex.cso");
		Egg::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"ssaoBlurPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		ssaoBlurPass_RootSignature = rootSigBuilder->BuildFromShader(ps);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

		Egg::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = false;
		depthDesc.stencilEnable = false;

		Egg::InputLayoutRef inputLayout = ilBuilder->Build();
		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(ssaoBlurPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R32_FLOAT });
		psoBuilder->SetPrimitiveTopologyType(Egg::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);
		ssaoBlurPass_PipelineState = psoBuilder->Build();
	}

	void CreateSSAOOcclusionPassPermanentResources(Egg::Module::IGraphicsModule * g) {
		auto shaderBuilder = g->CreateShaderBuilder();
		Egg::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"ssaoPass_Vertex.cso");
		Egg::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"ssaoOcclusionPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		ssaoOcclusionPass_RootSignature = rootSigBuilder->BuildFromShader(ps);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

		Egg::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = false;
		depthDesc.stencilEnable = false;

		Egg::InputLayoutRef inputLayout = ilBuilder->Build();
		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(ssaoOcclusionPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R32_FLOAT });
		psoBuilder->SetPrimitiveTopologyType(Egg::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);
		ssaoOcclusionPass_PipelineState = psoBuilder->Build();

		ssaoPass_RandomVectorTexture = g->resources->CreateTexture2D(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);

		std::unique_ptr<DirectX::PackedVector::XMCOLOR[]> colors = std::make_unique<DirectX::PackedVector::XMCOLOR[]>(256 * 256);
		for(int i = 0; i < 256; ++i)
		{
			for(int j = 0; j < 256; ++j)
			{
				// Random vector in [0,1].  We will decompress in shader to [-1,1].
				DirectX::XMFLOAT3 v(RandomFloat(), RandomFloat(), RandomFloat());

				colors[i * 256 + j] = DirectX::PackedVector::XMCOLOR(v.x, v.y, v.z, 0.0f);
			}
		}

		Egg::Graphics::UploadBatch upload;
		upload.Upload(ssaoPass_RandomVectorTexture, colors.get(), sizeof(colors));
		upload.ResourceBarrier(ssaoPass_RandomVectorTexture, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
		g->frame->SyncUpload(upload);

		CreateSSAOOcclusionPassSizeDependentResources();
	}

	void CreateSSAOOcclusionPassSizeDependentResources() {
		if(ssaoPass_OcclusionRenderTarget != 0) {
			graphics->resources->ReleaseResource(ssaoPass_OcclusionRenderTarget);
			ssaoPass_OcclusionRenderTarget = 0;
		}

		if(ssaoPass_BlurRenderTarget != 0) {
			graphics->resources->ReleaseResource(ssaoPass_BlurRenderTarget);
			ssaoPass_BlurRenderTarget = 0;
		}

		ssaoRenderTargetSize = DirectX::XMUINT2 { backbufferSize.x / 2, backbufferSize.y / 2 };

		ssaoRenderTargetSize.x = (ssaoRenderTargetSize.x == 0) ? 1 : ssaoRenderTargetSize.x;
		ssaoRenderTargetSize.y = (ssaoRenderTargetSize.y == 0) ? 1 : ssaoRenderTargetSize.y;

		ssaoPass_OcclusionRenderTarget = graphics->resources->CreateRenderTarget(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y, DXGI_FORMAT_R32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
		ssaoPass_BlurRenderTarget = graphics->resources->CreateRenderTarget(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y, DXGI_FORMAT_R32_FLOAT, ResourceType::PERMANENT_DEFAULT, ResourceState::PIXEL_SHADER_RESOURCE);
	}

	void CreateLightingPassPermanentResources(Egg::Module::IGraphicsModule * g) {
		auto shaderBuilder = g->CreateShaderBuilder();
		Egg::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"lightingPass_Vertex.cso");
		Egg::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"lightingPass_Pixel.cso");

		auto rootSigBuilder = g->CreateRootSignatureBuilder();
		lightingPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

		Egg::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = false;
		depthDesc.stencilEnable = true;
		depthDesc.frontFace.stencilDepthFailOp = Egg::StencilOp::KEEP;
		depthDesc.frontFace.stencilFailOp = Egg::StencilOp::KEEP;
		depthDesc.frontFace.stencilPassOp = Egg::StencilOp::KEEP;
		depthDesc.frontFace.stencilFunc = Egg::ComparisonFunc::EQUAL;
		depthDesc.stencilReadMask = 0xFF;
		depthDesc.stencilWriteMask = 0x00;
		depthDesc.depthWriteMaskZero = true;
		depthDesc.backFace = depthDesc.frontFace;
		depthDesc.backFace.stencilFunc = Egg::ComparisonFunc::NEVER;

		Egg::InputLayoutRef inputLayout = ilBuilder->Build();
		auto psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetRootSignature(lightingPass_RootSignature);
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetPrimitiveTopologyType(Egg::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);
		lightingPass_PipelineState = psoBuilder->Build();

		lightingPass_ShaderResourceViews = graphics->resources->CreateShaderResourceViews(3);
		CreateLightingPassResourceViews();
	}

	void CreateLightingPassResourceViews() {
		lightingPass_ShaderResourceViews->CreateSRV(0, gbufferPass_ColorRenderTarget);
		lightingPass_ShaderResourceViews->CreateSRV(1, gbufferPass_NormalsRenderTarget);
		lightingPass_ShaderResourceViews->CreateSRV(2, gbufferPass_DepthBuffer);
	}

	void CreateSkinningPass(FrameGraphBuilder & frameGraphBuilder) {
		skinningPass = frameGraphBuilder.CreateRenderPass("Skinning",
			[&](IResourceContext * context) -> void {
				
				skinningPass_Output.Expect(skinningPass_Input.Size());

				for(const RenderItem & item: skinningPass_Input) {
					RenderItem createdItem = item;

					ResourceDesc vbufferDesc = context->QueryDesc(item.gbuffer.vertexBuffer);
					const uint64_t elementCount = vbufferDesc.width / vbufferDesc.strideInBytes;
					const uint32_t stride = sizeof(Egg::PNT_Vertex);
					const size_t newSize = elementCount * stride;

					createdItem.gbuffer.vertexBuffer = context->CreateVertexBuffer(newSize, stride, ResourceType::TRANSIENT_DEFAULT, ResourceState::STREAM_OUT);
					skinningPass_Output.Produced(createdItem);
				}

			},
			[&](IRenderContext * context) -> void {
				if(skinningPass_Input.Size() == 0) {
					return;
				}

				auto zipped = Zip(skinningPass_Input, skinningPass_Output);

				context->SetRootSignature(skinningPass_RootSignature);
				context->SetPipelineState(skinningPass_PipelineState);
				context->SetPrimitiveTopology(PrimitiveTopology::POINTLIST);

				context->ResourceBarrier(skinningPass_FilledSize, ResourceState::STREAM_OUT, ResourceState::UNORDERED_ACCESS);
				context->FlushResourceBarriers();

				context->ClearUnorderedAccessViewUint(skinningPass_FilledSize, DirectX::XMUINT4{ 0, 0, 0, 0 });

				context->ResourceBarrier(skinningPass_FilledSize, ResourceState::UNORDERED_ACCESS, ResourceState::STREAM_OUT);
				context->FlushResourceBarriers();

				uint64_t offset = 0;

				void * associatedPtr = nullptr;

				for(auto [input, output] : zipped) {
					if(offset > 65532) {
						Log::Warn("StreamOutputFilledSize overflow");
					}
					context->SetStreamOutputFilledSize(skinningPass_FilledSize, offset);
					offset += 4;
					if(associatedPtr != input.boneData) {
						context->SetConstants(0, *input.boneData);
						associatedPtr = input.boneData;
					}
					context->SetStreamOutput(output.gbuffer.vertexBuffer);
					context->SetVertexBuffer(input.gbuffer.vertexBuffer);
					context->Draw(input.gbuffer.vertexCount);
					context->ResourceBarrier(output.gbuffer.vertexBuffer, ResourceState::STREAM_OUT, ResourceState::VERTEX_AND_CONSTANT_BUFFER);
				}
			}
		);
	}

	void CreateGbufferPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("Gbuffer", [&](IResourceContext * context) -> void {


		},
		[&](IRenderContext * context) -> void {

			gbufferPass_Input.Expect(skinningPass_Output.Size() + gbufferPass_Input.Size());
			gbufferPass_Input.Merge(skinningPass_Output);

			bool isBound = false;
			void * objectData = nullptr;

			context->SetRootSignature(gbufferPass_RootSignature);
			context->SetPipelineState(gbufferPass_PipelineState);

			context->ResourceBarrier(gbufferPass_ColorRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
			context->ResourceBarrier(gbufferPass_NormalsRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
			context->ResourceBarrier(gbufferPass_DepthBuffer, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ, ResourceState::DEPTH_WRITE);
			context->FlushResourceBarriers();

			context->SetRenderTargets(gbufferPass_RenderTargetViews, gbufferPass_DepthStencilView);
			context->SetViewport();
			context->SetScissorRect();
			context->ClearRenderTarget(0);
			context->ClearRenderTarget(1);
			context->ClearDepthStencil();
			context->SetStencilReference(0xFF);
			context->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

			for(RenderItem & item : gbufferPass_Input) {
				item.material->Apply(context);

				if(objectData != item.objectData) {
					context->SetConstants(1, *item.objectData);
					objectData = item.objectData;
				}

				if(!isBound) {
					context->SetConstants(2, *perFrameData);
					isBound = true;
				}

				context->SetVertexBuffer(item.gbuffer.vertexBuffer);
				context->SetIndexBuffer(item.gbuffer.indexBuffer);
				context->DrawIndexed(item.gbuffer.indexCount);
			}

			context->ResourceBarrier(gbufferPass_ColorRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
			context->ResourceBarrier(gbufferPass_NormalsRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
			context->ResourceBarrier(gbufferPass_DepthBuffer, ResourceState::DEPTH_WRITE, ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::DEPTH_READ );
			context->FlushResourceBarriers();
		});
	}

	void CreateSSAOOcclusionPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("SSAO Occlusion", [&](IResourceContext * context) -> void {



		},
		[&](IRenderContext * context) -> void {
			context->SetRootSignature(ssaoOcclusionPass_RootSignature);
			context->SetPipelineState(ssaoOcclusionPass_PipelineState);

			context->SetViewport(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y);
			context->ResourceBarrier(ssaoPass_OcclusionRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
			context->FlushResourceBarriers();
			context->SetRenderTargets(ssaoPass_OcclusionRenderTarget, 0);
			context->ClearRenderTarget(0);

			context->SetConstants(0, *perFrameData);
			context->SetConstants(1, *ssaoData);
			context->SetShaderResources(2, { gbufferPass_NormalsRenderTarget, gbufferPass_DepthBuffer, ssaoPass_RandomVectorTexture });

			context->SetVertexBuffer(fsQuad.vertexBuffer);
			context->Draw(fsQuad.vertexCount);

			context->SetRenderTargets(0, 0);
			context->ResourceBarrier(ssaoPass_OcclusionRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
			context->FlushResourceBarriers();
		});
	}

	void CreateSSAOBlurPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("SSAO Blur", [&](IResourceContext * context) -> void {



		},
		[&](IRenderContext * context) -> void {

			context->ResourceBarrier(ssaoPass_BlurRenderTarget, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET);
			context->FlushResourceBarriers();
			context->SetRootSignature(ssaoBlurPass_RootSignature);
			context->SetPipelineState(ssaoBlurPass_PipelineState);
			context->SetViewport(ssaoRenderTargetSize.x, ssaoRenderTargetSize.y);

			context->SetRenderTargets(ssaoPass_BlurRenderTarget, 0);
			context->ClearRenderTarget(0);

			//context->SetConstants();

			context->SetShaderResources(0, { ssaoPass_OcclusionRenderTarget, gbufferPass_DepthBuffer });
			context->SetVertexBuffer(fsQuad.vertexBuffer);
			context->Draw(fsQuad.vertexCount);

			context->ResourceBarrier(ssaoPass_BlurRenderTarget, ResourceState::RENDER_TARGET, ResourceState::PIXEL_SHADER_RESOURCE);
			context->FlushResourceBarriers();
		});
	}

	void CreateLightingPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("Lighting", [&](IResourceContext * context) -> void {



		}, [&](IRenderContext * context) -> void {
			context->SetRootSignature(lightingPass_RootSignature);
			context->SetPipelineState(lightingPass_PipelineState);
			context->SetViewport();
			context->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
			context->SetStencilReference(255);
			context->SetConstants(0, *perFrameData);
			context->SetShaderResources(1, lightingPass_ShaderResourceViews);

			// @TODO: set light data

			context->SetVertexBuffer(fsQuad.vertexBuffer);
			context->Draw(fsQuad.vertexCount);
			
		});
	}

	void CreateUIPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("UI",
			[&](IResourceContext * context) -> void { },
			[&](IRenderContext * context) -> void {
			context->BeginSpriteRendering(uiPass_viewProjInv);
			for(const UIRenderItem & i : uiPass_Input) {
				if(i.texture != nullptr) {
					context->DrawSprite(i.texture, i.textureSize, i.position);
				}
				if(i.font != nullptr) {
					context->DrawString(i.font, i.text, i.position, i.fontColor );
				}
			}
			context->EndSpriteRendering();
		});
	}

	void CreatePostProcessPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("postProcessPass", [&](IResourceContext * context) -> void {



		},
		[&](IRenderContext * context) -> void {

			//context->SetRenderTargets(0, 0);

		});
	}

	uint64_t cloudynoonTexture;
	Egg::ResourceViewsRef cloudynoonView;

public:

	DirectX::XMFLOAT4X4 uiPass_viewProjInv;

	void CreateBackgroundPassPermanentResources(Egg::Module::IGraphicsModule * g) {
		Egg::TextureBuilderRef textureBuilder = graphics->CreateTextureBuilder();
		textureBuilder->LoadTextureCube(L"cloudynoon.dds");
		Egg::TextureRef cloudynoon = textureBuilder->Build();

		ASSERT(cloudynoon->GetImageCount() == 6, "bad image count");

		const Egg::Image* img = cloudynoon->GetImage(0, 0, 0);

		cloudynoonTexture = g->resources->CreateTextureCube(img->width, img->height, img->format, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);

		g->resources->SetDebugName(cloudynoonTexture, L"Cloudynoon TextureCube");

		Egg::Graphics::UploadBatch batch;
		batch.Upload(cloudynoonTexture, cloudynoon);
		batch.ResourceBarrier(cloudynoonTexture, ResourceState::COPY_DEST, ResourceState::PIXEL_SHADER_RESOURCE);
		g->frame->SyncUpload(batch);

		cloudynoonView = g->resources->CreateShaderResourceViews(1);
		cloudynoonView->CreateSRV(0, cloudynoonTexture);

		Egg::ShaderBuilderRef shaderBuilder = g->CreateShaderBuilder();
		Egg::ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"envmapPass_Vertex.cso");
		Egg::ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"envmapPass_Pixel.cso");

		Egg::RootSignatureBuilderRef rootSigBuilder = g->CreateRootSignatureBuilder();
		envmapPass_RootSignature = rootSigBuilder->BuildFromShader(vs);

		auto ilBuilder = g->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		Egg::InputLayoutRef inputLayout = ilBuilder->Build();


		Egg::DepthStencilDesc depthDesc;
		depthDesc.depthEnable = false;
		depthDesc.stencilEnable = true;
		depthDesc.frontFace.stencilDepthFailOp = Egg::StencilOp::KEEP;
		depthDesc.frontFace.stencilFailOp = Egg::StencilOp::KEEP;
		depthDesc.frontFace.stencilPassOp = Egg::StencilOp::KEEP;
		depthDesc.frontFace.stencilFunc = Egg::ComparisonFunc::EQUAL;
		depthDesc.stencilReadMask = 0xFF;
		depthDesc.stencilWriteMask = 0x00;
		depthDesc.depthWriteMaskZero = true;
		depthDesc.backFace = depthDesc.frontFace;
		depthDesc.backFace.stencilFunc = Egg::ComparisonFunc::NEVER;

		Egg::GPipelineStateBuilderRef psoBuilder = g->CreateGPipelineStateBuilder();
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilFormat(gbufferPass_DepthStencilFormat);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetRootSignature(envmapPass_RootSignature);
		psoBuilder->SetPrimitiveTopologyType(Egg::Graphics::PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetDepthStencilState(depthDesc);

		envmapPass_PipelineState = psoBuilder->Build();
	}

	Egg::RootSignatureRef envmapPass_RootSignature;
	Egg::PipelineStateRef envmapPass_PipelineState;

	void CreateBackgroundPass(FrameGraphBuilder & builder) {
		builder.CreateRenderPass("Background", [](IResourceContext * ctx) ->void { },
			[this](IRenderContext * ctx) -> void {

			ctx->SetRenderTargets(nullptr, gbufferPass_DepthStencilView);
			ctx->SetStencilReference(0);

			ctx->SetRootSignature(envmapPass_RootSignature);
			ctx->SetPipelineState(envmapPass_PipelineState);

			ctx->SetConstants(0, *perFrameData);
			ctx->SetShaderResources(1, cloudynoonView);

			ctx->SetVertexBuffer(fsQuad.vertexBuffer);
			ctx->Draw(fsQuad.vertexCount);
		});
	}

	void Reset() {
		skinningPass_Input.Clear();
		gbufferPass_Input.Clear();
		skinningPass_Output.Clear();
		uiPass_Input.Clear();
	}

	void OnResize(int x, int y) {
		DirectX::XMUINT2 newSize{ static_cast<uint32_t>(x), static_cast<uint32_t>(y) };

		if(newSize.x != backbufferSize.x || newSize.y != backbufferSize.y) {
			backbufferSize = newSize;
			if(backbufferSize.x != 0 && backbufferSize.y != 0) {
				CreateGbufferPassSizeDependentResources();
				CreateLightingPassResourceViews();
				//CreateSSAOOcclusionPassSizeDependentResources();
			}
		}
	}

	void CreatePermanentResources(Egg::Module::IGraphicsModule * g) {
		graphics = g;
		backbufferSize = g->GetBackbufferSize();
		CreateSkinningPassPermanentResources(g);
		CreateGbufferPassPermanentResources(g);
		CreateLightingPassPermanentResources(g);
		CreateBackgroundPassPermanentResources(g);
		//CreateSSAOBlurPassPermanentResources(g);
	  	//CreateSSAOOcclusionPassPermanentResources(g);
		CreateFSQuad(g);
	}

	void CreateFrameGraph(FrameGraphBuilder & builder) {
		CreateSkinningPass(builder);
		CreateGbufferPass(builder);
		//CreateSSAOOcclusionPass(builder);
		//CreateSSAOBlurPass(builder);
		CreateLightingPass(builder);
		CreateBackgroundPass(builder);
		CreateUIPass(builder);
		//CreatePostProcessPass(builder);
	}

};
