#pragma once

#include <Egg/FrameGraph.h>
#include <Egg/FancyIterators.hpp>
#include <Egg/ResourceDesc.h>

#include <Egg/Vertex.h>

using Egg::Graphics::ResourceDesc;
using Egg::Graphics::ResourceType;
using Egg::Graphics::ResourceState;
using Egg::Graphics::ResourceFlags;
using Egg::Graphics::PrimitiveTopology;

struct GBuffer {
	uint64_t vbuffer;
	uint64_t ibuffer;
};

class AppDefinedGraphicsEngine {
	// handles
	int skinningPass_FilledSize;


	RenderPass * skinningPass;
	RenderPass * depthPrePass;
	RenderPass * lightningPass;
	RenderPass * postProcessPass;

public:

	ScratchBuffer<GBuffer> skinningPass_Input;
	ScratchBuffer<GBuffer> lightningPass_Input;

private:

	ScratchBuffer<GBuffer> skinningPass_Output;
	ScratchBuffer<GBuffer> gbufferPass_Output;

	void CreatePermanentResources(IResourceContext * context) {
		skinningPass_FilledSize = context->CreateTypedBuffer(4, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::STREAM_OUT, ResourceFlags::ALLOW_UNORDERED_ACCESS);
	}

	void CreateSkinningPass(FrameGraphBuilder & frameGraphBuilder) {
		skinningPass = frameGraphBuilder.CreateRenderPass("SkinningPass",
			[&](IResourceContext * context) -> void {
				
				skinningPass_Output.Expect(skinningPass_Input.Size());

				for(const GBuffer & buffer : skinningPass_Input) {
					GBuffer createdBuffer = buffer;

					ResourceDesc vbufferDesc = context->QueryDesc(buffer.vbuffer);
					const uint64_t elementCount = vbufferDesc.width / vbufferDesc.strideInBytes;
					const uint32_t stride = sizeof(Egg::PNT_Vertex);
					const size_t newSize = elementCount * stride;

					createdBuffer.vbuffer = context->CreateVertexBuffer(newSize, stride, ResourceType::TRANSIENT_DEFAULT, ResourceState::STREAM_OUT);
					skinningPass_Output.Produced(createdBuffer);
				}

			},
			[&](IRenderContext * context) -> void {
				if(skinningPass_Input.Size() == 0) {
					return;
				}

				auto zipped = Zip(skinningPass_Input, skinningPass_Output);

				// set primitive topology to linelist
				context->SetPrimitiveTopology(PrimitiveTopology::POINTLIST);
				context->SetStreamOutputFilledSize(skinningPass_FilledSize);

				for(auto [input, output] : zipped) {
					context->ClearTypedUAV(skinningPass_FilledSize);
					context->SetConstantBuffer(0, 0);
					context->SetStreamOutput(output.vbuffer);
					context->Draw(input.vbuffer);
					context->ResourceBarrier(output.vbuffer, ResourceState::STREAM_OUT, ResourceState::VERTEX_AND_CONSTANT_BUFFER);
				}

				context->ResetStreamOutput();

			}
		);
	}

	void CreateGbufferPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("gbufferPass", [&](IResourceContext * context) -> void {

			// does not allow the render pass to be culled
			context->CpuOnlyRenderPass();

		},
		[&](IRenderContext * context) -> void {
			
			gbufferPass_Output.Expect(skinningPass_Output.Size() + lightningPass_Input.Size());
			gbufferPass_Output.Merge(skinningPass_Output);
			gbufferPass_Output.Merge(lightningPass_Input);

		});
	}

	void CreateLightningPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("lightningPass", [&](IResourceContext * context) -> void {



		},
		[&](IRenderContext * context) -> void {



		});
	}

	void CreatePostProcessPass(FrameGraphBuilder & frameGraphBuilder) {
		frameGraphBuilder.CreateRenderPass("postProcessPass", [&](IResourceContext * context) -> void {



		},
		[&](IRenderContext * context) -> void {



		});
	}

public:


	void CreateFrameGraph(FrameGraphBuilder & builder) {
		CreateSkinningPass(builder);
		CreateGbufferPass(builder);
		CreateLightningPass(builder);
		CreatePostProcessPass(builder);
	}

};
