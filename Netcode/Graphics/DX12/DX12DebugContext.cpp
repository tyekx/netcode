#include "DX12DebugContext.h"
#include <Netcode/HandleTypes.h>
#include <Netcode/Config.h>
#include <Netcode/Vertex.h>
#include <Netcode/Modules.h>
#include <Netcode/BasicGeometry.h>
#include <Netcode/Graphics/ResourceEnums.h>

namespace Netcode::Graphics::DX12 {

	void DebugContext::PushVertex(const PC_Vertex & vertex, bool depthEnabled) {
		// assert: bufferSize > (numNoDepthVertices + numDepthVertices)
		if(depthEnabled) {
			vertices[numDepthVertices++] = vertex;
		} else {
			vertices[bufferSize - numNoDepthVertices++] = vertex;
		}
	}

	PC_Vertex * DebugContext::GetBufferForVertices(size_t numVertices, bool depthEnabled) {
		// assert: bufferSize >= (numNoDepthVertices + numDepthVertices + numVertices)
		PC_Vertex * tmp;
		if(depthEnabled) {
			tmp = vertices.data() + numDepthVertices;
			numDepthVertices += numVertices;
		} else {
			tmp = vertices.data() + (bufferSize - numNoDepthVertices - numVertices);
			numNoDepthVertices += numVertices;
		}
		return tmp;
	}

	void DebugContext::CreateResources(Module::IGraphicsModule * graphics) {
		Ref<Netcode::InputLayoutBuilder> ilBuilder = graphics->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("COLOR", DXGI_FORMAT_R32G32B32_FLOAT);
		Ref<Netcode::InputLayout> il = ilBuilder->Build();

		Netcode::DepthStencilDesc depthStencilDesc;
		Netcode::RasterizerDesc rasterizerDesc;
		rasterizerDesc.antialiasedLineEnable = true;
		depthStencilDesc.depthWriteMaskZero = true;

		Ref<Netcode::ShaderBuilder> shaderBuilder = graphics->CreateShaderBuilder();
		Ref<Netcode::ShaderBytecode> vertexShader = shaderBuilder->LoadBytecode(L"Netcode_DebugPrimVS.cso");
		Ref<Netcode::ShaderBytecode> pixelShader = shaderBuilder->LoadBytecode(L"Netcode_DebugPrimPS.cso");

		Ref<Netcode::RootSignatureBuilder> rootSigBuilder = graphics->CreateRootSignatureBuilder();
		rootSignature = rootSigBuilder->BuildFromShader(vertexShader);

		Ref<Netcode::GPipelineStateBuilder> gpsoBuilder = graphics->CreateGPipelineStateBuilder();
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

		defaultColor = Config::Get<Float3>("graphics.debug.defaultColor:Float3");
		bufferSize = Config::Get<uint32_t>("graphics.debug.primitiveBufferDepth:u32");
		uploadBuffer = graphics->resources->CreateVertexBuffer(bufferSize * sizeof(PC_Vertex),
			sizeof(PC_Vertex),
			ResourceType::PERMANENT_UPLOAD,
			ResourceState::ANY_READ);

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
		DrawLine(worldPosStart, worldPosEnd, defaultColor, depthEnabled);
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
		DrawSphere(worldPosOrigin, radius, defaultColor, true);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, bool depthEnabled)
	{
		DrawSphere(worldPosOrigin, radius, defaultColor, depthEnabled);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color)
	{
		DrawSphere(worldPosOrigin, radius, color, true);
	}

	void DebugContext::DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, bool depthEnabled)
	{
		const uint32_t numSlices = Config::Get<uint32_t>("graphics.debug.sphereSlices:u32");

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
		DrawBoundingBox(worldPosOrigin, halfExtents, defaultColor, true);
	}

	void DebugContext::DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled)
	{
		DrawBoundingBox(worldPosOrigin, halfExtents, defaultColor, depthEnabled);
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
		DrawBox(orientation, worldPosOrigin, halfExtents, defaultColor, true);
	}

	void DebugContext::DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled)
	{
		DrawBox(orientation, worldPosOrigin, halfExtents, defaultColor, depthEnabled);
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
		DrawCapsule(rotation, position, radius, halfHeight, defaultColor, true);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, bool depthEnabled)
	{
		DrawCapsule(rotation, position, radius, halfHeight, defaultColor, depthEnabled);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color)
	{
		DrawCapsule(rotation, position, radius, halfHeight, color, true);
	}

	void DebugContext::DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, bool depthEnabled)
	{
		const uint32_t numSlices = Config::Get<uint32_t>("graphics.debug.capsuleSlices:u32");

		uint32_t numVertices = BasicGeometry::GetCapsuleWireframeSize(numSlices);

		PC_Vertex * vData = GetBufferForVertices(numVertices, depthEnabled);

		BasicGeometry::CreateCapsuleWireframe(vData, sizeof(PC_Vertex), numSlices, radius, halfHeight, 0);

		for(uint32_t i = 0; i < numVertices; ++i) {
			vData[i].position = Vector3(vData[i].position).Rotate(rotation) + position;
			vData[i].color = color;
		}
	}

}
