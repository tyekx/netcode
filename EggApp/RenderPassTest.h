#pragma once

#include <egg/RenderPass.h>

class GraphicsPipeline {
	// handles
	int skinningPass_FilledSize;

private: // render passes without user defined input
	RenderPass * compositionPass;

public: // render passes with user defined input
	RenderPass * skinningPass;
	RenderPass * phongPass;

private:

	void CreateSkinningPass(PipelineBuilder & pipelineBuilder) {
		skinningPass = pipelineBuilder.CreateRenderPass();
		skinningPass->setup = [&](RenderGraphBuilder & builder) -> void {
			builder.SetPipelineState(0);
			builder.CreateResource(256, ResourceState::UNORDERED_ACCESS, ResourceFlags::ALLOW_UNORDERED_ACCESS);
		};

		skinningPass->render = [=](RenderContext & ctx, RenderData & data) -> void {
			ctx.SetPrimitiveTopology();
			ctx.ClearUAV(skinningPass_FilledSize);
			ctx.SetStreamOutput();

			while(data.HasUserInput()) {
				int userInput = data.NextUserInput();
				int skinningPass_StreamOutput = ctx.CreateResource(120000, ResourceState::STREAM_OUT);

				ctx.ClearUAV(skinningPass_FilledSize);



				data.SetOutput(skinningPass_StreamOutput);
			}

		};

	}

public:
	void CreateStates(PipelineBuilder & pipelineBuilder) {

	}

	void Setup(PipelineBuilder & pipelineBuilder) {

	}



};
