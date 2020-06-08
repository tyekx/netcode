
#include "../../Modules.h"
#include "../../Vertex.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class DebugContext {
		std::vector<PC_Vertex> vertices;
		Netcode::GpuResourceRef uploadBuffer;

	public:
		void CreateResources(Module::IGraphicsModule * graphics) {
			Netcode::InputLayoutBuilderRef ilBuilder = graphics->CreateInputLayoutBuilder();
			ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
			ilBuilder->AddInputElement("COLOR", DXGI_FORMAT_R32G32B32_FLOAT);
			Netcode::InputLayoutRef il = ilBuilder->Build();

			Netcode::GPipelineStateBuilderRef gpsoBuilder = graphics->CreateGPipelineStateBuilder();

			uploadBuffer = graphics->resources->CreateConstantBuffer(65536 * sizeof(PC_Vertex));
			vertices.reserve(65536);
		}

		void DrawDebugPrimitives(IRenderContext * context) {

		}
	};

	using DX12DebugContext = Netcode::Graphics::DX12::DebugContext;
	using DX12DebugContextRef = std::shared_ptr<DX12DebugContext>;

}
