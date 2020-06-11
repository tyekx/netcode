#include "../../BasicGeometry.h"

#include "DX12DebugContext.h"

namespace Netcode::Graphics::DX12 {
	void DebugContext::CreateResources(Module::IGraphicsModule * graphics) {
		Netcode::InputLayoutBuilderRef ilBuilder = graphics->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("COLOR", DXGI_FORMAT_R32G32B32_FLOAT);
		Netcode::InputLayoutRef il = ilBuilder->Build();

		Netcode::DepthStencilDesc depthStencilDesc;
		Netcode::RasterizerDesc rasterizerDesc;
		rasterizerDesc.antialiasedLineEnable = true;

		Netcode::ShaderBuilderRef shaderBuilder = graphics->CreateShaderBuilder();
		Netcode::ShaderBytecodeRef vertexShader = shaderBuilder->LoadBytecode(L"Netcode_DebugPrimVS.cso");
		Netcode::ShaderBytecodeRef pixelShader = shaderBuilder->LoadBytecode(L"Netcode_DebugPrimPS.cso");

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
	void DebugContext::UploadResources(IResourceContext * context) {
		if(numDepthVertices > 0) {
			context->CopyConstants(uploadBuffer, vertices.data(), numDepthVertices * sizeof(PC_Vertex), 0);
		}

		if(numNoDepthVertices > 0) {
			context->CopyConstants(uploadBuffer, vertices.data() + numDepthVertices, numNoDepthVertices * sizeof(PC_Vertex), numDepthVertices * sizeof(PC_Vertex));
		}
	}
	void DebugContext::Draw(IRenderContext * context, const Netcode::Float4x4 & viewProjMatrix) {
		if(numDepthVertices == 0 && numNoDepthVertices == 0) {
			return;
		}

		viewProj = viewProjMatrix;

		context->SetRenderTargets(nullptr, nullptr);
		context->SetPrimitiveTopology(PrimitiveTopology::LINELIST);
		context->SetScissorRect();
		context->SetViewport();
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

		numDepthVertices = 0;
		numNoDepthVertices = 0;
	}

	void DebugContext::DrawPoint(const Netcode::Float3 & point, float extent) {
		DrawPoint(point, extent, true);
	}

	void DebugContext::DrawPoint(const Netcode::Float3 & point, float extent, bool depthEnabled) {
		DrawLine(
			Netcode::Float3(point.x - extent, point.y, point.z),
			Netcode::Float3(point.x + extent, point.y, point.z),
			Netcode::Float3(1.0f, 0.0f, 0.0f),
			depthEnabled
		);

		DrawLine(
			Netcode::Float3(point.x, point.y - extent, point.z),
		    Netcode::Float3(point.x, point.y + extent, point.z),
			Netcode::Float3(0.0f, 1.0f, 0.0f),
			depthEnabled
		);

		DrawLine(
			Netcode::Float3(point.x, point.y, point.z - extent),
			Netcode::Float3(point.x, point.y, point.z + extent),
			Netcode::Float3(0.0f, 0.0f, 1.0f),
			depthEnabled
		);
	}

	void DebugContext::DrawLine(const Netcode::Float3 & worldPosStart, const Netcode::Float3 & worldPosEnd) {
		DrawLine(worldPosStart, worldPosEnd, true);
	}

	void DebugContext::DrawLine(const Netcode::Float3 & worldPosStart, const Netcode::Float3 & worldPosEnd, bool depthEnabled) {
		DrawLine(worldPosStart, worldPosEnd, Netcode::Float3{ 0.7f, 0.7f, 0.7f }, depthEnabled);
	}

	void DebugContext::DrawLine(const Netcode::Float3 & worldPosStart, const Netcode::Float3 & worldPosEnd, const Netcode::Float3 & color) {
		DrawLine(worldPosStart, worldPosEnd, color, true);
	}

	void DebugContext::DrawLine(const Netcode::Float3 & worldPosStart, const Netcode::Float3 & worldPosEnd, const Netcode::Float3 & color, bool depthEnabled) {
		Netcode::PC_Vertex vert0;
		vert0.position = worldPosStart;
		vert0.color = color;

		Netcode::PC_Vertex vert1;
		vert1.position = worldPosEnd;
		vert1.color = color;

		PushVertex(vert0, depthEnabled);
		PushVertex(vert1, depthEnabled);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius)
	{
		DrawSphere(worldPosOrigin, radius, Float3{ 0.7f, 0.7f, 0.7f }, true);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, bool depthEnabled)
	{
		DrawSphere(worldPosOrigin, radius, Float3{ 0.7f, 0.7f, 0.7f }, depthEnabled);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color)
	{
		DrawSphere(worldPosOrigin, radius, color, true);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, bool depthEnabled)
	{
		constexpr uint32_t numSlices = 8;

		uint32_t numVertices = BasicGeometry::GetSphereWireframeSize(numSlices);

		Netcode::PC_Vertex * vData = GetBufferForVertices(numVertices, depthEnabled);

		BasicGeometry::CreateSphereWireframe(vData, sizeof(Netcode::PC_Vertex), radius, numSlices, 0);

		for(uint32_t i = 0; i < numVertices; ++i) {
			vData[i].position = worldPosOrigin + vData[i].position;
			vData[i].color = color;
		}
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents)
	{
		DrawBoundingBox(worldPosOrigin, halfExtents, Float3{ 0.7f, 0.7f, 0.7f }, true);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled)
	{
		DrawBoundingBox(worldPosOrigin, halfExtents, Float3{ 0.7f, 0.7f, 0.7f }, depthEnabled);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color)
	{
		DrawBoundingBox(worldPosOrigin, halfExtents, color, true);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled)
	{
		DrawBox(Quaternion{}, worldPosOrigin, halfExtents, color, depthEnabled);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents)
	{
		DrawBox(orientation, worldPosOrigin, halfExtents, Float3{ 0.7f, 0.7f, 0.7f }, true);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled)
	{
		DrawBox(orientation, worldPosOrigin, halfExtents, Float3{ 0.7f, 0.7f, 0.7f }, depthEnabled);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color)
	{
		DrawBox(orientation, worldPosOrigin, halfExtents, color, true);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled)
	{
		const Vector3 negExt = -halfExtents;

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
			     worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation), color, depthEnabled);

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);

		DrawLine(worldPosOrigin + halfExtents.Permute<0, 1, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<0, 1, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<0, 4, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<0, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 1, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 1, 5>(negExt).Rotate(orientation), color, depthEnabled);
		DrawLine(worldPosOrigin + halfExtents.Permute<3, 4, 2>(negExt).Rotate(orientation),
				 worldPosOrigin + halfExtents.Permute<3, 4, 5>(negExt).Rotate(orientation), color, depthEnabled);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight)
	{
		DrawCapsule(rotation, position, radius, halfHeight, Float3{ 0.7f, 0.7f, 0.7f }, true);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, bool depthEnabled)
	{
		DrawCapsule(rotation, position, radius, halfHeight, Float3{ 0.7f, 0.7f, 0.7f }, depthEnabled);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color)
	{
		DrawCapsule(rotation, position, radius, halfHeight, color, true);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, bool depthEnabled)
	{
		constexpr uint32_t numSlices = 12;

		uint32_t numVertices = BasicGeometry::GetCapsuleWireframeSize(numSlices);

		PC_Vertex * vData = GetBufferForVertices(numVertices, depthEnabled);

		BasicGeometry::CreateCapsuleWireframe(vData, sizeof(PC_Vertex), numSlices, radius, halfHeight, 0);

		for(uint32_t i = 0; i < numVertices; ++i) {
			vData[i].position = Vector3(vData[i].position).Rotate(rotation) + position;
			vData[i].color = color;
		}
	}

}
