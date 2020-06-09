
#include "../../Modules.h"
#include "../../Vertex.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class DebugContext {
		Netcode::Float4x4 viewProj;
		size_t bufferSize;
		size_t numNoDepthVertices;
		size_t numDepthVertices;
		std::vector<PC_Vertex> vertices;

		Netcode::GpuResourceRef uploadBuffer;

		Netcode::PipelineStateRef depthPso;
		Netcode::PipelineStateRef noDepthPso;
		Netcode::RootSignatureRef rootSignature;

		inline void PushVertex(const PC_Vertex & vertex, bool depthEnabled) {
			// assert: bufferSize >= (numNoDepthVertices + numDepthVertices)
			if(depthEnabled) {
				vertices[numDepthVertices++] = vertex;
			} else {
				vertices[bufferSize - numNoDepthVertices++] = vertex;
			}
		}

	public:
		void CreateResources(Module::IGraphicsModule * graphics) {
			Netcode::InputLayoutBuilderRef ilBuilder = graphics->CreateInputLayoutBuilder();
			ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
			ilBuilder->AddInputElement("COLOR", DXGI_FORMAT_R32G32B32_FLOAT);
			Netcode::InputLayoutRef il = ilBuilder->Build();

			Netcode::DepthStencilDesc depthStencilDesc;
			Netcode::RasterizerDesc rasterizerDesc;
			rasterizerDesc.antialiasedLineEnable = true;

			Netcode::ShaderBuilderRef shaderBuilder = graphics->CreateShaderBuilder();
			Netcode::ShaderBytecodeRef vertexShader = shaderBuilder->LoadBytecode(L"Netcode_DebugPrimVS.hlsl");
			Netcode::ShaderBytecodeRef pixelShader = shaderBuilder->LoadBytecode(L"Netcode_DebugPrimPS.hlsl");

			Netcode::RootSignatureBuilderRef rootSigBuilder = graphics->CreateRootSignatureBuilder();
			rootSignature = rootSigBuilder->BuildFromShader(vertexShader);

			Netcode::GPipelineStateBuilderRef gpsoBuilder = graphics->CreateGPipelineStateBuilder();
			gpsoBuilder->SetNumRenderTargets(1);
			gpsoBuilder->SetRenderTargetFormat(0, graphics->GetBackbufferFormat());
			gpsoBuilder->SetDepthStencilFormat(graphics->GetDepthStencilFormat());
			gpsoBuilder->SetInputLayout(il);
			gpsoBuilder->SetPrimitiveTopologyType(PrimitiveTopologyType::LINE);
			gpsoBuilder->SetRasterizerState(rasterizerDesc);
			gpsoBuilder->SetDepthStencilState(depthStencilDesc);
			gpsoBuilder->SetVertexShader(vertexShader);
			gpsoBuilder->SetPixelShader(pixelShader);
			gpsoBuilder->SetRootSignature(rootSignature);
			depthPso = gpsoBuilder->Build();

			depthStencilDesc.depthEnable = false;

			gpsoBuilder->SetNumRenderTargets(1);
			gpsoBuilder->SetRenderTargetFormat(0, graphics->GetBackbufferFormat());
			gpsoBuilder->SetDepthStencilFormat(graphics->GetDepthStencilFormat());
			gpsoBuilder->SetInputLayout(il);
			gpsoBuilder->SetPrimitiveTopologyType(PrimitiveTopologyType::LINE);
			gpsoBuilder->SetRasterizerState(rasterizerDesc);
			gpsoBuilder->SetDepthStencilState(depthStencilDesc);
			gpsoBuilder->SetVertexShader(vertexShader);
			gpsoBuilder->SetPixelShader(pixelShader);
			gpsoBuilder->SetRootSignature(rootSignature);
			noDepthPso = gpsoBuilder->Build();

			uploadBuffer = graphics->resources->CreateVertexBuffer(65536 * sizeof(PC_Vertex),
																   sizeof(PC_Vertex),
																   ResourceType::PERMANENT_UPLOAD,
																   ResourceState::ANY_READ);

			bufferSize = 65536;
			vertices.resize(bufferSize);
		}

		void PrepareDrawing(IResourceContext * context, const Netcode::Float4x4 & vpMat) {
			viewProj = vpMat;

			if(numDepthVertices > 0) {
				context->CopyConstants(uploadBuffer, vertices.data(), numDepthVertices * sizeof(PC_Vertex), 0);
			}

			if(numNoDepthVertices > 0) {
				context->CopyConstants(uploadBuffer, vertices.data() + numDepthVertices, numNoDepthVertices * sizeof(PC_Vertex), numDepthVertices * sizeof(PC_Vertex));
			}
		}

		void DrawDebugPrimitives(IRenderContext * context) {
			if(numDepthVertices == 0 && numNoDepthVertices == 0) {
				return;
			}

			context->SetRootSignature(rootSignature);
			context->SetRootConstants(0, &viewProj, 16);
			context->SetVertexBuffer(uploadBuffer);

			if(numDepthVertices > 0) {
				context->SetPipelineState(depthPso);
				context->Draw(numDepthVertices);
			}

			if(numNoDepthVertices > 0) {
				context->SetPipelineState(noDepthPso);
				context->Draw(numNoDepthVertices, numDepthVertices);
			}
		}

		void DrawDebugPoint(const Netcode::Float3 & point, float extent, bool depthEnabled = true) {
			Netcode::PC_Vertex v0, v1, v2, v3, v4, v5;
			v0.color = v1.color = DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f };
			v2.color = v3.color = DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f };
			v4.color = v5.color = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f };

			v0.position = Netcode::Float3(point.x - extent, point.y, point.z);
			v1.position = Netcode::Float3(point.x + extent, point.y, point.z);

			v0.position = Netcode::Float3(point.x, point.y - extent, point.z);
			v1.position = Netcode::Float3(point.x, point.y + extent, point.z);

			v0.position = Netcode::Float3(point.x, point.y, point.z - extent);
			v1.position = Netcode::Float3(point.x, point.y, point.z + extent);

			PushVertex(v0, depthEnabled);
			PushVertex(v1, depthEnabled);
			PushVertex(v2, depthEnabled);
			PushVertex(v3, depthEnabled);
			PushVertex(v4, depthEnabled);
			PushVertex(v5, depthEnabled);
		}

		void DrawDebugLine(const Netcode::Float3 & p0, const Netcode::Float3 & p1, const Netcode::Float3 & color, bool depthEnabled = true) {
			Netcode::PC_Vertex vert0;
			vert0.position = p0;
			vert0.color = color;

			Netcode::PC_Vertex vert1;
			vert1.position = p1;
			vert1.color = color;

			PushVertex(vert0, depthEnabled);
			PushVertex(vert1, depthEnabled);
		}

		void DrawDebugVector(const Netcode::Float3 & startAt, const Netcode::Float3 & dir, float length, const Netcode::Float3 & color, bool depthEnabled = true) {
			Netcode::Vector3 st = startAt;
			Netcode::Vector3 d = dir;

			Netcode::PC_Vertex vert0;
			vert0.position = startAt;
			vert0.color = color;

			Netcode::PC_Vertex vert1;
			vert1.color = color;
			vert1.position = st + d * length;

			PushVertex(vert0, depthEnabled);
			PushVertex(vert1, depthEnabled);
		}

	};

	using DX12DebugContext = Netcode::Graphics::DX12::DebugContext;
	using DX12DebugContextRef = std::shared_ptr<DX12DebugContext>;

}
